file(GLOB sources *.h *.cpp ../common/*)
add_executable(tests ${sources})
target_link_libraries(tests Boost::unit_test_framework)
