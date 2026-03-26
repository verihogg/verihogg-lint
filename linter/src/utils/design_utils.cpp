#include "utils/design_utils.h"

#include <functional>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"

namespace SL = SURELOG;

namespace DesignUtils {

void ForEachFileContent(
    SL::Design* design,
    const std::function<void(const SL::FileContent*)>& func) {
  if (design == nullptr) {
    return;
  }
  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    func(fileContent);
  }
}

}  // namespace DesignUtils