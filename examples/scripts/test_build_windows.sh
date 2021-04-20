#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

export LANG=C

pushd ${PROJECT_FOLDER}

  rm -rf build
  mkdir -p build 

  pushd build
   cmake -DCMAKE_BUILD_TYPE="Debug" -DVS_PLATFORM="windows" -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw64.cmake -DVS_WINSPARKE_KEY="${HOME}/keys/windows/dsa_priv.pem" ..
   make -j10
   make deploy
  popd
popd
