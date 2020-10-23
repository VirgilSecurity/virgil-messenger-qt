#!/bin/bash

export QTDIR=/opt/Qt/5.15.0/clang_64

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

pushd ${PROJECT_FOLDER}

  rm -rf build
  mkdir -p build
  pushd build
   cmake -DVS_PLATFORM="macos" -DVS_CUSTOMER="Virgil" ..
   make
  popd

popd  

