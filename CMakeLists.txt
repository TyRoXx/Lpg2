cmake_minimum_required(VERSION 3.14)
project(Lpg2 LANGUAGES CXX)

if(MSVC)
    set(VCPKG_TARGET_TRIPLET x64-windows-static)
endif()
include(vcpkg/scripts/buildsystems/vcpkg.cmake)

if(UNIX)
    option(LPG2_WITH_GCOV "compile with gcov support" ON)
    if(LPG2_WITH_GCOV)
        set(DEPENDENCIES_INSTALL_DIR ${CMAKE_SOURCE_DIR}/../lpg2_dependencies/install)
        add_definitions(-coverage)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage")
        add_custom_target(testcoverage
            COMMAND
                find ${CMAKE_CURRENT_BINARY_DIR} -type f -name '*.gcda' -delete &&
                $<TARGET_FILE:tests> --run_test=* &&
                ${DEPENDENCIES_INSTALL_DIR}/bin/lcov --capture --no-external --gcov-tool /usr/bin/gcov-8 --directory ${CMAKE_CURRENT_SOURCE_DIR} --directory ${CMAKE_CURRENT_BINARY_DIR} --output-file coverage.info &&
                ${DEPENDENCIES_INSTALL_DIR}/bin/lcov --remove coverage.info '${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/*' --gcov-tool /usr/bin/gcov-8 --output-file coverage.info &&
                ${DEPENDENCIES_INSTALL_DIR}/bin/genhtml coverage.info --output-directory coverage-html &&
                firefox coverage-html/index.html
            WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR})
        add_dependencies(testcoverage tests)
    endif()
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system unit_test_framework filesystem REQUIRED)

find_package(benchmark REQUIRED)

if(UNIX)
    add_definitions(
        -Wall
        -Wextra
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wrestrict
        -Wnull-dereference
        -Wdouble-promotion
        #-Wshadow
        -fstrict-aliasing
        -Wformat=2
        -Wuseless-cast
        -pedantic
        -Werror
        -Wconversion
        -Wunused
        -Wundef
        -Wvla
        -Wswitch-enum
        -Wuninitialized
        -Wstrict-overflow=5
        -std=c++17
    )
endif()

include_directories(.)

add_subdirectory(lpg2)
add_subdirectory(benchmarks)
add_subdirectory(tests)
