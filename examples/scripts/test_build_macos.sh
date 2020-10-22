#!/bin/bash

export QTDIR=/opt/Qt/5.15.0/clang_64

rm -rf build
mkdir -p build
pushd build
 cmake -DVS_PLATFORM="macos" -DVS_CUSTOMER="Virgil" ..
 make
popd

