#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

pushd ${PROJECT_FOLDER}

  rm -rf build
  mkdir -p build 

  pushd build
   cmake -DVS_PLATFORM="windows" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw64.cmake ..
   make -j10
   make deploy
  popd
popd
