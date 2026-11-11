#include <Surelog/API/Surelog.h>
#include <Surelog/CommandLine/CommandLineParser.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <gtest/gtest.h>
#include <uhdm/vpi_user.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "main/lint_rules.h"
#include "rules/banned_display_write_tasks.h"
#include "rules/banned_program_blocks.h"
#include "rules/banned_psprintf.h"
#include "rules/banned_randsequence.h"
#include "rules/banned_simulation_control_tasks.h"
#include "utils/init.h"

namespace fs = std::filesystem;
namespace SL = SURELOG;

auto basePath() -> fs::path {
  return fs::current_path() / ".." / ".." / "tests" / "BannedApiChecks";
}

auto compileFile(const fs::path& path, SL::SymbolTable* symbols,
                 SL::ErrorContainer* errors)
    -> std::pair<SL::Design*, SL::FileContent*> {
  auto clp =
      std::make_unique<SL::CommandLineParser>(errors, symbols, false, false);
  InitCommandLineParser(clp.get());
  std::string path_str = path.string();
  std::array<const char*, 2> argv = {"", path_str.c_str()};
  if (!clp->parseCommandLine(2, argv.data())) {
    return {nullptr, nullptr};
  }
  auto compiler = start_compiler(clp.get());
  auto design = get_design(compiler);
  if (!design) {
    return {nullptr, nullptr};
  }
  auto fcs = design->getAllFileContents();
  SL::FileContent* fc = fcs.empty() ? nullptr : fcs.begin()->second;
  return {design, fc};
}

void expectNoErrors(
    const fs::path& dir,
    const std::function<void(SL::FileContent*, SL::ErrorContainer*,
                             SL::SymbolTable*)>& check) {
  for (auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());
    auto [design, fc] = compileFile(entry.path(), symbols.get(), errors.get());
    ASSERT_NE(design, nullptr);
    check(fc, errors.get(), symbols.get());
    errors->printMessages();
    EXPECT_EQ(errors->getErrors().size(), 0) << entry.path();
  }
}

void expectError(const fs::path& dir, verihogg_lint::LintIdEnum expected,
                 const std::function<void(SL::FileContent*, SL::ErrorContainer*,
                                          SL::SymbolTable*)>& check) {
  for (auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    auto symbols = std::make_unique<SL::SymbolTable>();
    auto errors = std::make_unique<SL::ErrorContainer>(symbols.get());
    auto [design, fc] = compileFile(entry.path(), symbols.get(), errors.get());
    ASSERT_NE(design, nullptr);
    check(fc, errors.get(), symbols.get());
    errors->printMessages();
    bool found = false;
    for (auto& err : errors->getErrors()) {
      if (static_cast<int>(err.getType()) == static_cast<int>(expected)) {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found) << entry.path();
  }
}

TEST(BannedProgramBlocks, NoError) {
  expectNoErrors(basePath() / "BannedProgramBlocks" / "NoError",
                 CheckBannedProgramBlocks);
}

TEST(BannedProgramBlocks, RaiseError) {
  expectError(basePath() / "BannedProgramBlocks" / "RaiseError",
              verihogg_lint::LINT_BANNED_PROGRAM_BLOCKS,
              CheckBannedProgramBlocks);
}

TEST(BannedRandsequence, NoError) {
  expectNoErrors(basePath() / "BannedRandsequence" / "NoError",
                 CheckBannedRandsequence);
}

TEST(BannedRandsequence, RaiseError) {
  expectError(basePath() / "BannedRandsequence" / "RaiseError",
              verihogg_lint::LINT_BANNED_RANDSEQUENCE, CheckBannedRandsequence);
}

TEST(BannedSimulationControlTasks, NoError) {
  expectNoErrors(basePath() / "BannedSimulationControlTasks" / "NoError",
                 CheckBannedSimulationControlTasks);
}

TEST(BannedSimulationControlTasks, RaiseError) {
  expectError(basePath() / "BannedSimulationControlTasks" / "RaiseError",
              verihogg_lint::LINT_BANNED_SIMULATION_CONTROL_TASKS,
              CheckBannedSimulationControlTasks);
}

TEST(BannedPsprintf, NoError) {
  expectNoErrors(basePath() / "BannedPsprintf" / "NoError",
                 CheckBannedPsprintf);
}

TEST(BannedPsprintf, RaiseError) {
  expectError(basePath() / "BannedPsprintf" / "RaiseError",
              verihogg_lint::LINT_BANNED_PSPRINTF, CheckBannedPsprintf);
}

TEST(BannedDisplayWriteTasks, NoError) {
  expectNoErrors(basePath() / "BannedDisplayWriteTasks" / "NoError",
                 CheckBannedDisplayWriteTasks);
}

TEST(BannedDisplayWriteTasks, RaiseError) {
  expectError(basePath() / "BannedDisplayWriteTasks" / "RaiseError",
              verihogg_lint::LINT_BANNED_DISPLAY_WRITE_TASKS,
              CheckBannedDisplayWriteTasks);
}

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
