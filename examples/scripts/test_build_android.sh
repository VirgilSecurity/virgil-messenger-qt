#!/bin/bash

SCRIPT_FOLDER="$(cd $(dirname "$0") && pwd)"
PROJECT_FOLDER="${SCRIPT_FOLDER}/../.."

pushd ${PROJECT_FOLDER}

  rm -rf build
  mkdir -p build

  pushd build
#   cmake -DVS_PLATFORM="android" ..
#   make 
#   make apk
   
        cmake   -DVS_PLATFORM="android" \
                -DCMAKE_BUILD_TYPE="MinSizeRel" \
                -DVS_KEYCHAIN="/root/WORK/2/virgil-messenger.keystore"  -DVS_KEY_ALIAS="upload" \
                -DVS_KEYCHAIN_PASSWORD="wdKJHrf370jbBDSc" \
                ..
        make
        make apk_release
   
   
  popd

popd  

#                -DVS_KEY_PASSWORD="wdKJHrf370jbBDSc" \