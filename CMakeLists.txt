cmake_minimum_required(VERSION 3.24)
project(Lpg2 LANGUAGES CXX)

if(MSVC)
    set(VCPKG_TARGET_TRIPLET vcpkg-msvc)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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
                $<TARGET_FILE:tests> &&
                ${DEPENDENCIES_INSTALL_DIR}/bin/lcov --capture --gcov-tool /usr/bin/gcov-11 --config-file ${CMAKE_CURRENT_SOURCE_DIR}/lcovrc --directory ${CMAKE_CURRENT_BINARY_DIR} --output-file coverage.info &&
                ${DEPENDENCIES_INSTALL_DIR}/bin/lcov --remove coverage.info '${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/*' --gcov-tool /usr/bin/gcov-11 --config-file ${CMAKE_CURRENT_SOURCE_DIR}/lcovrc --output-file coverage.info &&
                ${DEPENDENCIES_INSTALL_DIR}/bin/genhtml coverage.info --config-file ${CMAKE_CURRENT_SOURCE_DIR}/lcovrc --output-directory coverage-html &&
                firefox coverage-html/index.html
            WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR})
        add_dependencies(testcoverage tests)
    endif()
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system REQUIRED)

add_definitions(-DBENCHMARK_STATIC_DEFINE)
find_package(benchmark REQUIRED)

find_package(Catch2 3 REQUIRED)

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
        -std=c++20
    )
endif()

if(MSVC)
    add_definitions(/std:c++20)
    # disable the useless "warnings"
    add_definitions(/Wall /WX /wd5262 /wd5045 /wd4514 /wd4711 /wd4710 /wd4820 /wd5031 /wd4625 /wd4626 /wd5027 /wd4464 /wd4668 /wd4623 /wd5243 /wd4619 /wd5026 /wd4266 /wd4371 /wd4866 /wd4868)
    # 'return': conversion from '__int64' to 'size_t', signed/unsigned mismatch
    add_definitions(/wd4365)
endif()

find_program(LPG2_CLANG_FORMAT NAMES clang-format PATHS "C:/Program Files/LLVM/bin")
if(LPG2_CLANG_FORMAT)
    file(GLOB_RECURSE formatted
        benchmarks/**.cpp benchmarks/**.h
        tests/**.cpp tests/**.h
        lpg2/**.cpp lpg2/**.h
    )
    add_custom_target(clang-format COMMAND "${LPG2_CLANG_FORMAT}" -i ${formatted} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

include_directories(.)

add_subdirectory(lpg2)
add_subdirectory(benchmarks)
add_subdirectory(tests)
