add_executable(class_checks_test $<TARGET_OBJECTS:verihoggLintLib> class_checks_test.cpp)

target_link_libraries(class_checks_test PUBLIC verihoggLintLib)
target_link_libraries(class_checks_test PUBLIC gtest gtest_main)
target_link_libraries(class_checks_test PUBLIC ${GTEST_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${SURELOG_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${UHDM_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${CAPNP_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${ANTLR_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${ZLIB_LIBRARY})

target_include_directories(class_checks_test
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linter/include>
    ${SURELOG_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}
)

include(GoogleTest)
gtest_discover_tests(class_checks_test)
