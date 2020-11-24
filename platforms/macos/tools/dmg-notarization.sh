#!/bin/bash
set -o errtrace
#
#   Global variables
#
SCRIPT_FOLDER="$(cd $(dirname "$0") && pwd)"


#***************************************************************************************
function print_title() {
    echo
    echo "===================================="
    echo "=== ${PLATFORM} ${APPLICATION_NAME} build ${VERSION}"
    echo "=== Build type : ${BUILD_TYPE}"
    echo "=== Tool name : ${TOOL_NAME}"
    echo "=== Output directory : ${BUILD_DIR}"
    echo "=== Customer : ${PARAM_CUSTOMER}"
    echo "===================================="
    echo
}

#***************************************************************************************
function print_message() {
    echo
    echo "===================================="
    echo "=== ${1}"
    echo "===================================="
}

############################################################################################
print_usage() {
  echo
  echo "$(basename ${0})"
  echo
  echo "  -u < User name >          - User name"
  echo "  -p < Password  >          - Password"
  echo "  -i < PKG identify  >      - Pkg bundle identify"
  echo "  -f < Path to dmg file >   - DMG file path"
  echo "  -h                        - Print help"
  exit 0
}
############################################################################################
#
#  Script parameters
#
############################################################################################

while [ -n "$1" ]
 do
   case "$1" in
     -h) print_usage
         exit 0
         ;;
     -u) USER_NAME="$2"
         shift
         ;;
     -p) PASS="$2"
         shift
         ;;          
     -i) PKG_IDENTIFIER="$2"
         shift
         ;;          
     -f) DMG_FILE="$2"
         shift
         ;;          
     *) print_usage;;
   esac
   shift
done

#***************************************************************************************
function notarize_dmg() {
	print_message "Send Application for Apple's notarization"

	NOTARIZE_OUTPUT=$(xcrun altool -t osx -f "${DMG_FILE}" --primary-bundle-id "${PKG_IDENTIFIER}" --notarize-app --username ${USER_NAME} -p ${PASS} 2>&1)
	NOTARIZE_ID=$(echo ${NOTARIZE_OUTPUT} | tr -d "\n" | grep -F 'No errors uploading' | awk -F 'RequestUUID' '{print $2}' | awk -F ' ' '{print $2}')

	echo "NOTARIZE_ID = ${NOTARIZE_ID}"

	print_message "Get result of notarization"

	NOTARIZATION_DONE="false"
	for count in $(seq 1 300); do
		echo "Wait .. ${count} of 300"
		sleep 10s
		INFO_OUTPUT=$(xcrun altool --notarization-info "${NOTARIZE_ID}" --username ${USER_NAME} -p ${PASS} 2>&1 | tr -d "\n")

		if echo ${INFO_OUTPUT} | grep -q -F 'Status Message: Package Approved'; then
			NOTARIZATION_DONE="true"
			break
		fi

		if echo ${INFO_OUTPUT} | grep -q -F 'Status Message: Package Invalid'; then
			echo "${INFO_OUTPUT}"
			exit 1
		fi
	done

	if [ "${NOTARIZATION_DONE}" == "false" ]; then
		echo "${INFO_OUTPUT}"
		exit 1
	fi

	print_message "Staple result of the notarization"

	STAMPLE_OUTPUT=$(xcrun stapler staple -v "${DMG_FILE}" 2>&1 | tr -d "\n")

	if echo ${STAMPLE_OUTPUT} | grep -q -F 'The staple and validate action worked!'; then
		echo ""
	else
		echo "${STAMPLE_OUTPUT}"
		exit 1
	fi
}

#***************************************************************************************
notarize_dmg
