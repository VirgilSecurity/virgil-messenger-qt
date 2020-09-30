#!/bin/bash

SCRIPT_FOLDER="$(cd "$(dirname "$0")" && pwd)"

pushd  ${SCRIPT_FOLDER}/../release/qmake.ios

/bin/cat <<EOF > exportOptions.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>app-store</string>
    <key>destination</key>
    <string>export</string>
    <key>method</key>
    <string>app-store</string>
</dict>
</plist>
EOF

xcodebuild -exportArchive -archivePath artifacts/VirgilMessenger.xcarchive -exportOptionsPlist exportOptions.plist -exportPath artifacts -allowProvisioningUpdates

popd