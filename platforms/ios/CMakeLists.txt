#   Copyright (C) 2015-2021 Virgil Security Inc.
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

#
#   Provide default (Qt) implementation for platform dependent operations.
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# ---------------------------------------------------------------------------
#   System introspection.
# ---------------------------------------------------------------------------
if (NOT XCODE_VERSION)
        message(FATAL_ERROR "Xcode only")
endif ()

find_package(Qt5 COMPONENTS QmlImportScanner Gui REQUIRED)

# ---------------------------------------------------------------------------
#   Libraries
# ---------------------------------------------------------------------------
#
#   Library: platform-deps.
#
add_library(platform-deps INTERFACE)
target_link_libraries(platform-deps
        INTERFACE
        Qt5::Gui
        Qt5::QIosOptionalPlugin_NSPhotoLibrary
        )

#
#   Library: platform.
#
add_library(platform STATIC)

target_sources(platform
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformIos.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformIos.mm"

        # TODO: Move to the include/src.
        # TODO: Hide implementation under platform API.
        "${CMAKE_CURRENT_LIST_DIR}/AppDelegate.mm"
        "${CMAKE_CURRENT_LIST_DIR}/IosDocumentInteractionController.h"
        "${CMAKE_CURRENT_LIST_DIR}/IosDocumentInteractionController.mm"
        "${CMAKE_CURRENT_LIST_DIR}/IosViewController.h"
        "${CMAKE_CURRENT_LIST_DIR}/IosViewController.mm"
        )

target_include_directories(platform
        PUBLIC
        # TODO: When implementation will be hidden under platform API, then remove it.
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../include/controllers>"

        PUBLIC
        # TODO: When implementation will be hidden under platform API, make it PRIVATE.
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        )

target_link_libraries(platform
        PUBLIC
        platform-deps
        platform-base
        customer-config
        )

#
#   Library: platform-fs.
#
add_library(platform-fs STATIC)

target_sources(platform-fs
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/PlatformFsIos.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/PlatformFsIos.cpp"
        )

target_include_directories(platform-fs
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
        )

target_link_libraries(platform-fs
        PUBLIC
        platform-fs-base
        )

# ---------------------------------------------------------------------------
#   Main target
# ---------------------------------------------------------------------------
if (VS_IOS_SIMULATOR)
set(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphonesimulator")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator")
set(CMAKE_OSX_ARCHITECTURES "x86_64")
else ()
set(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")

set(CMAKE_OSX_ARCHITECTURES "arm64")
endif ()

set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")

set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)

add_executable(${VS_TARGET_NAME} MACOSX_BUNDLE)

# Settings for signing
if (VS_CODE_SIGN_IDENTITY)
set_xcode_property(${VS_TARGET_NAME} CODE_SIGN_IDENTITY "${VS_CODE_SIGN_IDENTITY}")
endif ()
if (VS_DEVELOPMENT_TEAM)
set_xcode_property(${VS_TARGET_NAME} DEVELOPMENT_TEAM "${VS_DEVELOPMENT_TEAM}")
endif ()
if (VS_PROVISIONING_PROFILE_SPECIFIER)
set_xcode_property(${VS_TARGET_NAME} PROVISIONING_PROFILE_SPECIFIER "${VS_PROVISIONING_PROFILE_SPECIFIER}")
endif ()
set_xcode_property(${VS_TARGET_NAME} IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")
set_xcode_property(${VS_TARGET_NAME} CODE_SIGN_ENTITLEMENTS "${VS_CUSTOMER_DIR}/platforms/ios/Entitlements/VirgilMessenger.entitlements")
set_xcode_property(${VS_TARGET_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")

set_target_properties(${VS_TARGET_NAME}
    PROPERTIES
    XCODE_ATTRIBUTE_INSTALL_PATH "/Applications"
    XCODE_ATTRIBUTE_SKIP_INSTALL "No"
    )
# Storyboard default
configure_file("${CMAKE_CURRENT_LIST_DIR}/LaunchScreen.storyboard.in"
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/LaunchScreen.storyboard" @ONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/Default-568h@2x.png"
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/Default-568h@2x.png" COPYONLY)

set(STORYBOARD_DEFAULT
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/LaunchScreen.storyboard
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/Default-568h@2x.png
    )

set(VS_ASSET_DIR "${VS_CUSTOMER_DIR}/platforms/ios/Assets.xcassets")
target_sources(${VS_TARGET_NAME} PRIVATE ${VS_ASSET_DIR} ${STORYBOARD_DEFAULT})
set_source_files_properties("${VS_ASSET_DIR}"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
    )

set_target_properties(${VS_TARGET_NAME}
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/Info.plist.in"
    LINK_FLAGS "-framework Foundation -framework UserNotifications -Wl,-e,_qt_main_wrapper"
    RESOURCE "${STORYBOARD_DEFAULT}"
    )

qt5_import_qml_plugins(${VS_TARGET_NAME})

#
#  IOS Push Notification extension.
#
set(VS_TARGET_NOTIFICATION_EXTENSION_NAME ${VS_TARGET_NAME}_NotificationExtension)
add_executable(${VS_TARGET_NOTIFICATION_EXTENSION_NAME} MACOSX_BUNDLE)

set_target_properties(${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
    PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/NotificationExtensionInfo.plist.in"
    )

if (VS_CODE_SIGN_IDENTITY)
set_xcode_property(${VS_TARGET_NOTIFICATION_EXTENSION_NAME} CODE_SIGN_IDENTITY "${VS_CODE_SIGN_IDENTITY}")
endif ()

if (VS_DEVELOPMENT_TEAM)
set_xcode_property(${VS_TARGET_NOTIFICATION_EXTENSION_NAME} DEVELOPMENT_TEAM "${VS_DEVELOPMENT_TEAM}")
endif ()

if (VS_PROVISIONING_PROFILE_SPECIFIER)
set_xcode_property(${VS_TARGET_NOTIFICATION_EXTENSION_NAME} PROVISIONING_PROFILE_SPECIFIER "${VS_PROVISIONING_PROFILE_SPECIFIER}")
endif ()

set_xcode_property(${VS_TARGET_NOTIFICATION_EXTENSION_NAME} IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")
set_xcode_property(${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
    CODE_SIGN_ENTITLEMENTS
    "${VS_CUSTOMER_DIR}/platforms/ios/Entitlements/VirgilMessenger_NotificationExtension.entitlements"
    )

target_sources(${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/NotificationService.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/NotificationService.h"
    )

set_target_properties(${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
    PROPERTIES
    BUNDLE YES
    BUNDLE_EXTENSION appex
    XCODE_ATTRIBUTE_WRAPPER_EXTENSION appex
    XCODE_ATTRIBUTE_APPLICATION_EXTENSION_API_ONLY YES
    XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES
    XCODE_PRODUCT_TYPE "com.apple.product-type.app-extension"
    XCODE_EXPLICIT_FILE_TYPE "wrapper.app-extension"
    )

target_include_directories(${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platforms/ios>
    )

find_library(USER_NOTIFICATIONS_LIB UserNotifications)
find_library(USER_NOTIFICATIONS_UI_LIB UserNotificationsUI)
target_link_libraries(${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
        PRIVATE
        ${USER_NOTIFICATIONS_LIB}
        ${USER_NOTIFICATIONS_UI_LIB}
        core-messenger
        )

# target_link_options(${VS_TARGET_NOTIFICATION_EXTENSION_NAME} PRIVATE "-Wl,-U,_main")

add_custom_command(TARGET ${VS_TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
    ARGS -E copy_directory
        "$<TARGET_BUNDLE_DIR:${VS_TARGET_NOTIFICATION_EXTENSION_NAME}>"
        "$<TARGET_FILE_DIR:${VS_TARGET_NAME}>/PlugIns/${VS_TARGET_NOTIFICATION_EXTENSION_NAME}.appex"
    MAIN_DEPENDENCY ${VS_TARGET_NOTIFICATION_EXTENSION_NAME}
    )

# ---------------------------------------------------------------------------
#   Deploy
# ---------------------------------------------------------------------------
add_custom_target(xcarchive
        COMMENT "Building xcarchive..."

        COMMAND xcodebuild
            -project "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.xcodeproj"
            -scheme ${PROJECT_NAME}
            -sdk iphoneos
            -destination generic/platform=iOS
            -configuration Release
            archive
            -archivePath "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.xcarchive"

        COMMAND "${CMAKE_COMMAND}" -E
            tar czf "${PROJECT_NAME}-${PROJECT_VERSION}.tgz" -- "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.xcarchive"
        )

add_custom_target(upload
        COMMENT "Upload xcarchive..."
        COMMAND xcodebuild
            -exportArchive
            -archivePath "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.xcarchive"
            -exportOptionsPlist "${CMAKE_CURRENT_LIST_DIR}/exportOptions.plist"
            -allowProvisioningUpdates
        )

add_custom_target(ipa
        COMMENT "Build IPA..."
        COMMAND xcodebuild
        -exportArchive
        -archivePath "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.xcarchive"
        -exportOptionsPlist "${CMAKE_CURRENT_LIST_DIR}/exportOptions-ipa.plist"
        -allowProvisioningUpdates
        -exportPath "${CMAKE_BINARY_DIR}"
        )
