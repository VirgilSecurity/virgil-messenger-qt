#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"
QXMPP_DIR="${SCRIPT_FOLDER}/../ext/qxmpp"
BUILD_DIR_BASE="${QXMPP_DIR}"

#
#   Arguments
#
PLATFORM="host"

#
#   Build
#
function build() {
    local BUILD_TYPE=$1
    local CMAKE_ARGUMENTS=$2
    local CORES=10

    local BUILD_DIR=${BUILD_DIR_BASE}/cmake-build-${PLATFORM}/${BUILD_TYPE}
    local INSTALL_DIR=${BUILD_DIR_BASE}/cmake-build-${PLATFORM}/${BUILD_TYPE}/installed
    local LIBS_DIR=${INSTALL_DIR}/usr/local/lib

    echo
    echo "===================================="
    echo "=== ${PLATFORM} ${BUILD_TYPE} build"
    echo "=== Output directory: ${BUILD_DIR}"
    echo "===================================="
    echo

    rm -rf ${BUILD_DIR}
    mkdir -p ${BUILD_DIR}
    mkdir -p ${INSTALL_DIR}

    pushd ${BUILD_DIR}
      # prepare to build
      cmake ${BUILD_DIR_BASE} ${CMAKE_ARGUMENTS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "Unix Makefiles"

      # build all targets
      make -j ${CORES}

      # install all targets
      make DESTDIR=${INSTALL_DIR} install

    popd
}

# Common CMake arguments for the project
CMAKE_ARGUMENTS=" \
-DBUILD_SHARED=OFF \
-DBUILD_EXAMPLES=OFF \
-DBUILD_TESTS=OFF \
-DWITH_OPUS=ON \
-DWITH_VPX=ON \
-DCMAKE_PREFIX_PATH=${1}"

echo "${CMAKE_ARGUMENTS}"

#
#   Build both Debug and Release
#
build "release" "${CMAKE_ARGUMENTS}"
