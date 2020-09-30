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

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
# ---------------------------------------------------------------------------
# Include Cmake helpers
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(vsplatforms)
include(varpadding)
include(getprebuild)
include(customers)

# Target name and version
set(VS_TARGET_NAME "virgil-messenger")
set(VS_TARGET_VERSION "3.4.18")

get_prebuild()

set(VS_QT_EXTRA "")
# IOS, IOS_SIM
if(VS_PLATFORM STREQUAL "ios" OR VS_PLATFORM STREQUAL "ios-sim")
    set(VS_TARGET_NAME "VirgilMessenger")

# Android
elseif(VS_PLATFORM STREQUAL "android")
    # Get firebase
    get_firebase()
    
    # Set Android version name
    set(ANDROID_VERSION_NAME "${VS_TARGET_VERSION}")
    # Fill Android version code
    string(REPLACE "." ";" VS_VERSION_LIST ${VS_TARGET_VERSION})
    list(GET VS_VERSION_LIST 0 TMP_VERS)
    
    PAD_STRING(ANDROID_VERSION_CODE 3 "0" "${TMP_VERS}")
    list(GET VS_VERSION_LIST 1 TMP_VERS)    
    PAD_STRING(TMP_VERS 3 "0" "${TMP_VERS}")
    string(APPEND ANDROID_VERSION_CODE "${TMP_VERS}")
    
    list(GET VS_VERSION_LIST 2 TMP_VERS)        
    PAD_STRING(TMP_VERS 4 "0" "${TMP_VERS}")    
    string(APPEND ANDROID_VERSION_CODE "${TMP_VERS}")    
    # Android QT Extra
    string(APPEND VS_QT_EXTRA "AndroidExtras")
    # Android gradle path
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platforms/android")    
endif()    

project(${VS_TARGET_NAME} VERSION ${VS_TARGET_VERSION} LANGUAGES C CXX)

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
#   Define prebuilt folders
# ---------------------------------------------------------------------------

# Default build type is Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()
string( TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER )
message("-- Build type: [${CMAKE_BUILD_TYPE}]")

set(PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/prebuilt")
set(PREBUILT_QT_DIR "${PREBUILT_DIR}/qt")

if(VS_PLATFORM STREQUAL "android")
    message(STATUS "ANDROID_VERSION_CODE=${ANDROID_VERSION_CODE}")
    message(STATUS "ANDROID_VERSION_NAME=${ANDROID_VERSION_NAME}")
    set(PREBUILT_BASE_DIR "${PREBUILT_DIR}/${VS_PLATFORM}.${ANDROID_ABI}/${CMAKE_BUILD_TYPE_LOWER}/installed/usr/local")    
else()
    set(PREBUILT_BASE_DIR "${PREBUILT_DIR}/${VS_PLATFORM}/${CMAKE_BUILD_TYPE_LOWER}/installed/usr/local")
endif()

set(PREBUILT_INCLUDE_DIR "${PREBUILT_BASE_DIR}/include")
set(PREBUILT_LIB_DIR "${PREBUILT_BASE_DIR}/lib")
set(QUICKFUTURE_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/quickfuture/src")

message(STATUS "Setting prebuild QT directory:	        [${PREBUILT_QT_DIR}]")
message(STATUS "Setting prebuild directory:		[${PREBUILT_BASE_DIR}]")
message(STATUS "Setting prebuild include directory:	[${PREBUILT_INCLUDE_DIR}]")
message(STATUS "Setting prebuild library directory:	[${PREBUILT_LIB_DIR}]")

# ---------------------------------------------------------------------------
#   Customers selection
# ---------------------------------------------------------------------------

set(VS_CUSTOMER "Virgil" CACHE "STRING" "Customer for build")
set(VS_CUSTOMER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/customers/${VS_CUSTOMER}")
message(STATUS "Current customer : [${VS_CUSTOMER}]")
message(STATUS "Current customer directory: [${VS_CUSTOMER_DIR}]")
message(STATUS "Include customer cmake profile: [customer.cmake]")
include("${VS_CUSTOMER_DIR}/customer.cmake")

vs_customer_prepare()

# ---------------------------------------------------------------------------
#   Build system type
# ---------------------------------------------------------------------------

if(VS_PLATFORM STREQUAL "android")
    message("-- Prepare sources for Android ...")
    add_library(${VS_TARGET_NAME} SHARED)
    
elseif(VS_PLATFORM STREQUAL "macos")
    add_executable(${VS_TARGET_NAME} MACOSX_BUNDLE)
    set_target_properties(${VS_TARGET_NAME}
        PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/platforms/macos/virgil-messenger.plist"
    )
    set(MACOSX_DEPLOYMENT_TARGET 10.14)
#    macdeployqt(${VS_TARGET_NAME} "${CMAKE_CURRENT_LIST_DIR}/src/qml")
else()
    add_executable(${VS_TARGET_NAME})
endif()

# ---------------------------------------------------------------------------
#   Target sources and libraries
# ---------------------------------------------------------------------------
target_sources(${VS_TARGET_NAME}

        PRIVATE
        # -- Includes
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQApplication.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQAttachmentBuilder.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQClipboardProxy.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQCommon.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQCustomer.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQContactManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQCryptoTransferManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDiscoveryManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDownload.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQLastActivityIq.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQLastActivityManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQLogging.h
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
        ${CMAKE_CURRENT_LIST_DIR}/include/macos/VSQMacos.h
        ${CMAKE_CURRENT_LIST_DIR}/include/helpers/VSQSingleton.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ui/VSQUiHelper.h
        # Thirdparty
        ${CMAKE_CURRENT_LIST_DIR}/include/thirdparty/optional/optional.hpp
        # Push notification (Android)
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/VSQPushNotifications.h>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/android/VSQFirebaseListener.h>
        # Customer includes
        ${VS_CUSTOMER_INCLUDES}
        
	# Sources
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQAttachmentBuilder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQClipboardProxy.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQCommon.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQContactManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQCryptoTransferManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDiscoveryManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDownload.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQMessenger.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQLastActivityIq.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQLastActivityManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQLogging.cpp
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
        
        #   QuickFuture
        ${QUICKFUTURE_DIR}/qffuture.cpp
        
        # Push notification
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/VSQPushNotifications.cpp>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/android/VSQFirebaseListener.cpp>

        # Qt Resources
        ${CMAKE_CURRENT_LIST_DIR}/src/resources.qrc
        # Customer sources
        ${VS_CUSTOMER_SOURCES}
        )

target_link_directories(${VS_TARGET_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${PREBUILT_LIB_DIR}>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${PREBUILT_DIR}/firebase_cpp_sdk/libs/android/${ANDROID_ABI}/c++>>        
        )

target_link_libraries(${VS_TARGET_NAME}
        PUBLIC

        # Qt5
        Qt5::Core
        Qt5::Quick
        $<$<STREQUAL:"${VS_PLATFORM}","linux">:Qt5::Widgets>                
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
        $<$<STREQUAL:"${VS_PLATFORM}","android">:ssl>        	               
        $<$<STREQUAL:"${VS_PLATFORM}","android">:crypto>        		
        
        $<$<STREQUAL:"${VS_PLATFORM}","linux">:curl>        
        
        $<$<STREQUAL:"${VS_PLATFORM}","windows">:curl>                
        )

#
#   Common include directories
#
target_include_directories(${VS_TARGET_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_QT_DIR}/config/pc>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}/qxmpp>
        $<BUILD_INTERFACE:${QUICKFUTURE_DIR}>
        $<BUILD_INTERFACE:${VS_CUSTOMER_INCLUDE_DIR}>
	$<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${PREBUILT_DIR}/firebase_cpp_sdk/include>>        
        
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

