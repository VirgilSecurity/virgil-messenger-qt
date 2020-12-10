#   Copyright (C) 2015-2020 Virgil Security Inc.
#
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are
#   met:
#
#       (1) Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#       (2) Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#       (3) Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived from
#       this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
#
#   Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

if(VS_PLATFORM STREQUAL "android")

  # Create "apk" and "aab" targets

  find_program(ANDROID_DEPLOY_QT androiddeployqt)
  
  if(DEFINED ENV{JAVA_HOME})
    set(JAVA_HOME $ENV{JAVA_HOME} CACHE INTERNAL "Saved JAVA_HOME variable")
  elseif(EXISTS "/etc/alternatives/java_sdk")
    set(JAVA_HOME "/etc/alternatives/java_sdk" CACHE INTERNAL "Saved JAVA_HOME variable")
  endif()
  
  if(JAVA_HOME)
    set(android_deploy_qt_jdk "--jdk ${JAVA_HOME}")
    message(STATUS "Java home: [${android_deploy_qt_jdk}]")
  endif()

  if (ANDROID_SDK_PLATFORM)
    set(android_deploy_qt_platform "--android-platform ${ANDROID_SDK_PLATFORM}")
    message(STATUS "Android deploy QT platform: [${ANDROID_SDK_PLATFORM}]")
  endif()
    
  add_custom_target(apk_release
    COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${ANDROID_DEPLOY_QT}
       --input "${CMAKE_BINARY_DIR}/android_deployment_settings.json"
       --output "${CMAKE_BINARY_DIR}/android-build"
       --apk "${CMAKE_BINARY_DIR}/android-build/${PROJECT_NAME}.apk"
       ${android_deploy_qt_platform}
       ${android_deploy_qt_jdk}
       --gradle
       --sign "${VS_KEYCHAIN}" "${VS_KEY_ALIAS}"
       --storepass "${VS_KEYCHAIN_PASSWORD}"
       --keypass "${VS_KEY_PASSWORD}"
       --no-gdbserver      
    VERBATIM)

  add_custom_target(aab_release
    COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${ANDROID_DEPLOY_QT}
      --input "${CMAKE_BINARY_DIR}/android_deployment_settings.json"
      --output "${CMAKE_BINARY_DIR}/android-build"
      --aab
      ${android_deploy_qt_platform}
      ${android_deploy_qt_jdk}
      --gradle
      --sign "${VS_KEYCHAIN}" "${VS_KEY_ALIAS}"
      --storepass "${VS_KEYCHAIN_PASSWORD}"
      --keypass "${VS_KEY_PASSWORD}"      
      --no-gdbserver      
   VERBATIM)

elseif(VS_PLATFORM STREQUAL "linux")

  find_program(LINUX_DEPLOY_QT cqtdeployer)

  add_custom_target(deploy
    COMMAND ${LINUX_DEPLOY_QT}
       -bin ${PROJECT_NAME} 
       -qmlDir ${PROJECT_SOURCE_DIR}/src/qml 
       -targetDir ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dist
       -qmake ${QT_QMAKE_EXECUTABLE} clear
    VERBATIM)

elseif(VS_PLATFORM STREQUAL "macos")

  find_program(MAC_DEPLOY_QT macdeployqt)
  find_program(MAC_CODESIGN codesign)  
  find_program(MAC_APPDMG appdmg)    
  
  add_custom_target(dmg_release
    COMMAND echo "Deploy MacOS bundle data..."
    COMMAND ${MAC_DEPLOY_QT}
       ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app
       -qmldir=${PROJECT_SOURCE_DIR}/src/qml        
       -verbose=1  
    COMMAND echo "Signing sparkle [Autoupdate]"              
    COMMAND ${MAC_CODESIGN} --display --verbose=4 --force --deep --timestamp --options runtime -s "${VS_MACOS_IDENT}" ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/Autoupdate
    COMMAND echo "Signing sparkle [fileop]"                  
    COMMAND ${MAC_CODESIGN} --display --verbose=4 --force --deep --timestamp --options runtime -s "${VS_MACOS_IDENT}" ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app/Contents/MacOS/fileop
    COMMAND echo "Signing bundle..."       
    COMMAND ${MAC_CODESIGN}
	--display
	--verbose=4
	--force
	--deep
	--timestamp
	--options runtime
	-s "${VS_MACOS_IDENT}"
	"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app"
    COMMAND echo "Create DMG..."       	
    COMMAND ${MAC_APPDMG} 
	"${CMAKE_BINARY_DIR}/dmg.json"
	"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
	-v
    COMMAND echo "Set DMG icon..."	
    COMMAND
	${PROJECT_SOURCE_DIR}/platforms/macos/tools/seticon
	${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}
	"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"
    VERBATIM)

  add_custom_target(dmg_notarization
    COMMAND ${PROJECT_SOURCE_DIR}/platforms/macos/tools/dmg-notarization.sh -f "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg" -u ${NOTARIZATION_LOGIN} -p ${NOTARIZATION_PASSWORD} -i "${MACOSX_BUNDLE_GUI_IDENTIFIER}"
    VERBATIM)  
    
  add_custom_target(dmg_debug
    COMMAND echo "Deploy MacOS bundle data (without signing)..."
    COMMAND ${MAC_DEPLOY_QT}
       ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app
       -qmldir=${PROJECT_SOURCE_DIR}/src/qml        
       -verbose=1  
       -dmg
    COMMAND
	${PROJECT_SOURCE_DIR}/platforms/macos/tools/seticon
	${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}
	"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg"       
    VERBATIM)
    
  add_custom_target(dmg_update
    COMMAND echo "Creating sparkle update files..."
    COMMAND cp -f "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dmg" "${CMAKE_BINARY_DIR}/update/${PROJECT_NAME}-${PROJECT_VERSION}.dmg"
    COMMAND ${PROJECT_SOURCE_DIR}/ext/prebuilt/macos/sparkle/bin/generate_appcast "${CMAKE_BINARY_DIR}/update"
    VERBATIM)
    
elseif(VS_PLATFORM STREQUAL "ios")    

  add_custom_target(xcarchive
    COMMAND echo "Building xcarchive..."
    COMMAND xcodebuild -project ${PROJECT_NAME}.xcodeproj -scheme ${PROJECT_NAME} -sdk iphoneos -configuration Release archive -archivePath ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcarchive
    VERBATIM)

  add_custom_target(upload_testflight
    COMMAND echo "Upload xcarchive..."
    COMMAND xcodebuild -exportArchive -archivePath ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcarchive -exportOptionsPlist ${PROJECT_SOURCE_DIR}/platforms/ios/exportOptions.plist -allowProvisioningUpdates
    VERBATIM)
        
endif()