cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(MY_LINTER VERSION 1.0 LANGUAGES CXX)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()

# Use C++17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/Surelog)

set(LINTER_SRC
    linter/src/main/lint.cpp
    linter/src/utils/ast_utils.cpp
    linter/src/utils/location_utils.cpp
    linter/src/utils/name_utils.cpp
    linter/src/utils/string_utils.cpp
    linter/src/rules/class_variable_lifetime.cpp
    linter/src/rules/dpi_decl_string.cpp
    linter/src/rules/fatal_rule.cpp
    linter/src/rules/hierarchical_interface_identifier.cpp
    linter/src/rules/implicit_data_type.cpp
    linter/src/rules/parameter_dynamic_array.cpp
    linter/src/rules/prototype_return_data_type.cpp
    linter/src/rules/repeat_in_sequence.cpp
    linter/src/main/rule_dispatcher.cpp
    linter/src/rules/coverpoint_expression_type.cpp
    linter/src/rules/expression_covergroup.cpp
    linter/src/rules/concatenation_multiplier.cpp
    linter/src/rules/parametr_override.cpp
    linter/src/rules/multiple_dot_star_connection.cpp
    linter/src/rules/select_in_event_control.cpp
    linter/src/rules/empty_assignment_pattern.cpp
    linter/src/rules/missing_for_loop_initialization.cpp
    linter/src/rules/missing_for_loop_condition.cpp
    linter/src/rules/missing_for_loop_step.cpp
    linter/src/rules/foreach_loop_condition.cpp
    linter/src/rules/select_in_weight.cpp
    linter/src/rules/assignment_pattern.cpp
    linter/src/rules/assignment_pattern_context.cpp
    linter/src/rules/scalar_assignment_pattern.cpp
    linter/src/rules/target_array_unpacked_concatenation.cpp
    linter/src/rules/inside_operator.cpp
    linter/src/rules/inside_operator_range.cpp
    linter/src/rules/type_casting.cpp

)

foreach(f ${LINTER_SRC})
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    message(FATAL_ERROR "Linter source file not found: ${f}\nCheck for typos/incorrect path in CMakeLists.txt")
  endif()
endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(lint ${LINTER_SRC})

target_include_directories(lint
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linter/include>
)

target_link_libraries(lint PUBLIC surelog)

if(MSVC)
    target_compile_options(lint PRIVATE /W4 /bigobj)
else()
    target_compile_options(lint PRIVATE -Wall -O3 -Wextra -Wpedantic)
endif()

# Install rule
include(GNUInstallDirs)
install(TARGETS lint
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

