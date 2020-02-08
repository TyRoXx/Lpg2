file(GLOB sources *.cpp)
add_executable(benchmarks ${sources})
target_link_libraries(benchmarks benchmark::benchmark)
