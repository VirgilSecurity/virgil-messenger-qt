#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"

#
#   Global variables
#
ARCH_NAME=prebuilt.tar.gz
PREBUILT_ARCHIVE="https://www.dropbox.com/s/1dtsu4udijl8wot/${ARCH_NAME}"
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

rm -rf ${INSTALL_DIR} || true
mkdir -p ${INSTALL_DIR}
check_error

pushd ${INSTALL_DIR}
wget ${PREBUILT_ARCHIVE}
check_error

tar -xvf ${ARCH_NAME}
check_error

rm ${ARCH_NAME}
popd
