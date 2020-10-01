#!/bin/bash
rm -rf build-virgil
mkdir -p build-virgil
pushd build-virgil
 cmake -DVS_PLATFORM="android" ..
 make 
 make apk
popd

rm -rf build-area52
mkdir -p build-area52
pushd build-area52
 cmake -DVS_PLATFORM="android" ..
 make 
 make apk
popd
