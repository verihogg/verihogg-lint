cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(verihogg-lint VERSION 1.0 LANGUAGES CXX)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SURELOG_ROOT "" CACHE PATH "Path to Surelog installation prefix")

find_path(SURELOG_INCLUDE_DIR
  NAMES Surelog/API/Surelog.h
  HINTS ${SURELOG_ROOT} ENV SURELOG_ROOT
  PATH_SUFFIXES include
)

find_library(SURELOG_LIBRARY
  NAMES surelog
  HINTS ${SURELOG_ROOT} ENV SURELOG_ROOT
  PATH_SUFFIXES lib lib64
)

set(UHDM_ROOT "" CACHE PATH "Path to UHDM installation prefix")
set(ANTLR4_ROOT "" CACHE PATH "Path to ANTLR4 C++ runtime prefix")
set(ZLIB_ROOT "" CACHE PATH "Path to zlib installation prefix")

find_path(UHDM_INCLUDE_DIR
  NAMES uhdm/SymbolId.h
  HINTS ${UHDM_ROOT} ENV UHDM_ROOT
  PATH_SUFFIXES include
)

find_library(UHDM_LIBRARY
  NAMES uhdm
  HINTS ${UHDM_ROOT} ENV UHDM_ROOT
  PATH_SUFFIXES lib lib64
)

find_library(CAPNP_LIBRARY
  NAMES capnp
  HINTS ${CAPNP_ROOT} ENV CAPNP_ROOT
  PATH_SUFFIXES lib lib64
)

find_path(ANTLR_INCLUDE_DIR
  NAMES antlr4-runtime.h
  HINTS ${ANTLR4_ROOT} ENV ANTLR4_ROOT
  PATH_SUFFIXES include/antlr4-runtime include
)

find_library(ANTLR_LIBRARY
  NAMES antlr4-runtime antlr4_runtime
  HINTS ${ANTLR4_ROOT} ENV ANTLR4_ROOT
  PATH_SUFFIXES lib lib64
)

find_library(ZLIB_LIBRARY
  NAMES z zlib
  HINTS ${ZLIB_ROOT} ENV ZLIB_ROOT
  PATH_SUFFIXES lib lib64
)

if(NOT SURELOG_INCLUDE_DIR OR NOT SURELOG_LIBRARY)
  message(FATAL_ERROR
    "Surelog was not found. Install Surelog and set SURELOG_ROOT (or pass -DSURELOG_ROOT=<prefix>).")
endif()

if(NOT UHDM_INCLUDE_DIR OR NOT UHDM_LIBRARY)
  message(FATAL_ERROR
    "UHDM headers were not found. Install UHDM and set UHDM_ROOT (or pass -DUHDM_ROOT=<prefix>).")
endif()

if (NOT CAPNP_LIBRARY)
  message(FATAL_ERROR
    "Cap'n Proto library was not found. Install Cap'n Proto and set CAPNP_ROOT (or pass -DCAPNP_ROOT=<prefix>).")
endif()

if(NOT ANTLR_LIBRARY)
  message(FATAL_ERROR
    "ANTLR4 runtime library was not found. Install the ANTLR4 C++ runtime and set ANTLR4_ROOT (or pass -DANTLR4_ROOT=<prefix>).")
endif()

if(NOT ZLIB_LIBRARY)
  message(FATAL_ERROR
    "zlib was not found. Install zlib and set ZLIB_ROOT (or pass -DZLIB_ROOT=<prefix>).")
endif()

set(LINTER_SRC
    linter/src/main/lint.cpp
    linter/src/main/cli.cpp
    linter/src/utils/ast_utils.cpp
    linter/src/utils/location_utils.cpp
    linter/src/utils/name_utils.cpp
    linter/src/utils/string_utils.cpp
    linter/src/utils/design_utils.cpp
    linter/src/utils/class_scope_utils.cpp
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
    linter/src/rules/time_value.cpp
    linter/src/rules/multiple_bins.cpp
    linter/src/rules/assertion_statement_atribute_instance.cpp
    linter/src/rules/system_function_arguments.cpp
    linter/src/rules/wildcard_operator.cpp
    linter/src/rules/exponent_format_time_value.cpp
    linter/src/rules/nof_parameter_override.cpp
    linter/src/rules/missing_function_implementation.cpp
    linter/src/rules/function_implemention_scope.cpp
    linter/src/rules/extend_class.cpp
    linter/src/rules/duplicate_constructor.cpp
    linter/src/rules/duplicate_class.cpp    
    
)
#     linter/src/rules/extern_task_undeclared.cpp
#     linter/src/rules/extern_function_undeclared.cpp
#     linter/src/rules/extern_constraint_undeclared.cpp

foreach(f ${LINTER_SRC})
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
    message(FATAL_ERROR "Linter source file not found: ${f}\nCheck for typos/incorrect path in CMakeLists.txt")
  endif()
endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(${PROJECT_NAME} ${LINTER_SRC})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linter/include>
    ${SURELOG_INCLUDE_DIR}
    ${UHDM_INCLUDE_DIR}
    ${ANTLR_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${SURELOG_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${UHDM_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CAPNP_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${ANTLR_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${ZLIB_LIBRARY})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /bigobj)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Install rule
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

