#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"
BUILD_DIR="${SCRIPT_FOLDER}/../ext/android-libs"
LIBS_REPO="https://github.com/VirgilSecurity/openssl-curl-android.git"
LIBS_BRANCH="virgil"

#***************************************************************************************

function usage_example() {
    echo "Example: ${0} $HOME/Library/Android/sdk/ndk/20.1.5948944 darwin-x86_64 com.virgilsecurity.qtmessenger"
}

#***************************************************************************************
check_error() {
   RETRES=$?
   if [ $RETRES != 0 ]; then
        echo "----------------------------------------------------------------------"
        echo "############# !!! PROCESS ERROR ERRORCODE=[$RETRES]  #################"
        echo "----------------------------------------------------------------------"
        [ "$1" == "0" ] || exit $RETRES
   else
        echo "-----# Process OK. ---------------------------------------------------"
   fi
   return $RETRES
}

#***************************************************************************************

#
#    Check input parameters
#

ANDROID_NDK_HOME="${1}"
HOST_TAG="${2}"

if [ -z "$ANDROID_NDK_HOME" ] || [ ! -d ${ANDROID_NDK_HOME} ]; then
    echo "Wrong NDK path: ${ANDROID_NDK_HOME}"
    usage_example
    exit 1
fi

if [ -z "$HOST_TAG" ]; then
    echo "Host tag is not set: ${HOST_TAG}"
    usage_example
    exit 1
fi

#
# Prepare directory
#
if [ -d ${BUILD_DIR} ]; then
     rm -rf ${BUILD_DIR}
fi
mkdir ${BUILD_DIR}
check_error

pushd ${BUILD_DIR}
     #
     #    Clone repository
     #
     git clone --recursive -b ${LIBS_BRANCH} ${LIBS_REPO}
     check_error

     #
     #    Build for all Android architectures
     #
     pushd openssl-curl-android
          ./build.sh /Users/kutashenko/Library/Android/sdk/ndk/20.1.5948944 darwin-x86_64 com.virgilsecurity.qtmessenger
          check_error
     popd
popd