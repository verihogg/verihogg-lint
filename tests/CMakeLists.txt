add_executable(class_checks_test $<TARGET_OBJECTS:verihoggLintLib> class_checks_test.cpp)

target_link_libraries(class_checks_test PUBLIC verihoggLintLib)
target_link_libraries(class_checks_test PUBLIC gtest gtest_main)
target_link_libraries(class_checks_test PUBLIC ${GTEST_LIBRARY})

target_include_directories(class_checks_test
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linter/include>
    ${SURELOG_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}
)

include(GoogleTest)
gtest_discover_tests(class_checks_test)

add_executable(non_standard_checks_test $<TARGET_OBJECTS:verihoggLintLib> Non_standard_check.cpp)

target_link_libraries(non_standard_checks_test PUBLIC verihoggLintLib)
target_link_libraries(non_standard_checks_test PUBLIC gtest gtest_main)
target_link_libraries(non_standard_checks_test PUBLIC ${GTEST_LIBRARY})

target_include_directories(non_standard_checks_test
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linter/include>
    ${SURELOG_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}
)

set(NON_STANDARD_CHECK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/NonStandardChecks")

target_compile_definitions(non_standard_checks_test PRIVATE
  VERIHOGG_TEST_NON_STANDARD_ROOT="${NON_STANDARD_CHECK_ROOT}"
)

gtest_discover_tests(non_standard_checks_test)

add_executable(assignment_pattern_checks_test $<TARGET_OBJECTS:verihoggLintLib> assignment_pattern_checks_test.cpp)

target_link_libraries(assignment_pattern_checks_test PUBLIC verihoggLintLib)
target_link_libraries(assignment_pattern_checks_test PUBLIC gtest gtest_main)
target_link_libraries(assignment_pattern_checks_test PUBLIC ${GTEST_LIBRARY})

target_include_directories(assignment_pattern_checks_test
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linter/include>
    ${SURELOG_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}
)

gtest_discover_tests(assignment_pattern_checks_test)
