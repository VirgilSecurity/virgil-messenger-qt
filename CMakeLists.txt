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

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
# ---------------------------------------------------------------------------
# Include Cmake helpers
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(vsplatforms)
include(clangformat)
include(varpadding)
include(getprebuild)
include(customers)

#
# Code formatting
#
option(ENABLE_CLANGFORMAT "On/Off formatting source code with clangformat." ON)


# ---------------------------------------------------------------------------
# Project version
# ---------------------------------------------------------------------------
set(VS_TARGET_VERSION "3.4.18")
set(VS_CORE_VERSION "0.1.3.134")

# ---------------------------------------------------------------------------
#   Customers selection
# ---------------------------------------------------------------------------
set(VS_CUSTOMER "Virgil" CACHE "STRING" "Customer for build")
set(VS_CUSTOMER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/customers/${VS_CUSTOMER}")
set(VS_CUSTOMER_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/customers/Common")
message(STATUS "Current customer : [${VS_CUSTOMER}]")
message(STATUS "Current customer directory: [${VS_CUSTOMER_DIR}]")
message(STATUS "Include customer cmake target name: [customer-target.cmake]")
# Include customer file for project target name
include("${VS_CUSTOMER_DIR}/customer.cmake")
include("${VS_CUSTOMER_COMMON_DIR}/customer_common.cmake")

# ---------------------------------------------------------------------------
# Define project
# ---------------------------------------------------------------------------
project(${VS_TARGET_NAME} VERSION ${VS_TARGET_VERSION} LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Detecting platform if VS_PLATFORM not specified
# ---------------------------------------------------------------------------
if(NOT VS_PLATFORM)
    if(ANDROID)
        message(STATUS "Detected target platform: [Android]")
        set(VS_PLATFORM "android")
    elseif(IOS)
        message(STATUS "Detected target platform: [IOS]")
        set(VS_PLATFORM "ios")        
    elseif(APPLE)
        message(STATUS "Detected target platform: [MacOS]")
        set(VS_PLATFORM "macos")        
    elseif(UNIX)
        message(STATUS "Detected target platform: [Unix]")
        set(VS_PLATFORM "linux")
    elseif(MINGW32)
        message(STATUS "Detected target platform: [Windows]")
        set(VS_PLATFORM "windows")
    endif()
endif()

# ---------------------------------------------------------------------------
# Get prebuild core libraries
# ---------------------------------------------------------------------------
get_prebuild()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()
string( TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER )
message("-- Build type: [${CMAKE_BUILD_TYPE}]")

if(CMAKE_BUILD_TYPE_LOWER STREQUAL "minsizerel" OR CMAKE_BUILD_TYPE_LOWER STREQUAL "relwithdebinfo")
    set(CMAKE_BUILD_TYPE_LOWER "release")
endif()

set(PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/prebuilt")
set(PREBUILT_QT_DIR "${PREBUILT_DIR}/qt")

if(VS_IOS_SIMULATOR AND VS_PLATFORM STREQUAL "ios")
    set(VS_IOS_SIM_PREFIX "-sim")
endif()

if(VS_PLATFORM STREQUAL "android")
    set(PREBUILT_BASE_DIR "${PREBUILT_DIR}/${VS_PLATFORM}.${ANDROID_ABI}/${CMAKE_BUILD_TYPE_LOWER}/installed/usr/local")    
else()
    set(PREBUILT_BASE_DIR "${PREBUILT_DIR}/${VS_PLATFORM}${VS_IOS_SIM_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/installed/usr/local")
endif()

set(PREBUILT_INCLUDE_DIR "${PREBUILT_BASE_DIR}/include")
set(PREBUILT_LIB_DIR "${PREBUILT_BASE_DIR}/lib")
set(QUICKFUTURE_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/quickfuture/src")

message(STATUS "Setting prebuild QT directory:	        [${PREBUILT_QT_DIR}]")
message(STATUS "Setting prebuild directory:		[${PREBUILT_BASE_DIR}]")
message(STATUS "Setting prebuild include directory:	[${PREBUILT_INCLUDE_DIR}]")
message(STATUS "Setting prebuild library directory:	[${PREBUILT_LIB_DIR}]")

# ---------------------------------------------------------------------------
# Include customer file and prepare data
# ---------------------------------------------------------------------------
include("${VS_CUSTOMER_DIR}/customer.cmake")
include("${VS_CUSTOMER_COMMON_DIR}/customer_common.cmake")
vs_customer_prepare()

# ---------------------------------------------------------------------------
# Prepare platform depends
# ---------------------------------------------------------------------------
set(VS_QT_EXTRA "")

# IOS, IOS_SIM
if(VS_PLATFORM STREQUAL "ios")
    if(NOT XCODE_VERSION)
      message(FATAL_ERROR "Xcode only")
    endif()

    # QT Extra packages
    string(APPEND VS_QT_EXTRA "QmlImportScanner")

# Android
elseif(VS_PLATFORM STREQUAL "android")
    # Get firebase
    get_firebase()

    # Set Android version name
    set(ANDROID_VERSION_NAME "${VS_TARGET_VERSION}")
    # Fill Android version code
    string(REPLACE "." ";" VS_VERSION_LIST ${VS_TARGET_VERSION})

    list(GET VS_VERSION_LIST 0 TMP_VERS)
    PAD_STRING(ANDROID_VERSION_CODE 3 "0" "1${TMP_VERS}")
    
    list(GET VS_VERSION_LIST 1 TMP_VERS)    
    PAD_STRING(TMP_VERS 2 "0" "${TMP_VERS}")
    string(APPEND ANDROID_VERSION_CODE "${TMP_VERS}")
    
    list(GET VS_VERSION_LIST 2 TMP_VERS)        
    PAD_STRING(TMP_VERS 2 "0" "${TMP_VERS}")    
    string(APPEND ANDROID_VERSION_CODE "${TMP_VERS}")    
    
    # Get build number ( or zero)
    if (DEFINED ENV{BUILD_NUMBER})
        set(VS_BUILD_NUMBER "$ENV{BUILD_NUMBER}")
    else()
        set(VS_BUILD_NUMBER "0")    
    endif()
    PAD_STRING(TMP_VERS 3 "0" "${VS_BUILD_NUMBER}")
    string(APPEND ANDROID_VERSION_CODE "${TMP_VERS}")
    message(STATUS "Android version name: [${ANDROID_VERSION_NAME}]")
    message(STATUS "Android version code: [${ANDROID_VERSION_CODE}]")    

    # Android QT Extra
    string(APPEND VS_QT_EXTRA "AndroidExtras")
    # Android gradle path
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platforms/android")    
endif()    

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
include(FetchContent)

find_package(Qt5 COMPONENTS Core Widgets Quick Sql Network Concurrent Qml Xml ${VS_QT_EXTRA} REQUIRED)

# ---------------------------------------------------------------------------
#   Prepare for build target
# ---------------------------------------------------------------------------
# Add deployers targets
include(deployers)    

# Android
if(VS_PLATFORM STREQUAL "android")
    message("-- Prepare sources for Android ...")
    add_library(${VS_TARGET_NAME} SHARED)
    include(${ANDROID_SDK}/android_openssl/CMakeLists.txt)

    if (ANDROID_ABI STREQUAL "armeabi-v7a")
        set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/arm/")
    elseif (ANDROID_ABI STREQUAL "arm64-v8a")
	set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/arm64/")
    elseif (ANDROID_ABI STREQUAL "x86")
	set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/x86/")
    elseif (ANDROID_ABI STREQUAL "x86_64")
	set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/x86_64/")
    endif()
    
    message(STATUS "Android OpenSSL root path:    [${SSL_ROOT_PATH}]")    
    message(STATUS "Android OpenSSL library path: [${VS_ANDROID_SSL_DIR}]")

# Linux
elseif(VS_PLATFORM STREQUAL "linux")
    add_executable(${VS_TARGET_NAME})

# MacOS    
elseif(VS_PLATFORM STREQUAL "macos")
    set(App_ICON "${VS_CUSTOMER_DIR}/platforms/macos/pkg_resources/${MACOSX_BUNDLE_ICON_FILE}")
    set_source_files_properties(${App_ICON} 
	PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Sparkle	
    message(STATUS "Copy Sparkle framework")
    execute_process(
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/${VS_TARGET_NAME}.app/Contents/Frameworks 
        COMMAND cp -Rf ${PREBUILT_DIR}/${VS_PLATFORM}/sparkle/Sparkle.framework ${CMAKE_BINARY_DIR}/${VS_TARGET_NAME}.app/Contents/Frameworks 
    )

    add_executable(${VS_TARGET_NAME} MACOSX_BUNDLE ${App_ICON})
    
    set_target_properties(${VS_TARGET_NAME} 
	PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/platforms/macos/Info.plist.in" 
    )

    add_custom_command(TARGET ${VS_TARGET_NAME}
       POST_BUILD COMMAND 
       ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/../Frameworks/"
       $<TARGET_FILE:${VS_TARGET_NAME}>)
    
    set_target_properties(${VS_TARGET_NAME} 
        PROPERTIES LINK_FLAGS "-F${PREBUILT_DIR}/${VS_PLATFORM}/sparkle -framework Sparkle -framework CoreFoundation -framework Foundation"
    )
# IOS    
elseif(VS_PLATFORM STREQUAL "ios")    

    if(VS_IOS_SIMULATOR)
	set(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphonesimulator")
	set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator")    
	set(CMAKE_OSX_ARCHITECTURES "x86_64")	
    else()
	set(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos")
	set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")    
	set(CMAKE_OSX_ARCHITECTURES "arm64")	
    endif()

    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,-e,_qt_main_wrapper")
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
    set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)    
    
    add_executable(${VS_TARGET_NAME} MACOSX_BUNDLE)
    
    # Settings for signing
    if(VS_CODE_SIGN_IDENTITY)
        SET_XCODE_PROPERTY(${VS_TARGET_NAME} CODE_SIGN_IDENTITY "${VS_CODE_SIGN_IDENTITY}")
    endif()
    if(VS_DEVELOPMENT_TEAM)
	SET_XCODE_PROPERTY(${VS_TARGET_NAME} DEVELOPMENT_TEAM "${VS_DEVELOPMENT_TEAM}")
    endif()
    if(VS_PROVISIONING_PROFILE_SPECIFIER)
	SET_XCODE_PROPERTY(${VS_TARGET_NAME} PROVISIONING_PROFILE_SPECIFIER "${VS_PROVISIONING_PROFILE_SPECIFIER}")
    endif()	
    SET_XCODE_PROPERTY(${VS_TARGET_NAME} IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")
    SET_XCODE_PROPERTY(${VS_TARGET_NAME} CODE_SIGN_ENTITLEMENTS "${VS_CUSTOMER_DIR}/platforms/ios/Entitlements/VirgilMessenger.entitlements")    
    SET_XCODE_PROPERTY(${VS_TARGET_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")    

    set_target_properties(${VS_TARGET_NAME}
        PROPERTIES
            XCODE_ATTRIBUTE_INSTALL_PATH "/Applications"
            XCODE_ATTRIBUTE_SKIP_INSTALL "No"
    )

    set(VS_ASSET_DIR "${VS_CUSTOMER_DIR}/platforms/ios/Assets.xcassets")
    target_sources(${VS_TARGET_NAME} PRIVATE ${VS_ASSET_DIR})
    set_source_files_properties("${VS_ASSET_DIR}"
	PROPERTIES 
	    MACOSX_PACKAGE_LOCATION "Resources"
    )

    set_target_properties(${VS_TARGET_NAME}
        PROPERTIES 
    	    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/platforms/ios/Info.plist.in"
	    LINK_FLAGS "-framework Foundation -framework UserNotifications"        
    )

    qt5_import_qml_plugins(${VS_TARGET_NAME})
else()
    message(FATAL_ERROR "Unknown target platform")
endif()

# ---------------------------------------------------------------------------
#   Target sources and libraries
# ---------------------------------------------------------------------------
target_sources(${VS_TARGET_NAME}

        PRIVATE
	# -- Headers
	${CMAKE_CURRENT_LIST_DIR}/include/AccountSelectionModel.h
	${CMAKE_CURRENT_LIST_DIR}/include/KeyboardEventFilter.h
	${CMAKE_CURRENT_LIST_DIR}/include/Validator.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQApplication.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQAttachmentBuilder.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQClipboardProxy.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQCommon.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQContactManager.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQCrashReporter.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQCryptoTransferManager.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQDiscoveryManager.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQDownload.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQLastActivityIq.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQLastActivityManager.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQMessenger.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQSettings.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQSqlChatModel.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQSqlConversationModel.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQNetworkAnalyzer.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQTransfer.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQTransferManager.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQUpload.h
	${CMAKE_CURRENT_LIST_DIR}/include/VSQUtils.h
	${CMAKE_CURRENT_LIST_DIR}/include/android/VSQAndroid.h
	$<$<STREQUAL:"${VS_PLATFORM}","macos">:${CMAKE_CURRENT_LIST_DIR}/include/macos/VSQMacos.h>
	${CMAKE_CURRENT_LIST_DIR}/include/helpers/VSQSingleton.h
	${CMAKE_CURRENT_LIST_DIR}/include/helpers/FutureWorker.h
	${CMAKE_CURRENT_LIST_DIR}/include/ui/VSQUiHelper.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/AccountSelectionState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/AccountSettingsState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/ApplicationStateManager.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/AttachmentPreviewState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/BackupKeyState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/ChatListState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/ChatState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/DownloadKeyState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/NewChatState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/OperationState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/SignInAsState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/SignInState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/SignInUsernameState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/SignUpState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/SplashScreenState.h
	${CMAKE_CURRENT_LIST_DIR}/include/states/StartState.h
                         
	# Logging
	${CMAKE_CURRENT_LIST_DIR}/include/logging/VSQLogging.h
	${CMAKE_CURRENT_LIST_DIR}/include/logging/VSQLogWorker.h
	${CMAKE_CURRENT_LIST_DIR}/include/logging/VSQMessageLogContext.h
	# Generated
	${CMAKE_CURRENT_LIST_DIR}/include/VSQCustomer.h
	# Thirdparty
	${CMAKE_CURRENT_LIST_DIR}/include/thirdparty/optional/optional.hpp

	# IOS
	$<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/PushNotifications.h>
	$<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/XmppPushNotifications.h>

	# Android
	$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/PushNotifications.h>
	$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/XmppPushNotifications.h>
	$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/android/FirebaseListener.h>

	# -- Sources
	${CMAKE_CURRENT_LIST_DIR}/src/AccountSelectionModel.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/KeyboardEventFilter.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/Validator.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQAttachmentBuilder.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQClipboardProxy.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQCommon.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQContactManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQCrashReporter.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQCryptoTransferManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQDiscoveryManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQDownload.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQMessenger.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQLastActivityIq.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQLastActivityManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQSettings.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQSqlChatModel.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQSqlConversationModel.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQNetworkAnalyzer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQTransfer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQTransferManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQUpload.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQUtils.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/android/VSQAndroid.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/VSQApplication.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/ui/VSQUiHelper.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/hal.cpp
	# State
	${CMAKE_CURRENT_LIST_DIR}/src/states/AccountSelectionState.cpp  
	${CMAKE_CURRENT_LIST_DIR}/src/states/AccountSettingsState.cpp   
	${CMAKE_CURRENT_LIST_DIR}/src/states/ApplicationStateManager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/states/AttachmentPreviewState.cpp 
	${CMAKE_CURRENT_LIST_DIR}/src/states/BackupKeyState.cpp         
	${CMAKE_CURRENT_LIST_DIR}/src/states/ChatListState.cpp          
	${CMAKE_CURRENT_LIST_DIR}/src/states/ChatState.cpp              
	${CMAKE_CURRENT_LIST_DIR}/src/states/DownloadKeyState.cpp       
	${CMAKE_CURRENT_LIST_DIR}/src/states/NewChatState.cpp           
	${CMAKE_CURRENT_LIST_DIR}/src/states/SignInAsState.cpp          
	${CMAKE_CURRENT_LIST_DIR}/src/states/SignInState.cpp            
	${CMAKE_CURRENT_LIST_DIR}/src/states/SignInUsernameState.cpp    
	${CMAKE_CURRENT_LIST_DIR}/src/states/SignUpState.cpp            
	${CMAKE_CURRENT_LIST_DIR}/src/states/SplashScreenState.cpp      
	# Logging
	${CMAKE_CURRENT_LIST_DIR}/src/logging/VSQLogging.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/logging/VSQLogWorker.cpp
	# QT QRC
	${CMAKE_CURRENT_LIST_DIR}/src/resources.qrc
	${VS_CUSTOMER_DIR}/src/customer.qrc

	# MACOS
	$<$<STREQUAL:"${VS_PLATFORM}","macos">:${CMAKE_CURRENT_LIST_DIR}/src/macos/VSQMacos.mm>

	# IOS
	$<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/platforms/ios/AppDelegate.mm>
	$<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/PushNotifications.cpp>
	$<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/XmppPushNotifications.cpp>

	# Android
	$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/PushNotifications.cpp>
	$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/XmppPushNotifications.cpp>
	$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/android/FirebaseListener.cpp>
        
        #   QuickFuture
        ${QUICKFUTURE_DIR}/qffuture.cpp
        
        # Customer sources
        ${VS_CUSTOMER_SOURCES}
        )

target_link_directories(${VS_TARGET_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${PREBUILT_LIB_DIR}>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${PREBUILT_DIR}/firebase_cpp_sdk/libs/android/${ANDROID_ABI}/c++>>        
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${VS_ANDROID_SSL_DIR}>>
        )

target_link_libraries(${VS_TARGET_NAME}
        PUBLIC

        # Qt5
        Qt5::Core
        Qt5::Quick
        $<$<STREQUAL:"${VS_PLATFORM}","linux">:Qt5::Widgets>                
        $<$<STREQUAL:"${VS_PLATFORM}","macos">:Qt5::Widgets>                        
        $<$<STREQUAL:"${VS_PLATFORM}","android">:Qt5::Widgets>                        
        $<$<STREQUAL:"${VS_PLATFORM}","android">:Qt5::AndroidExtras>        
        $<$<STREQUAL:"${VS_PLATFORM}","android">:log>        
        $<$<STREQUAL:"${VS_PLATFORM}","windows">:Qt5::Widgets>                        
        Qt5::Sql
        Qt5::Network
        Qt5::Concurrent
        Qt5::Qml
        Qt5::Xml
    
        # Other libs
        qxmpp
#	vs-module-messenger
	
	vs-module-logger
	vs-messenger-internal
	vsc_keyknox_sdk
	vsc_pythia_sdk
	vsc_core_sdk
	vsc_foundation
	vsc_foundation_pb
	vsc_pythia
	vsc_common
	protobuf-nanopb
	mbedcrypto
	ed25519
	relic_s
	json-c
	
        $<$<STREQUAL:"${VS_PLATFORM}","android">:firebase_messaging>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:firebase_app>        
        $<$<STREQUAL:"${VS_PLATFORM}","android">:firebase_auth> 
        $<$<STREQUAL:"${VS_PLATFORM}","android">:curl>                
        $<$<STREQUAL:"${VS_PLATFORM}","android">:ssl_1_1>        	               
        $<$<STREQUAL:"${VS_PLATFORM}","android">:crypto_1_1>        		

        $<$<STREQUAL:"${VS_PLATFORM}","linux">:curl>        
        
        $<$<STREQUAL:"${VS_PLATFORM}","windows">:curl>                
        )

#
#   Common include directories
#
target_include_directories(${VS_TARGET_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/notifications>        
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/config/pc>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}/qxmpp>
        $<BUILD_INTERFACE:${QUICKFUTURE_DIR}>
        $<BUILD_INTERFACE:${VS_CUSTOMER_INCLUDE_DIR}>
	$<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications>>    
	$<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/android>>
	$<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${PREBUILT_DIR}/firebase_cpp_sdk/include>>    	
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","macos">:${PREBUILT_DIR}/${VS_PLATFORM}/sparkle/Sparkle.framework/Headers>>        
        )

target_compile_definitions(${VS_TARGET_NAME}
    PRIVATE
    $<BUILD_INTERFACE:INFO_CLIENT=1>
    $<BUILD_INTERFACE:CFG_CLIENT=1>
    $<BUILD_INTERFACE:VS_CUSTOMER=${VS_CUSTOMER}>

    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:VS_ANDROID=1>>
    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:VS_MOBILE=1>>
    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:VS_PUSHNOTIFICATIONS=1>>

    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","ios">:VS_IOS=1>>
    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","ios">:VS_MOBILE=1>>    
    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","ios">:VS_PUSHNOTIFICATIONS=1>>    
    
    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","macos">:MACOS=1>>
    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","macos">:VS_DESKTOP=1>>    

    $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","linux">:VS_DESKTOP=1>>    
    )

target_compile_options(${VS_TARGET_NAME}
    INTERFACE
        -Wno-multichar
    )

# ---------------------------------------------------------------------------
#   Additional debug parameters
# ---------------------------------------------------------------------------

target_compile_definitions(${VS_TARGET_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
  )

#if(COMMAND add_clangformat AND ENABLE_CLANGFORMAT)
#    add_clangformat(${VS_TARGET_NAME})
#endif()
