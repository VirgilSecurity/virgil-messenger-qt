#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"

#
#   Global variables
#
CORE_VER=$(head -n 1 ${SCRIPT_FOLDER}/../VERSION_CORE)
ARCH_NAME=prebuilt-${CORE_VER}.tgz
PREBUILT_ARCHIVE="https://bintray.com/virgilsecurity/iotl-demo-cdn/download_file?file_path=${ARCH_NAME}"

export INSTALL_DIR="${SCRIPT_FOLDER}/../ext/"


#*************************************************************************************************************

print_message "CORE VERSION = ${CORE_VER}"

if [ -d ${INSTALL_DIR}/prebuilt ] && [ "${PREBUILT_SKIP}" == "true" ]; then
 print_message "Prebuild libraries found. Download skipped"
 exit 0
fi

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
