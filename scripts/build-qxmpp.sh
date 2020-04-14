#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"
QXMPP_DIR="${SCRIPT_FOLDER}/../ext/qxmpp"
BUILD_DIR_BASE="${QXMPP_DIR}"
CMAKE_CUSTOM_PARAM="${@:2}"


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

#
#   Build
#
function build() {
    local BUILD_TYPE=$1
    local CMAKE_ARGUMENTS=$2
    local CORES=10

    local BUILD_DIR=${BUILD_DIR_BASE}/cmake-build-${QT_BUILD_DIR_SUFFIX}/${BUILD_TYPE}
    local INSTALL_DIR=${QT_INSTALL_DIR_BASE}/${QT_BUILD_DIR_SUFFIX}/${BUILD_TYPE}/installed

    echo
    echo "===================================="
    echo "=== ${QT_BUILD_DIR_SUFFIX} ${BUILD_TYPE} build"
    echo "=== Output directory: ${BUILD_DIR}"
    echo "===================================="
    echo

    rm -rf ${BUILD_DIR}
    mkdir -p ${BUILD_DIR}
    mkdir -p ${INSTALL_DIR}

    pushd ${BUILD_DIR}
      # prepare to build
      cmake ${BUILD_DIR_BASE} ${CMAKE_ARGUMENTS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "Unix Makefiles"
      check_error

      # build all targets
      make -j ${CORES}
      check_error

      # install all targets
      make DESTDIR=${INSTALL_DIR} install
      check_error
      if [ -d ${INSTALL_DIR}/usr/local/lib64 ]; then 
         cp -rf ${INSTALL_DIR}/usr/local/lib64/* ${INSTALL_DIR}/usr/local/lib 
         check_error
         rm -rf ${INSTALL_DIR}/usr/local/lib64
         check_error
      fi 

    popd
}

# Common CMake arguments for the project
CMAKE_ARGUMENTS=" \
-DBUILD_SHARED=OFF \
-DBUILD_EXAMPLES=OFF \
-DBUILD_TESTS=OFF \
-DWITH_OPUS=OFF \
-DWITH_VPX=OFF \
-DCMAKE_PREFIX_PATH=${1} \
${CMAKE_CUSTOM_PARAM} \
"

echo "${CMAKE_ARGUMENTS}"

#
#   Build both Debug and Release
#
build "release" "${CMAKE_ARGUMENTS}"
