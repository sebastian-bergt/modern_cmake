#!/bin/bash

set -euxo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

ROOT_DIR=$SCRIPT_DIR
BUILD_DIR=$ROOT_DIR/build
INSTALL_DIR=$ROOT_DIR/build/install
TOOLCHAIN=$ROOT_DIR/cmake/toolchain_linux_x86_64_gcc8.cmake

CONAN_PROFILE=$ROOT_DIR/conan/linux_x86_64_gcc8
BUILD_SHARED=ON
BUILD_TYPE=Release

mkdir -p $BUILD_DIR
rm -rf $BUILD_DIR/*
cd $BUILD_DIR

# export CONAN_CMAKE_TOOLCHAIN_FILE=$TOOLCHAIN

conan install .. -s build_type=$BUILD_TYPE --build=missing -pr=$CONAN_PROFILE

cmake .. -GNinja \
-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
-DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
-DCMAKE_MODULE_PATH=$BUILD_DIR \
-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
-DBUILD_SHARED_LIBS=$BUILD_SHARED

cmake --build .

ctest

cmake --build . --target install

# cpack -G DEB
cpack -G TGZ

# DEPENDENCY_INSTALL_DEB_FILE=$BUILD_DIR/modern_cmake_1.0.0_amd64.deb
DEPENDENCY_INSTALL_TGZ_FILE=$BUILD_DIR/modern_cmake-1.0.0-Linux.tar.gz

cpack -G TGZ --config CPackSourceConfig.cmake


CUSTOMER_ROOT_DIR=$ROOT_DIR/customer
CUSTOMER_BUILD_DIR=$CUSTOMER_ROOT_DIR/build
CUSTOMER_DEPENDENCY_INSTALL_DIR=$CUSTOMER_ROOT_DIR/build/install

mkdir -p $CUSTOMER_BUILD_DIR
rm -rf $CUSTOMER_BUILD_DIR/*
cd $CUSTOMER_BUILD_DIR

# dpkg-deb -x $DEPENDENCY_INSTALL_DEB_FILE $CUSTOMER_DEPENDENCY_INSTALL_DIR
mkdir -p $CUSTOMER_DEPENDENCY_INSTALL_DIR && tar -xvzf $DEPENDENCY_INSTALL_TGZ_FILE -C $CUSTOMER_DEPENDENCY_INSTALL_DIR

cmake .. -GNinja -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH=$CUSTOMER_DEPENDENCY_INSTALL_DIR

cmake --build .

ctest
