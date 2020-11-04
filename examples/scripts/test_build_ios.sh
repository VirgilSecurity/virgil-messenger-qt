#!/bin/bash

export QTDIR=/opt/Qt/5.15.0/clang_64

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

pushd ${PROJECT_FOLDER}

  rm -rf build-sim
  mkdir -p build-sim
  
  rm -rf build
  mkdir -p build

#  cmake -S. -Bbuild-sim -GXcode -DVS_PLATFORM="ios" -DVS_IOS_SIMULATOR=TRUE -DVS_DEVELOPMENT_TEAM="JWNLQ3HC5A"
#  cmake --build build-sim --config Debug 
  
  cmake -S. -Bbuild -GXcode -DVS_PLATFORM="ios" -DVS_DEVELOPMENT_TEAM="JWNLQ3HC5A"
  cmake --build build --config Release

popd  
