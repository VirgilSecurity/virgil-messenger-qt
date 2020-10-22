#!/bin/bash
rm -rf build
mkdir -p build 

pushd build
 cmake -DVS_PLATFORM="linux" .. 
 make 
popd