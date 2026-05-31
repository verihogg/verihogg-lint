#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "fix/replacement.h"

namespace fs = std::filesystem;

static auto writeTempFile(const std::string& tag, const std::string& content)
    -> fs::path {
  const fs::path p =
      fs::temp_directory_path() /
      ("verihogg_diff_" + tag + "_" +
       std::to_string(std::hash<std::string>{}(tag + content)) + ".sv");
  std::ofstream f(p, std::ios::binary | std::ios::trunc);
  f << content;
  return p;
}

struct ReplacementSpec {
  unsigned offset;
  unsigned length;
};

static auto makeFR(const fs::path& file, ReplacementSpec spec,
                   const std::string& text) -> FileReplacements {
  FileReplacements fr;
  Replacement r;
  r.filename = file.string();
  r.offset = spec.offset;
  r.length = spec.length;
  r.text = text;
  r.rule_id = "TEST";
  EXPECT_TRUE(fr.add(r));
  return fr;
}

static auto runDiff(const FileReplacements& fr) -> std::string {
  std::ostringstream oss;
  fr.printDiff(oss);
  return oss.str();
}

static auto splitLines(const std::string& s) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::istringstream ss(s);
  std::string line;
  while (std::getline(ss, line)) {
    result.push_back(line);
  }
  return result;
}

static auto countHunks(const std::string& diff) -> int {
  int count = 0;
  for (const auto& line : splitLines(diff)) {
    if (line.starts_with("@@")) {
      ++count;
    }
  }
  return count;
}

static auto lineOffset(const std::string& src, int n) -> unsigned {
  unsigned off = 0;
  int cur = 1;
  while (cur < n && off < src.size()) {
    if (src.at(off) == '\n') {
      ++cur;
    }
    ++off;
  }
  return off;
}

TEST(PrintDiff, NoChangesProducesNoOutput) {
  const fs::path f = writeTempFile("no_change", "module foo;\nendmodule\n");
  const auto fr = makeFR(f, {.offset = 7U, .length = 3U}, "foo");
  EXPECT_TRUE(runDiff(fr).empty());
  fs::remove(f);
}

TEST(PrintDiff, SingleLineReplacementHunkHeader) {
  const std::string src = "line 1\nline 2\nline 3\n";
  const fs::path f = writeTempFile("single_repl", src);

  const auto fr = makeFR(f, {.offset = 12U, .length = 1U}, "TWO");
  const std::string out = runDiff(fr);

  ASSERT_FALSE(out.empty());

  const auto ls = splitLines(out);
  ASSERT_GE(ls.size(), 7U);
  EXPECT_TRUE(ls.at(0).starts_with("--- a/")) << ls.at(0);
  EXPECT_TRUE(ls.at(1).starts_with("+++ b/")) << ls.at(1);
  EXPECT_EQ(ls.at(2), "@@ -1,3 +1,3 @@") << ls.at(2);
  EXPECT_EQ(ls.at(3), " line 1");
  EXPECT_EQ(ls.at(4), "-line 2");
  EXPECT_EQ(ls.at(5), "+line TWO");
  EXPECT_EQ(ls.at(6), " line 3");

  fs::remove(f);
}

TEST(PrintDiff, LineInsertionHunkHeader) {
  const std::string src = "line 1\nline 2\nline 3\n";
  const fs::path f = writeTempFile("insertion", src);
  const auto fr = makeFR(f, {.offset = 7U, .length = 0U}, "line X\n");

  const auto ls = splitLines(runDiff(fr));

  ASSERT_GE(ls.size(), 7U);
  EXPECT_EQ(ls.at(2), "@@ -1,3 +1,4 @@") << ls.at(2);
  EXPECT_EQ(ls.at(3), " line 1");
  EXPECT_EQ(ls.at(4), "+line X");
  EXPECT_EQ(ls.at(5), " line 2");
  EXPECT_EQ(ls.at(6), " line 3");

  fs::remove(f);
}

TEST(PrintDiff, LineRemovalHunkHeader) {
  const std::string src = "line 1\nline 2\nline 3\n";
  const fs::path f = writeTempFile("removal", src);
  const auto fr = makeFR(f, {.offset = 7U, .length = 7U}, "");

  const auto ls = splitLines(runDiff(fr));

  ASSERT_GE(ls.size(), 5U);
  EXPECT_EQ(ls.at(2), "@@ -1,3 +1,2 @@") << ls.at(2);
  EXPECT_EQ(ls.at(3), " line 1");
  EXPECT_EQ(ls.at(4), "-line 2");
  EXPECT_EQ(ls.at(5), " line 3");

  fs::remove(f);
}

TEST(PrintDiff, DistantChangesProduceTwoHunks) {
  constexpr int kLines = 20;
  constexpr int kLastLine = kLines;

  std::string src;
  for (int i = 1; i <= kLines; ++i) {
    src += "line " + std::to_string(i) + "\n";
  }

  const unsigned off1 = 0U;
  const unsigned len1 = static_cast<unsigned>(std::string("line 1").size());
  const unsigned offLast = lineOffset(src, kLastLine);
  const unsigned lenLast = static_cast<unsigned>(
      std::string("line " + std::to_string(kLastLine)).size());

  const fs::path f = writeTempFile("two_hunks", src);

  FileReplacements fr;
  {
    Replacement r;
    r.filename = f.string();
    r.offset = off1;
    r.length = len1;
    r.text = "LINE 1";
    r.rule_id = "R1";
    ASSERT_TRUE(fr.add(r));
  }
  {
    Replacement r;
    r.filename = f.string();
    r.offset = offLast;
    r.length = lenLast;
    r.text = "LINE " + std::to_string(kLastLine);
    r.rule_id = "R2";
    ASSERT_TRUE(fr.add(r));
  }

  EXPECT_EQ(countHunks(runDiff(fr)), 2);

  fs::remove(f);
}

TEST(PrintDiff, NearbyChangesProduceOneHunk) {
  constexpr int kLines = 15;
  constexpr int kLineA = 5;
  constexpr int kLineB = 7;

  std::string src;
  for (int i = 1; i <= kLines; ++i) {
    src += "line " + std::to_string(i) + "\n";
  }

  const unsigned offA = lineOffset(src, kLineA);
  const unsigned lenA = static_cast<unsigned>(
      std::string("line " + std::to_string(kLineA)).size());
  const unsigned offB = lineOffset(src, kLineB);
  const unsigned lenB = static_cast<unsigned>(
      std::string("line " + std::to_string(kLineB)).size());

  const fs::path f = writeTempFile("one_hunk", src);

  FileReplacements fr;
  {
    Replacement r;
    r.filename = f.string();
    r.offset = offA;
    r.length = lenA;
    r.text = "LINE " + std::to_string(kLineA);
    r.rule_id = "R1";
    ASSERT_TRUE(fr.add(r));
  }
  {
    Replacement r;
    r.filename = f.string();
    r.offset = offB;
    r.length = lenB;
    r.text = "LINE " + std::to_string(kLineB);
    r.rule_id = "R2";
    ASSERT_TRUE(fr.add(r));
  }

  EXPECT_EQ(countHunks(runDiff(fr)), 1);

  fs::remove(f);
}

TEST(PrintDiff, HeaderPrefixesAreAAndB) {
  const std::string src = "foo\nbar\n";
  const fs::path f = writeTempFile("header_prefix", src);
  const auto fr = makeFR(f, {.offset = 4U, .length = 3U}, "baz");
  const auto ls = splitLines(runDiff(fr));

  ASSERT_GE(ls.size(), 2U);
  EXPECT_TRUE(ls.at(0).starts_with("--- a/")) << ls.at(0);
  EXPECT_TRUE(ls.at(1).starts_with("+++ b/")) << ls.at(1);
  EXPECT_EQ(ls.at(0).substr(6), ls.at(1).substr(6));

  fs::remove(f);
}

TEST(PrintDiff, EmptyFileReplacementsProducesNoOutput) {
  const FileReplacements fr;
  EXPECT_TRUE(runDiff(fr).empty());
}

TEST(PrintDiff, NonexistentFileProducesNoOutput) {
  FileReplacements fr;
  Replacement r;
  r.filename = "/nonexistent/path/file.sv";
  r.offset = 0U;
  r.length = 3U;
  r.text = "xyz";
  r.rule_id = "TEST";
  ASSERT_TRUE(fr.add(r));
  EXPECT_TRUE(runDiff(fr).empty());
}
