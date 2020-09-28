#!/bin/bash
rm -rf build
mkdir -p build 
pushd build
 cmake -DVS_PLATFORM="android" ..
 make 
 make apk
popd
