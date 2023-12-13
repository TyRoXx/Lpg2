#!/usr/bin/env bash
BUILD_SUPER_DIR=`pwd`/..
DEPENDENCIES_DIR=$BUILD_SUPER_DIR/lpg2_dependencies

if [ -f /usr/local/bin/apt ]; then
    sudo apt install ninja-build g++ clang-format || exit 1
elif [ -f pacman ]; then
    sudo pacman -Syy curl zip unzip tar --no-confirm || exit 1
else
    echo "No supported package manager found"
fi

mkdir -p $DEPENDENCIES_DIR || exit 1
pushd $DEPENDENCIES_DIR || exit 1

INSTALL_PREFIX=`pwd`/install
rm -rf $INSTALL_PREFIX

# lcov
sudo perl -MCPAN -e 'install "DateTime"' || exit 1
LCOV_VERSION=2.0
if [[ ! -d lcov-$LCOV_VERSION ]]; then
    wget https://github.com/linux-test-project/lcov/releases/download/v$LCOV_VERSION/lcov-$LCOV_VERSION.tar.gz || exit 1
    tar zxvf lcov-$LCOV_VERSION.tar.gz || exit 1
fi
pushd lcov-$LCOV_VERSION || exit 1
make -j4 PREFIX=$INSTALL_PREFIX install || exit 1
popd || exit 1

popd || exit 1

pushd vcpkg || exit 1
export CC=gcc
export CXX=g++
./bootstrap-vcpkg.sh || exit 1
./vcpkg install `cat ../vcpkg_dependencies` || exit 1
./vcpkg upgrade --no-dry-run || exit 1
