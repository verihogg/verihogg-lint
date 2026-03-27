add_library(linter
  src/rules/extend_class.cpp
  src/rules/duplicate_class.cpp
  src/rules/duplicate_constructor.cpp
  src/rules/extern_constraint_undeclared.cpp
  src/rules/extern_function_undeclared.cpp
  src/rules/extern_task_undeclared.cpp
  src/rules/extend_interface_class.cpp
  src/rules/implement_class.cpp
  src/rules/implement_interface_class.cpp
  src/utils/init.cpp
  src/utils/ast_utils.cpp
  src/utils/location_utils.cpp
)

target_link_libraries(linter PUBLIC surelog)
target_include_directories(linter PUBLIC include)
