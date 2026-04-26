#include <gtest/gtest.h>

#include "fix/source_manager.h"

TEST(FixSourceManager, IsLoadedFalseByDefault) {
  FixSourceManager sm;
  EXPECT_FALSE(sm.isLoaded("any/file.sv"));
}

TEST(FixSourceManager, GetLineUnloadedReturnsEmpty) {
  FixSourceManager sm;
  EXPECT_EQ(sm.getLine("not/loaded.sv", 1), "");
}

TEST(FixSourceManager, GetOffsetUnloadedThrows) {
  FixSourceManager sm;
  EXPECT_THROW((void)sm.getOffset("not/loaded.sv", 1, 1), std::out_of_range);
}

TEST(FixSourceManager, GetSliceUnloadedReturnsEmpty) {
  FixSourceManager sm;
  EXPECT_EQ(sm.getSlice("not/loaded.sv", LineCol{1, 0}, 10), "");
}

TEST(FixSourceManager, RangeLengthUnloadedReturnsZero) {
  FixSourceManager sm;
  EXPECT_EQ(sm.rangeLength("not/loaded.sv", LineRange{{1, 1}, {1, 5}}), 0U);
}
