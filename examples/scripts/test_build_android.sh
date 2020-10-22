#!/bin/bash
rm -rf build_test
mkdir -p build_test
pushd build_test
 cmake -DVS_PLATFORM="android" ..
 make 
 make apk
popd
