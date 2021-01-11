#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"

pushd  ${SCRIPT_FOLDER}/../release/qmake.ios

xcodebuild -project VirgilMessenger.xcodeproj -scheme VirgilMessenger -sdk iphoneos -configuration Release -destination generic/platform=iOS archive -archivePath artifacts/VirgilMessenger.xcarchive

popd