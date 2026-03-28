#include "rules/circular_inheritance.h"

#include <forward_list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "main/lint_rules.h"
#include "utils/ast_utils.h"
#include "utils/location_utils.h"

namespace {
class DependencyGraph {
 public:
  DependencyGraph() = default;
  ~DependencyGraph() = default;

  DependencyGraph(const DependencyGraph&) = delete;
  auto operator=(const DependencyGraph&) -> DependencyGraph& = delete;

  DependencyGraph(DependencyGraph&&) = default;
  auto operator=(DependencyGraph&&) -> DependencyGraph& = default;

  auto AddVertex(SL::NodeId node) -> bool {
    if (Contains(node)) {
      return true;
    }

    auto [element, inserted] =
        adjacencyList.try_emplace(node, std::forward_list<unsigned>{});
    if (inserted) {
      vertexCount++;
      return true;
    }
    return false;
  }

  auto AddEdge(SL::NodeId fromNode, SURELOG::NodeId toNode) -> bool {
    auto fromIt = adjacencyList.find(fromNode);
    auto toIt = adjacencyList.find(toNode);

    if (fromIt == adjacencyList.end() || toIt == adjacencyList.end()) {
      return false;
    }

    if (fromNode == toNode) {
      return false;
    }

    fromIt->second.push_front(toNode);
    return true;
  }

  auto FindCyclicDependencies() -> std::unordered_set<unsigned>;

  auto Contains(SURELOG::NodeId node) const noexcept -> bool {
    return adjacencyList.contains(node);
  }

 private:
  size_t vertexCount = 0;
  std::unordered_map<unsigned, std::forward_list<unsigned>> adjacencyList;
};

void BackTrackVertex(std::vector<unsigned>& path, unsigned current,
                     std::unordered_set<unsigned>& inCurrentPath) {
  if (!path.empty() && path.back() == current) {
    path.pop_back();
    inCurrentPath.erase(current);
  }
}
void CheckAdjacentElement(
    std::unordered_map<unsigned, std::forward_list<unsigned>>& adjacencyList,
    unsigned current, std::unordered_set<unsigned>& cyclicDependencies,
    std::vector<unsigned>& stack, std::unordered_set<unsigned>& visited,
    std::unordered_set<unsigned>& inCurrentPath) {
  auto element = adjacencyList.find(current);
  if (element == adjacencyList.end()) {
    return;
  }
  for (auto neighbor : element->second) {
    if (inCurrentPath.contains(neighbor)) {
      cyclicDependencies.insert(SURELOG::NodeId(current));
    }
    if (!visited.contains(neighbor)) {
      stack.push_back(neighbor);
    }
  }
}

auto DependencyGraph::FindCyclicDependencies() -> std::unordered_set<unsigned> {
  std::unordered_set<unsigned> cyclicDependencies;

  if (vertexCount == 0) {
    return cyclicDependencies;
  }

  std::vector<unsigned> stack;
  std::vector<unsigned> path;
  std::unordered_set<unsigned> visited;
  std::unordered_set<unsigned> inCurrentPath;

  stack.reserve(vertexCount);
  path.reserve(vertexCount);

  for (const auto& [vertex, inserted] : adjacencyList) {
    if (visited.contains(vertex)) {
      continue;
    }

    stack.push_back(vertex);

    while (!stack.empty()) {
      unsigned current = stack.back();

      if (!visited.contains(current)) {
        visited.insert(current);
        inCurrentPath.insert(current);
        path.push_back(current);

        CheckAdjacentElement(adjacencyList, current, cyclicDependencies, stack,
                             visited, inCurrentPath);
        // auto element = adjacencyList.find(current);
        // if (element != adjacencyList.end()) {
        //   for (auto neighbor : element->second) {
        //     if (inCurrentPath.contains(neighbor)) {
        //       cyclicDependencies.insert(SURELOG::NodeId(current));
        //     }
        //     if (!visited.contains(neighbor)) {
        //       stack.push_back(neighbor);
        //     }
        //   }
        // }
      } else {
        stack.pop_back();
        BackTrackVertex(path, current, inCurrentPath);
      }
    }
  }

  return cyclicDependencies;
}

}  // namespace

void CheckCircularInheritance(const SURELOG::FileContent* fileContent,
                              SURELOG::ErrorContainer* errors,
                              SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  DependencyGraph dependencyGraph;
  const std::unordered_map<std::string, SURELOG::NodeId> kClassSet =
      GetClassIds(fileContent);

  const std::vector<SURELOG::NodeId> kClassDeclarations =
      fileContent->sl_collect_all(fileContent->getRootNode(),
                                  SURELOG::VObjectType::paClass_declaration);

  for (const auto& classId : kClassDeclarations) {
    const std::string kClassName = GetStringConst(fileContent, classId);
    if (IsBuiltinClass(kClassName)) {
      continue;
    }

    if (!dependencyGraph.AddVertex(classId)) {
      throw std::runtime_error("Failed to add vertex: " +
                               std::to_string(sizeof(classId)));
    }

    const std::string kSuperName = GetSuperclassString(fileContent, classId);
    if (kSuperName == "") {
      continue;
    }

    const std::string kFullName = GetPrefix(fileContent, classId) + kSuperName;
    const SURELOG::NodeId kSuperId = kClassSet.at(kFullName);
    if (!dependencyGraph.AddVertex(kSuperId)) {
      throw std::runtime_error("Failed to add vertex: " +
                               std::to_string(sizeof(kSuperId)));
    }
    dependencyGraph.AddEdge(classId, kSuperId);
  }
  std::unordered_set cyclicDependencies =
      dependencyGraph.FindCyclicDependencies();
  for (const auto& numOfId : cyclicDependencies) {
    const SURELOG::NodeId kNode(numOfId);
    const std::string kName = GetStringConst(fileContent, kNode);
    ReportError(fileContent, kNode, kName,
                verihogg_lint::LINT_CIRCULAR_INHERITANCE, errors, symbols);
  }
}
