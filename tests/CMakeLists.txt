add_executable(class_checks_test class_checks_test.cpp)

target_link_libraries(class_checks_test PUBLIC linter gtest)
target_link_libraries(class_checks_test PUBLIC ${GTEST_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${SURELOG_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${UHDM_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${CAPNP_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${ANTLR_LIBRARY})
target_link_libraries(class_checks_test PUBLIC ${ZLIB_LIBRARY})


include(GoogleTest)
gtest_discover_tests(class_checks_test)
add_test(NAME linter_test COMMAND class_checks_test)
