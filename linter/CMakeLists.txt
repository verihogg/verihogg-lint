add_library(linter
  src/rules/extend_class.cpp
  src/utils/init.cpp
  src/utils/ast_utils.cpp
  src/utils/location_utils.cpp
)

target_link_libraries(linter PUBLIC surelog)
target_include_directories(linter PUBLIC include)
