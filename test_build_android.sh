#!/bin/bash
rm -rf build_test
mkdir -p build_test
pushd build_test
 cmake -DVS_PLATFORM="android" -DCMAKE_BUILD_TYPE="MinSizeRel" ..
 #-DVS_CUSTOMER="Virgil" ..
 make 
 make apk
popd
