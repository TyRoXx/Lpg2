#!/usr/bin/env bash

if [[ "$#" != 1 ]]; then
    echo "Please provide enough arguments"
    echo "Usage: $0 <build directory>"
    exit 1
fi

BUILD_SUPER_DIR=$1
DEPENDENCIES_DIR=$BUILD_SUPER_DIR/lpg2_dependencies

if apt; then
    sudo apt install ninja-build g++-8 clang-format-3.9 || exit 1
elif pacman; then
    sudo pacman -Syy curl zip unzip tar --no-confirm || exit 1
else
    echo "No supported package manager found"
fi

mkdir -p $DEPENDENCIES_DIR || exit 1
pushd $DEPENDENCIES_DIR || exit 1

INSTALL_PREFIX=`pwd`/install.temp
rm -rf $INSTALL_PREFIX

# lcov
LCOV_VERSION=1.14
if [[ ! -d lcov-$LCOV_VERSION ]]; then
    wget https://github.com/linux-test-project/lcov/releases/download/v$LCOV_VERSION/lcov-$LCOV_VERSION.tar.gz || exit 1
    tar zxvf lcov-$LCOV_VERSION.tar.gz || exit 1
fi
pushd lcov-$LCOV_VERSION || exit 1
make -j4 PREFIX=$INSTALL_PREFIX install || exit 1
popd || exit 1

rm -rf install
mv install.temp install || exit 1

popd || exit 1

pushd vcpkg || exit 1
export CC=gcc
export CXX=g++
./bootstrap-vcpkg.sh || exit 1
./vcpkg install `cat ../vcpkg_dependencies` || exit 1
