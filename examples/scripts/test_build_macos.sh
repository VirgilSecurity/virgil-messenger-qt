#!/bin/bash

export QTDIR=/opt/Qt/5.15.0/clang_64

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

pushd ${PROJECT_FOLDER}

  rm -rf build
  mkdir -p build
  pushd build
   cmake -DVS_PLATFORM="macos" -DVS_CUSTOMER="Virgil" -DVS_MACOS_IDENT="68125DC3B6986E51D7702695CD4CA6A7C2058FA4" ..
   make
   make deploy
  popd

popd  

