#!/bin/bash
rm -rf build
mkdir -p build
pushd build
 cmake -DCMAKE_BUILD_TYPE="Release" -DVS_PLATFORM="android" -DVS_CUSTOMER="Virgil" ..
 make 
 make apk
popd
