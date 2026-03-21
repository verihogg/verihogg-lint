#include "rules/circular_inheritance.h"

#include <forward_list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "utils/ast_utils.h"
#include "utils/location_utils.h"

using namespace SURELOG;

namespace {
class DependencyGraph {
 public:
  DependencyGraph() = default;
  ~DependencyGraph() = default;

  DependencyGraph(const DependencyGraph&) = delete;
  DependencyGraph& operator=(const DependencyGraph&) = delete;

  DependencyGraph(DependencyGraph&&) = default;
  DependencyGraph& operator=(DependencyGraph&&) = default;

  bool addVertex(NodeId id) {
    if (contains(id)) return true;

    auto [it, inserted] =
        adjacencyList.try_emplace(id, std::forward_list<unsigned>{});
    if (inserted) {
      vertexCount++;
      return true;
    }
    return false;
  }

  bool addEdge(NodeId from, NodeId to) {
    auto fromIt = adjacencyList.find(from);
    auto toIt = adjacencyList.find(to);

    if (fromIt == adjacencyList.end() || toIt == adjacencyList.end()) {
      return false;
    }

    if (from == to) {
      return false;
    }

    fromIt->second.push_front(to);
    return true;
  }

  std::unordered_set<unsigned> findCyclicDependencies() const {
    std::unordered_set<unsigned> cyclicDependencies;

    if (vertexCount == 0) return cyclicDependencies;

    std::vector<unsigned> stack;
    std::vector<unsigned> path;
    std::unordered_set<unsigned> visited;
    std::unordered_set<unsigned> inCurrentPath;

    stack.reserve(vertexCount);
    path.reserve(vertexCount);

    for (const auto& [vertex, _] : adjacencyList) {
      if (visited.find(vertex) != visited.end()) {
        continue;
      }

      stack.push_back(vertex);

      while (!stack.empty()) {
        unsigned current = stack.back();

        if (visited.find(current) == visited.end()) {
          visited.insert(current);
          inCurrentPath.insert(current);
          path.push_back(current);

          auto it = adjacencyList.find(current);
          if (it != adjacencyList.end()) {
            for (auto neighbor : it->second) {
              if (inCurrentPath.find(neighbor) != inCurrentPath.end()) {
                cyclicDependencies.insert(NodeId(current));
              }
              if (visited.find(neighbor) == visited.end()) {
                stack.push_back(neighbor);
              }
            }
          }
        } else {
          stack.pop_back();
          if (!path.empty() && path.back() == current) {
            path.pop_back();
            inCurrentPath.erase(current);
          }
        }
      }
    }

    return cyclicDependencies;
  }

  size_t size() const noexcept { return vertexCount; }
  bool empty() const noexcept { return vertexCount == 0; }
  void clear() noexcept {
    adjacencyList.clear();
    vertexCount = 0;
  }

  bool contains(NodeId id) const noexcept {
    return adjacencyList.find(id) != adjacencyList.end();
  }

  size_t outDegree(NodeId id) const {
    auto it = adjacencyList.find(id);
    if (it == adjacencyList.end()) {
      return 0;
    }
    return std::distance(it->second.begin(), it->second.end());
  }

 private:
  size_t vertexCount = 0;
  std::unordered_map<unsigned, std::forward_list<unsigned>> adjacencyList;
};

}  // namespace

void checkCircularInheritance(const FileContent* fC, ErrorContainer* errors,
                              SymbolTable* symbols) {
  if (!fC) return;

  DependencyGraph dependencyGraph;
  const std::unordered_map<std::string, NodeId> classSet = getClassIds(fC);

  const std::vector<NodeId> classDeclarations =
      fC->sl_collect_all(fC->getRootNode(), VObjectType::paClass_declaration);

  for (auto& classId : classDeclarations) {
    const std::string className = getStringConst(fC, classId);
    if (isBuiltinClass(className)) continue;

    if (!dependencyGraph.addVertex(classId)) {
      throw std::runtime_error("Failed to add vertex: " +
                               std::to_string(RawNodeId(classId)));
    }

    const std::string superName = getSuperclassString(fC, classId);
    if (superName == "") continue;

    const std::string fullName = getPrefix(fC, classId) + superName;
    const NodeId superId = classSet.at(fullName);
    if (!dependencyGraph.addVertex(superId)) {
      throw std::runtime_error("Failed to add vertex: " +
                               std::to_string(RawNodeId(superId)));
    }
    dependencyGraph.addEdge(classId, superId);
  }
  std::unordered_set cyclicDependencies =
      dependencyGraph.findCyclicDependencies();
  for (auto& numOfId : cyclicDependencies) {
    const NodeId id(numOfId);
    const std::string name = getStringConst(fC, id);
    ReportError(fC, id, name, ErrorDefinition::LINT_CIRCULAR_INHERITANCE,
                errors, symbols);
  }
}
