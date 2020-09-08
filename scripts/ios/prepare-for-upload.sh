#!/bin/bash

#original credit for the SwiftSupport and Getting the Developer Directory goes to @bq
#https://github.com/bq/ipa-packager

#To debug or troubleshoot, remove the >/dev/null or 2>/dev/null from the file

IPA="$1"
IPADIR=$(dirname "${IPA}")
TEMPDIR="${IPADIR}/$(cat /dev/urandom | env LC_CTYPE=C tr -cd 'a-z0-9' | head -c 5)"

cd "${IPADIR}"

#check the IPA to make sure it exists
if [ ! -f "${IPA}" ]; then
	echo "Usage: sh auto_package_ipa.sh PATH_TO_SIGNED_IPA"
	exit 1
fi

#get xcode directory
DEVELOPER_DIR=`xcode-select --print-path`
if [ ! -d "${DEVELOPER_DIR}" ]; then
	echo "No developer directory found!"
	exit 1
fi

#unzip the IPA so we can re-build the .app
echo "Unzipping the IPA archive..."
unzip "${IPA}" -d "${TEMPDIR}" >/dev/null

#rename the original IPA
mv "${IPA}" "${IPA/.ipa/-original.ipa}"
echo "Backed up original IPA to ${IPA/.ipa/-original.ipa}"

APPFILE=$(ls "${TEMPDIR}/Payload/")
APP="${TEMPDIR}/Payload/${APPFILE}"
APPFRAMEWORKS="${APP}/Frameworks"

#add the SwiftSupport requirement
if [ -d "${APPFRAMEWORKS}" ]; then

	echo "Copying Swift Support Requirements..."
	SWIFTSUPPORT="${TEMPDIR}/SwiftSupport"

    mkdir -p "${SWIFTSUPPORT}"
    for SWIFT_LIB in $(ls -1 "${APPFRAMEWORKS}"); do
    	#may throw an error, saying it can't find a *.framework, this is normal
        cp "${DEVELOPER_DIR}/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift/iphoneos/${SWIFT_LIB}" "${SWIFTSUPPORT}" >/dev/null 2>/dev/null
    done
fi

#zip the files, save the ipa and cleanup
echo "Zipping updated IPA archive..."
cd "${TEMPDIR}"
zip --symlinks --recurse-paths "${IPA}" . >/dev/null
cd "${IPADIR}"
rm -d -r "${TEMPDIR}"

echo "Completed! Saved as ${IPA}"




# #!/bin/bash

# # Usage:

# # 1. Copy the script into a text editor and save it with no extension
# # 2. Make it executable like so: chmod +x path/to/script
# # 3. Run it from the Terminal in one of two ways:
# #       * path/to/script ipa_path="path/to/ipa" archive_path="path/to/xcarchive"
# #       * path/to/script ipa_path="path/to/ipa" toolchain_path="path/to/toolchain"

# for ARGUMENT in "$@"
# do

#     KEY=$(echo $ARGUMENT | cut -f1 -d=)
#     VALUE=$(echo $ARGUMENT | cut -f2 -d=)

#     case "$KEY" in
#             ipa_path)          ipaPath=${VALUE} ;; # Format: "Path/to/app.ipa"
#             archive_path)      archivePath=${VALUE} ;; # Format: "Path/to/app.xcarchive"
#             toolchain_path)    toolchainPath=${VALUE} ;; # Format: "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift-5.0/iphoneos"
#             *)
#     esac

# done

# # Derived Variables
# ipaDirectory=$(dirname "$ipaPath")
# ipaName=$(basename "$ipaPath")
# zipName=${ipaName/.ipa/.zip}
# appName=${ipaName/.ipa/}
# zipSuffix=-unzipped
# unzippedDirectoryName=${appName}${zipSuffix}
# newIpaSuffix=-with-swift-support
# newIpaName=${appName}${newIpaSuffix}
# swiftSupportPath=SwiftSupport/iphoneos
# ipaSwiftSupportDirectory=${ipaDirectory}/${unzippedDirectoryName}/${swiftSupportPath}

# # Changes the .ipa file extension to .zip and unzips it
# function unzipIPA {
#     mv "${ipaDirectory}/${ipaName}" "${ipaDirectory}/${zipName}"
#     unzip "${ipaDirectory}/${zipName}" -d "${ipaDirectory}/${unzippedDirectoryName}"
# }

# # Copies the SwiftSupport folder from the .xcarchive into the .ipa
# function copySwiftSupportFromArchiveIntoIPA {
#     mkdir -p "$ipaSwiftSupportDirectory"
#     cd "${archivePath}/${swiftSupportPath}"
#     for file in *.dylib; do
#         echo ">>> $file"
#         cp "$file" "$ipaSwiftSupportDirectory"
#     done
# }

# # Creates the SwiftSupport folder from the Xcode toolchain and copies it into the .ipa
# function copySwiftSupportFromToolchainIntoIPA {
#     mkdir -p "$ipaSwiftSupportDirectory"
#     cd "${ipaDirectory}/${unzippedDirectoryName}/Payload/${appName}.app/Frameworks"
#     lib="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift-5.0/iphoneos/libswiftFoundation.dylib"

#     cp "${lib}" "$ipaSwiftSupportDirectory"

#     # for file in *.dylib; do
#     #   if [ -f ${file} ]; then
#     #     cp "${toolchainPath}/${file}" "$ipaSwiftSupportDirectory"
#     #   fi
#     # done
# }

# # Adds the SwiftSupport folder from one of two sources depending on the presence of an .xcarchive
# function addSwiftSupportFolder {
#   if [ -z "$archivePath" ]
#   then
#     copySwiftSupportFromToolchainIntoIPA
#   else
#     copySwiftSupportFromArchiveIntoIPA
#   fi
# }

# # Zips the new folder back up and changes the extension to .ipa
# function createAppStoreIPA {
#     cd "${ipaDirectory}/${unzippedDirectoryName}"
#     zip -r "${ipaDirectory}/${newIpaName}.zip" ./*
#     mv "${ipaDirectory}/${newIpaName}.zip" "${ipaDirectory}/${newIpaName}.ipa"
# }

# # Renames original .ipa and deletes the unzipped folder
# function cleanUp {
#     mv "${ipaDirectory}/${zipName}" "${ipaDirectory}/${ipaName}"
#     rm -r "${ipaDirectory}/${unzippedDirectoryName}"
# }

# # Execute Steps
# unzipIPA
# addSwiftSupportFolder
# createAppStoreIPA
# cleanUp
