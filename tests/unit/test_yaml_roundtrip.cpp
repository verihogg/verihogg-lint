#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "fix/replacement.h"

namespace fs = std::filesystem;

namespace {

struct OffsetLength {
  unsigned offset;
  unsigned length;
};

auto MakeTempPath(const std::string& suffix) -> fs::path {
  return fs::temp_directory_path() /
         ("verihogg_test_" + suffix + "_" +
          std::to_string(std::hash<std::string>{}(suffix)) + ".yaml");
}

auto WriteFile(const fs::path& path, const std::string& content) -> void {
  std::ofstream f(path);
  f << content;
}

auto ReadFile(const fs::path& path) -> std::string {
  std::ifstream f(path);
  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

auto MakeR(const std::string& filename, OffsetLength pos,
           const std::string& text, const std::string& rule = "TEST")
    -> Replacement {
  Replacement r;
  r.filename = filename;
  r.offset = pos.offset;
  r.length = pos.length;
  r.text = text;
  r.rule_id = rule;
  return r;
}

}  // namespace

TEST(ExportToYaml, EmptyProducesValidYaml) {
  const FileReplacements fr;
  const fs::path out = MakeTempPath("empty_export");

  ASSERT_TRUE(fr.exportToYaml(out.string()));

  const std::string content = ReadFile(out);
  EXPECT_FALSE(content.empty());
  EXPECT_NE(content.find("Replacements"), std::string::npos);

  fs::remove(out);
}

TEST(ExportToYaml, SingleReplacementRoundtrip) {
  FileReplacements fr;
  ASSERT_TRUE(fr.add(MakeR("/src/foo.sv", {10, 5}, "logic", "TYPE_CASTING")));

  const fs::path out = MakeTempPath("single_export");
  ASSERT_TRUE(fr.exportToYaml(out.string()));

  const std::string yaml = ReadFile(out);
  EXPECT_NE(yaml.find("/src/foo.sv"), std::string::npos);
  EXPECT_NE(yaml.find("10"), std::string::npos);
  EXPECT_NE(yaml.find('5'), std::string::npos);
  EXPECT_NE(yaml.find("logic"), std::string::npos);

  fs::remove(out);
}

TEST(ExportToYaml, CreatesParentDirectories) {
  FileReplacements fr;
  ASSERT_TRUE(fr.add(MakeR("/src/foo.sv", {0, 1}, "x")));

  const fs::path out =
      fs::temp_directory_path() / "verihogg_nested_dir" / "sub" / "out.yaml";
  ASSERT_TRUE(fr.exportToYaml(out.string()));
  EXPECT_TRUE(fs::exists(out));

  fs::remove_all(out.parent_path().parent_path());
}

TEST(ImportFromYaml, NonExistentFileReturnsFalse) {
  FileReplacements fr;
  EXPECT_FALSE(fr.importFromYaml("/nonexistent/path/fixes.yaml"));
}

TEST(ImportFromYaml, InvalidYamlReturnsFalse) {
  const fs::path path = MakeTempPath("invalid_yaml");
  WriteFile(path, "}{this is not valid yaml{{");

  FileReplacements fr;
  EXPECT_FALSE(fr.importFromYaml(path.string()));

  fs::remove(path);
}

TEST(ImportFromYaml, MissingReplacementsKeyReturnsFalse) {
  const fs::path path = MakeTempPath("no_replacements_key");
  WriteFile(path, "SomeOtherKey:\n  - Foo: bar\n");

  FileReplacements fr;
  EXPECT_FALSE(fr.importFromYaml(path.string()));

  fs::remove(path);
}

TEST(ImportFromYaml, EmptySequenceSucceeds) {
  const fs::path path = MakeTempPath("empty_seq");
  WriteFile(path, "Replacements: []\n");

  FileReplacements fr;
  EXPECT_TRUE(fr.importFromYaml(path.string()));
  EXPECT_TRUE(fr.empty());

  fs::remove(path);
}

TEST(ImportFromYaml, MalformedEntrySkipped) {
  const fs::path path = MakeTempPath("malformed_entry");
  WriteFile(path,
            "Replacements:\n"
            "  - FilePath: /src/a.sv\n"
            "    # Missing Offset and Length — should be skipped\n"
            "  - FilePath: /src/b.sv\n"
            "    Offset: 5\n"
            "    Length: 3\n"
            "    ReplacementText: ok\n");

  FileReplacements fr;
  EXPECT_TRUE(fr.importFromYaml(path.string()));
  EXPECT_EQ(fr.totalCount(), 1U);

  fs::remove(path);
}

TEST(ImportFromYaml, SingleEntry) {
  const fs::path path = MakeTempPath("single_entry");
  WriteFile(path,
            "Replacements:\n"
            "  - FilePath: /src/foo.sv\n"
            "    Offset: 42\n"
            "    Length: 7\n"
            "    ReplacementText: \"logic \"\n");

  FileReplacements fr;
  ASSERT_TRUE(fr.importFromYaml(path.string()));
  EXPECT_EQ(fr.totalCount(), 1U);

  fs::remove(path);
}

TEST(ImportFromYaml, MultipleEntriesAcrossFiles) {
  const fs::path path = MakeTempPath("multi_entry");
  WriteFile(path,
            "Replacements:\n"
            "  - FilePath: /src/a.sv\n"
            "    Offset: 30\n"
            "    Length: 0\n"
            "    ReplacementText: \"'\"\n"
            "  - FilePath: /src/b.sv\n"
            "    Offset: 20\n"
            "    Length: 3\n"
            "    ReplacementText: \"==?\"\n"
            "  - FilePath: /src/a.sv\n"
            "    Offset: 5\n"
            "    Length: 9\n"
            "    ReplacementText: \"\"\n");

  FileReplacements fr;
  ASSERT_TRUE(fr.importFromYaml(path.string()));
  EXPECT_EQ(fr.totalCount(), 3U);

  fs::remove(path);
}

TEST(ImportFromYaml, ConflictingEntriesAreSkipped) {
  const fs::path path = MakeTempPath("conflict");
  WriteFile(path,
            "Replacements:\n"
            "  - FilePath: /src/x.sv\n"
            "    Offset: 10\n"
            "    Length: 5\n"
            "    ReplacementText: \"aaa\"\n"
            "  - FilePath: /src/x.sv\n"
            "    Offset: 12\n"
            "    Length: 3\n"
            "    ReplacementText: \"bbb\"\n");

  FileReplacements fr;
  ASSERT_TRUE(fr.importFromYaml(path.string()));
  EXPECT_EQ(fr.totalCount(), 1U);

  fs::remove(path);
}

TEST(ImportFromYaml, ExactDuplicateAccepted) {
  const fs::path path = MakeTempPath("duplicate");
  WriteFile(path,
            "Replacements:\n"
            "  - FilePath: /src/z.sv\n"
            "    Offset: 5\n"
            "    Length: 0\n"
            "    ReplacementText: \"'\"\n"
            "  - FilePath: /src/z.sv\n"
            "    Offset: 5\n"
            "    Length: 0\n"
            "    ReplacementText: \"'\"\n");

  FileReplacements fr;
  ASSERT_TRUE(fr.importFromYaml(path.string()));
  EXPECT_EQ(fr.totalCount(), 1U);

  fs::remove(path);
}

TEST(YamlRoundtrip, ExportThenImportPreservesReplacements) {
  FileReplacements original;
  ASSERT_TRUE(original.add(MakeR("/src/a.sv", {100, 5}, "logic", "RULE_A")));
  ASSERT_TRUE(original.add(MakeR("/src/a.sv", {50, 0}, "'", "RULE_B")));
  ASSERT_TRUE(original.add(MakeR("/src/b.sv", {20, 3}, "==?", "RULE_C")));

  const fs::path path = MakeTempPath("roundtrip");
  ASSERT_TRUE(original.exportToYaml(path.string()));

  FileReplacements restored;
  ASSERT_TRUE(restored.importFromYaml(path.string()));

  EXPECT_EQ(restored.totalCount(), original.totalCount());

  fs::remove(path);
}

TEST(YamlRoundtrip, EmptyFileReplacementsRoundtrip) {
  const FileReplacements original;

  const fs::path path = MakeTempPath("empty_roundtrip");
  ASSERT_TRUE(original.exportToYaml(path.string()));

  FileReplacements restored;
  ASSERT_TRUE(restored.importFromYaml(path.string()));

  EXPECT_TRUE(restored.empty());

  fs::remove(path);
}

TEST(YamlRoundtrip, RemovalRoundtrip) {
  FileReplacements original;
  ASSERT_TRUE(original.add(MakeR("/src/c.sv", {30, 9}, "", "REMOVAL")));

  const fs::path path = MakeTempPath("removal_roundtrip");
  ASSERT_TRUE(original.exportToYaml(path.string()));

  FileReplacements restored;
  ASSERT_TRUE(restored.importFromYaml(path.string()));
  EXPECT_EQ(restored.totalCount(), 1U);

  fs::remove(path);
}