file(GLOB sources *.h *.cpp)
add_executable(tests ${sources})
target_link_libraries(tests lpg2 Catch2::Catch2WithMain)
