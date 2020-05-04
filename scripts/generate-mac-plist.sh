#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"
source ${SCRIPT_FOLDER}/ish/common.sh

# Sparkle
SUFeedURL="${1}"
SUPublicEDKey="${2}"
VERSION_PLIST="${3}"

#***************************************************************************************
function print_usage() {
    echo
    echo "${0} <SUFeedURL> <SUPublicEDKey> <Version>"
    echo
    echo "example: ${0} \"https://example.com/appcast.xml\" \"ab90ab90a9b09a9b09ab09\"" "0.1.0.100"
    echo
}

#***************************************************************************************
function fill_plist() {
    local PLIST_DIR="${SCRIPT_FOLDER}/../platforms/macos"
    local TEMPLATE_PLIST="${PLIST_DIR}/virgil-messenger.plist.in"
    local DST_PLIST="${PLIST_DIR}/virgil-messenger.plist"

    if [ -f ${DST_PLIST} ]; then
        rm "${DST_PLIST}"
    fi

    echo
    echo "=== Fill Info.plist for sparkle"
    sed -e "s,@VERSION@,${VERSION_PLIST},g" -e "s,@SUFeedURL@,${SUFeedURL},g" -e "s,@SUPublicEDKey@,${SUPublicEDKey},g" ${TEMPLATE_PLIST} >${DST_PLIST}
    check_error
}

#***************************************************************************************

if [ z"${SUFeedURL}" == "z" ]; then
    print_usage
    exit 1
fi

if [ z"${SUPublicEDKey}" == "z" ]; then
    print_usage
    exit 1
fi

if [ z"${VERSION_PLIST}" == "z" ]; then
    print_usage
    exit 1
fi

fill_plist
