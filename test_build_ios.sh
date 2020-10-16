#!/bin/bash

export QTDIR=/opt/Qt/5.15.0/clang_64

rm -rf build
mkdir -p build

cmake -S. -Bbuild -GXcode -DVS_PLATFORM="ios" -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="JWNLQ3HC5A" 
cmake --build build --config Release --target VirgilMessenger

#pushd build
# cmake -DVS_PLATFORM="ios" -DCMAKE_SYSTEM_NAME=iOS -DVS_CUSTOMER="Virgil" -DCMAKE_OSX_ARCHITECTURES="arm64" -GXcode  ..
# make
#popd

#-DCMAKE_OSX_ARCHITECTURES="armv7;armv7s;arm64;i386;x86_64"  ..
-DCMAKE_SYSTEM_NAME=iOS -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO -DCMAKE_IOS_INSTALL_COMBINED=YES