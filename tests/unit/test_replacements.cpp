#include <gtest/gtest.h>

#include "fix/replacement.h"

static auto makeR(
    unsigned offset,  // NOLINT(bugprone-easily-swappable-parameters)
    unsigned length,  // NOLINT(bugprone-easily-swappable-parameters)
    const std::string& text, const std::string& rule = "R") -> Replacement {
  Replacement r;
  r.filename = "test.sv";
  r.offset = offset;
  r.length = length;
  r.text = text;
  r.rule_id = rule;
  return r;
}

TEST(Replacements, FirstAddAlwaysSucceeds) {
  Replacements repls;
  EXPECT_TRUE(repls.add(makeR(10, 5, "X")));
}

TEST(Replacements, OverlappingRangesConflict) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(10, 5, "X", "R1")));
  std::string msg;
  EXPECT_FALSE(repls.add(makeR(12, 3, "Y", "R2"), &msg));
  EXPECT_FALSE(msg.empty());
}

TEST(Replacements, AdjacentRangesNoConflict) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(10, 5, "X")));
  EXPECT_TRUE(repls.add(makeR(15, 3, "Y")));
}

TEST(Replacements, TwoIdenticalInsertionsSamePointDeduped) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(5, 0, "A")));
  EXPECT_TRUE(repls.add(makeR(5, 0, "A")));
}

TEST(Replacements, TwoInsertionsSamePointDifferentTextNoConflict) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(5, 0, "A")));
  EXPECT_TRUE(repls.add(makeR(5, 0, "B")));
}

TEST(Replacements, InsertionBeforeReplacementSameOffsetNoConflict) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(10, 0, "A")));
  EXPECT_TRUE(repls.add(makeR(10, 5, "B")));
}

TEST(Replacements, ApplyEndToStart) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(12, 3, "AND")));
  ASSERT_TRUE(repls.add(makeR(4, 3, "BAR")));
  EXPECT_EQ(repls.apply("say foo and bar done"), "say BAR and AND done");
}

TEST(Replacements, ApplyRemoval) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(4, 10, "")));
  EXPECT_EQ(repls.apply("say automatic logic;"), "say logic;");
}

TEST(Replacements, ApplyInsertion) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(4, 0, "automatic ")));
  EXPECT_EQ(repls.apply("say logic;"), "say automatic logic;");
}

TEST(Replacements, ApplyOutOfBoundsThrows) {
  Replacements repls;
  ASSERT_TRUE(repls.add(makeR(100, 5, "X")));
  EXPECT_THROW((void)repls.apply("short"), std::out_of_range);
}

TEST(FileReplacements, DuplicateFixAccepted) {
  FileReplacements fr;
  const auto r = makeR(10, 5, "X", "R1");
  ASSERT_TRUE(fr.add(r));
  EXPECT_TRUE(fr.add(r));
}

TEST(FileReplacements, DifferentTextSameRangeIsConflict) {
  FileReplacements fr;
  ASSERT_TRUE(fr.add(makeR(10, 5, "X", "R1")));
  std::string msg;
  EXPECT_FALSE(fr.add(makeR(10, 5, "Y", "R2"), &msg));
  EXPECT_FALSE(msg.empty());
}

TEST(FixIt, InsertInvalidLocationThrows) {
  EXPECT_THROW(FixIt::Insert({}, "text", "desc"), std::invalid_argument);
}

TEST(FixIt, InsertEmptyDescThrows) {
  EXPECT_THROW(FixIt::Insert({"f.sv", 1, 1}, "text", ""),
               std::invalid_argument);
}

TEST(FixIt, RemoveCrossFileThrows) {
  FixRange r;
  r.begin = {.filename = "a.sv", .line = 1, .col = 1};
  r.end = {.filename = "b.sv", .line = 1, .col = 1};
  EXPECT_THROW(FixIt::Remove(r, "desc"), std::invalid_argument);
}

TEST(FixIt, RemoveEndBeforeBeginSameLineThrows) {
  constexpr unsigned kLine = 5;
  constexpr unsigned kBeginCol = 10;
  constexpr unsigned kEndCol = 3;
  FixRange r;
  r.begin = {.filename = "f.sv", .line = kLine, .col = kBeginCol};
  r.end = {.filename = "f.sv", .line = kLine, .col = kEndCol};
  EXPECT_THROW(FixIt::Remove(r, "desc"), std::invalid_argument);
}

TEST(FixIt, InsertValidRange) {
  const auto fix = FixIt::Insert({.filename = "f.sv", .line = 1, .col = 1},
                                 "logic ", "insert logic");
  EXPECT_EQ(fix.kind, FixKind::Insertion);
  EXPECT_EQ(fix.replacement, "logic ");
  EXPECT_EQ(fix.description, "insert logic");
  EXPECT_EQ(fix.range.begin.col, fix.range.end.col);
}
