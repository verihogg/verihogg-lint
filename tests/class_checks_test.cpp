#include <gtest/gtest.h>
#include <rules/extend_class.h>
#include <utils/init.h>

#include <filesystem>
#include <functional>
#include <memory>

#include "Surelog/API/Surelog.h"
#include "Surelog/CommandLine/CommandLineParser.h"
#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

namespace fs = std::filesystem;
using namespace SURELOG;

const fs::path base_path = fs::current_path() / ".." / "..";

namespace {

FileContent* getFileContentFromPath(fs::path path, ErrorContainer* errors,
                                    SymbolTable* symbols) {
  const auto clp =
      std::make_unique<CommandLineParser>(errors, symbols, false, false);
  initCommandLineParser(clp.get());

  const std::string path_str = path.string();
  const char* argv[2] = {"", path_str.c_str()};
  if (!clp->parseCommandLine(2, argv)) {
    std::cerr << "Can't parse command line" << std::endl;
  }

  try {
    const auto compiler = start_compiler(clp.get());
    const auto vec = get_design(compiler)->getAllFileContents();
    return vec[0].second;
  } catch (const std::exception& e) {
    std::cerr << "Compiler error: " << e.what() << '\n';
    return nullptr;
  }
}

void testCheckWithNoErrorsExpected(
    const fs::path tests_path,
    std::function<void(const FileContent*, ErrorContainer*, SymbolTable*)>
        check_func) {
  std::vector<fs::path> paths(fs::directory_iterator{tests_path},
                              fs::directory_iterator{});
  for (auto& file_path : paths) {
    auto symbols = std::make_unique<SymbolTable>();
    auto errors = std::make_unique<ErrorContainer>(symbols.get());

    const FileContent* fC =
        getFileContentFromPath(file_path, errors.get(), symbols.get());
    check_func(fC, errors.get(), symbols.get());
    errors->printMessages();
    ASSERT_EQ(errors.get()->getErrors().size(), 0);
  }
}

}  // namespace

TEST(ExtendClassTest, NoErrors) {
  const fs::path tests_path{base_path / "tests" / "ClassChecks" /
                            "ExtendClass" / "NoError"};

  testCheckWithNoErrorsExpected(tests_path, checkExtendClass);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
