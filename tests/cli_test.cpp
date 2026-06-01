#include "main/cli.h"

#include <gtest/gtest.h>

#include <gsl/span>
#include <string>
#include <vector>

#include "main/surelog_flags.h"

namespace {

auto ParseFor(std::vector<const char*> argv) -> cli::Options {
  argv.insert(argv.begin(), "verihogg-lint");
  return cli::ParseArgs(gsl::span<const char*>{argv.data(), argv.size()});
}

TEST(CliParseArgs, UnknownLongFlag) {
  const auto opts = ParseFor({"--foobar"});
  EXPECT_NE(opts.error_message.find("unrecognized option '--foobar'"),
            std::string::npos)
      << opts.error_message;
}

TEST(CliParseArgs, UnknownShortFlag) {
  const auto opts = ParseFor({"-typoFlag"});
  EXPECT_NE(opts.error_message.find("unrecognized option '-typoFlag'"),
            std::string::npos)
      << opts.error_message;
}

TEST(CliParseArgs, ConfigFileBare) {
  const auto opts = ParseFor({"--config-file"});
  EXPECT_NE(opts.error_message.find("requires '=<path>'"), std::string::npos)
      << opts.error_message;
}

TEST(CliParseArgs, ConfigFileMissing) {
  const auto opts = ParseFor({"--config-file=/nonexistent/path.yaml"});
  EXPECT_NE(opts.error_message.find("cannot open config file"),
            std::string::npos)
      << opts.error_message;
}

TEST(CliParseArgs, FilelistMissing) {
  const auto opts = ParseFor({"-f", "/nonexistent/path.f"});
  EXPECT_NE(opts.error_message.find("cannot open filelist"), std::string::npos)
      << opts.error_message;
}

TEST(CliParseArgs, SourceFileMissing) {
  const auto opts = ParseFor({"/nonexistent/path.sv"});
  EXPECT_NE(opts.error_message.find("cannot open source file"),
            std::string::npos)
      << opts.error_message;
}

TEST(CliParseArgs, HelpSkipsValidation) {
  const auto opts = ParseFor({"missing.sv", "--help"});
  EXPECT_TRUE(opts.show_help);
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, SurelogLongFlagAccepted) {
  const auto opts = ParseFor({"--top-module", "top"});
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, SurelogShortFlagConsumesArg) {
  const auto opts = ParseFor({"-L", "libname"});
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, SurelogPrefixFlag) {
  const auto opts = ParseFor({"-Idir/path", "-Dvar=value"});
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, PlusFlagsAccepted) {
  const auto opts = ParseFor({"+incdir+/some/path", "+define+FOO=1"});
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, UvmBuiltin) {
  const auto opts = ParseFor({"--uvm"});
  EXPECT_EQ(opts.uvm_mode, cli::UvmMode::Builtin);
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, UvmCustomPath) {
  const auto opts = ParseFor({"--uvm=/some/path"});
  EXPECT_EQ(opts.uvm_mode, cli::UvmMode::Custom);
  EXPECT_EQ(opts.uvm_path, std::filesystem::path{"/some/path"});
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliParseArgs, UvmVersion) {
  const auto opts = ParseFor({"--uvm-version"});
  EXPECT_TRUE(opts.show_uvm_version);
  EXPECT_TRUE(opts.error_message.empty()) << opts.error_message;
}

TEST(CliSurelogFlags, AllNoArgFlagsClassified) {
  for (const auto f : cli::kSurelogNoArgFlags) {
    EXPECT_EQ(cli::ClassifySurelogFlag(f), cli::SurelogFlagKind::NoArg)
        << "Flag " << f << " is in kSurelogNoArgFlags but was not classified "
        << "as NoArg";
  }
}

TEST(CliSurelogFlags, AllOneArgFlagsClassified) {
  for (const auto f : cli::kSurelogOneArgFlags) {
    EXPECT_EQ(cli::ClassifySurelogFlag(f), cli::SurelogFlagKind::OneArg)
        << "Flag " << f << " is in kSurelogOneArgFlags but was not classified "
        << "as OneArg";
  }
  for (const auto f : cli::kSurelogLongOneArgFlags) {
    EXPECT_EQ(cli::ClassifySurelogFlag(f), cli::SurelogFlagKind::OneArg)
        << "Flag " << f
        << " is in kSurelogLongOneArgFlags but was not classified as OneArg";
  }
}

TEST(CliSurelogFlags, AllTwoArgFlagsClassified) {
  for (const auto f : cli::kSurelogTwoArgFlags) {
    EXPECT_EQ(cli::ClassifySurelogFlag(f), cli::SurelogFlagKind::TwoArg)
        << "Flag " << f << " is in kSurelogTwoArgFlags but was not classified "
        << "as TwoArg";
  }
}

TEST(CliSurelogFlags, PrefixFlagsMatch) {
  EXPECT_EQ(cli::ClassifySurelogFlag("-Idir"), cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("-Dvar=val"), cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("-Pparam=val"),
            cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("-pvalue+param=val"),
            cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("-timescale=1ns/1ps"),
            cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("--enable-feature=foo"),
            cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("--disable-feature=foo"),
            cli::SurelogFlagKind::NoArg);
}

TEST(CliSurelogFlags, PlusFlagsRecognized) {
  EXPECT_EQ(cli::ClassifySurelogFlag("+incdir+/path"),
            cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("+define+FOO=1"),
            cli::SurelogFlagKind::NoArg);
  EXPECT_EQ(cli::ClassifySurelogFlag("+libext+sv"),
            cli::SurelogFlagKind::NoArg);
}

TEST(CliSurelogFlags, UnknownReturnsUnknown) {
  EXPECT_EQ(cli::ClassifySurelogFlag("-zzz_unknown"),
            cli::SurelogFlagKind::Unknown);
  EXPECT_EQ(cli::ClassifySurelogFlag("--nope"), cli::SurelogFlagKind::Unknown);
}

}  // namespace