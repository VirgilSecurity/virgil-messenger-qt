#!/bin/bash
rm -rf build
mkdir -p build
pushd build
 cmake -DVS_PLATFORM="android" -DVS_CUSTOMER="Virgil" ..
 make 
 make apk
popd
