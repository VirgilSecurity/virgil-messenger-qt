#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"

#
#   Global variables
#
CORE_VER=$(head -n 1 ${SCRIPT_FOLDER}/../VERSION_CORE)
ARCH_NAME=prebuilt-${CORE_VER}.tgz
PREBUILT_ARCHIVE="https://bintray.com/virgilsecurity/iotl-demo-cdn/download_file?file_path=${ARCH_NAME}"

export INSTALL_DIR="${SCRIPT_FOLDER}/../ext/"

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

#*************************************************************************************************************

echo
echo "=== CORE VERSION = ${CORE_VER}"
echo

rm -rf ${INSTALL_DIR} || true
mkdir -p ${INSTALL_DIR}
check_error

pushd ${INSTALL_DIR}
wget -O ${ARCH_NAME} ${PREBUILT_ARCHIVE}
check_error

tar -xvf ${ARCH_NAME}
check_error

rm ${ARCH_NAME}
popd
