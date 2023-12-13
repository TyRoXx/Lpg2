#!/usr/bin/env bash
BUILD_DIR=../build-Lpg2
./install_dependencies.sh || exit 1
./vcpkg/downloads/tools/cmake-3.27.1-linux/cmake-3.27.1-linux-x86_64/bin/cmake -B $BUILD_DIR -S . -G "CodeBlocks - Ninja" || exit 1
