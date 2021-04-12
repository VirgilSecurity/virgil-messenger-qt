#!/bin/bash

BINARY_PATH="${1}"
KEY_PATH="${2}"

export DSA_SIGN="$(openssl dgst -sha1 -binary < "${BINARY_PATH}" | openssl dgst -sha1 -sign ${KEY_PATH} | openssl enc -base64)"
export BUILD_DATE="$(LANG=C; date)"

echo "Build time: ${BUILD_DATE}"
echo "Installer sign: ${DSA_SIGN}"

j2 -f env -o appcast.xml appcast.xml
