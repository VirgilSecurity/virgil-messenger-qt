#!/bin/bash
set -e
#
#   Global variables
#
SCRIPT_FOLDER="$(cd $(dirname "$0") && pwd)"

#***************************************************************************************
print_message() {
    echo
    echo "===================================="
    echo "=== ${1}"
    echo "===================================="
}

print_notarize_status() {
	echo "============================================"    
	echo "=== Get status state:	$(echo "${INFO_OUTPUT}" | jq -r '."success-message"')"
	echo "=== Status message:	$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."Status Message"')"
	echo "=== Status URL:		$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."LogFileURL"')"
	echo "=== Date:			$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."Date"')"
	echo "=== Status:		$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."Status"')"	
	echo "=== Status code:		$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."Status Code"')"
	echo "============================================"
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

	NOTARIZE_OUTPUT=$(xcrun altool -t osx -f "${DMG_FILE}" --output-format json --primary-bundle-id "${PKG_IDENTIFIER}" --notarize-app --username ${USER_NAME} -p ${PASS})
	NOTARIZE_ID="$(echo "${NOTARIZE_OUTPUT}" | jq -r '."notarization-upload"."RequestUUID"')"

	echo "============================================"
    	echo "=== Tool version: 	$(echo "${NOTARIZE_OUTPUT}" | jq -r '."tool-version"')"
	echo "=== Upload result:	$(echo "${NOTARIZE_OUTPUT}" | jq -r '."success-message"')"
	echo "=== OS version:		$(echo "${NOTARIZE_OUTPUT}" | jq -r '."os-version"')"
	echo "=== Notarization ID:	${NOTARIZE_ID}"
	echo "============================================"	

	if [ "${NOTARIZE_ID}" == "" ] || [ "${NOTARIZE_ID}" == "null" ]; then
	    echo "Error notarization"
	    exit 127
	fi 

	print_message "Get result of notarization"

	NOTARIZATION_DONE="false"
	for count in $(seq 1 300); do
		echo "Wait .. ${count} of 300"
		sleep 10s
		INFO_OUTPUT=$(xcrun altool --output-format json --notarization-info "${NOTARIZE_ID}" --username ${USER_NAME} -p ${PASS})
		NOTARIZE_STATUS_CODE="$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."Status Code"')"
		NOTARIZE_STATUS="$(echo "${INFO_OUTPUT}" | jq -r '."notarization-info"."Status"')"		
		print_notarize_status

		if [ "${NOTARIZE_STATUS_CODE}" == "null" ] || [ "${NOTARIZE_STATUS}" == "in progress" ]; then
		  continue
		fi
		if [ "${NOTARIZE_STATUS_CODE}" != "0" ]; then
		    exit 127
		else
		    print_message "Staple result of the notarization"
		    STAMPLE_OUTPUT=$(xcrun stapler staple -v "${DMG_FILE}" 2>&1 | tr -d "\n")
		    if echo ${STAMPLE_OUTPUT} | grep -q -F 'The staple and validate action worked!'; then
		        echo ""
		        exit 0
		    else
		        echo "${STAMPLE_OUTPUT}"
		        exit 127
		    fi		    
		fi
	done
	print_message "Notarization time out"

}

#***************************************************************************************
notarize_dmg
