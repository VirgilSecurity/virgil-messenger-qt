#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

pushd ${PROJECT_FOLDER}

  rm -rf build
  mkdir -p build
  pushd build
   cmake -DVS_PLATFORM="macos" -DVS_CUSTOMER="Virgil"  ..
   make
   make dmg_debug
  popd

popd  

