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
set(VS_TARGET_VERSION "3.5.7")
set(VS_CORE_VERSION "0.2.1.45")
set(VERSION_DATABASE_SCHEME "0")

# ---------------------------------------------------------------------------
#   Customers selection
# ---------------------------------------------------------------------------
set(VS_CUSTOMER "Virgil" CACHE STRING "Customer for build")
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

message(STATUS "CMAKE_PREFIX_PATH: [${CMAKE_PREFIX_PATH}]")
message(STATUS "CMAKE_FIND_ROOT_PATH: [${CMAKE_FIND_ROOT_PATH}]")

# ---------------------------------------------------------------------------
# Detecting platform if VS_PLATFORM not specified
# ---------------------------------------------------------------------------
if (NOT VS_PLATFORM)
    if (ANDROID)
        message(STATUS "Detected target platform: [Android]")
        set(VS_PLATFORM "android")
    elseif (IOS)
        message(STATUS "Detected target platform: [IOS]")
        set(VS_PLATFORM "ios")
    elseif (APPLE)
        message(STATUS "Detected target platform: [MacOS]")
        set(VS_PLATFORM "macos")
    elseif (UNIX)
        message(STATUS "Detected target platform: [Unix]")
        set(VS_PLATFORM "linux")
    elseif (MINGW32)
        message(STATUS "Detected target platform: [Windows]")
        set(VS_PLATFORM "windows")
    endif ()
endif ()

# ---------------------------------------------------------------------------
# Get prebuild core libraries
# ---------------------------------------------------------------------------
get_prebuild()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
message("-- Build type: [${CMAKE_BUILD_TYPE}]")

#if(CMAKE_BUILD_TYPE_LOWER STREQUAL "minsizerel" OR CMAKE_BUILD_TYPE_LOWER STREQUAL "relwithdebinfo")
#    set(CMAKE_BUILD_TYPE_LOWER "release")
#endif()

set(CMAKE_BUILD_TYPE_LOWER "release")

set(PREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/prebuilt")
set(PREBUILT_QT_DIR "${PREBUILT_DIR}/qt")

if (VS_IOS_SIMULATOR AND VS_PLATFORM STREQUAL "ios")
    set(VS_IOS_SIM_PREFIX "-sim")
endif ()

if (VS_PLATFORM STREQUAL "android")
    set(PREBUILT_BASE_DIR "${PREBUILT_DIR}/${VS_PLATFORM}.${ANDROID_ABI}/${CMAKE_BUILD_TYPE_LOWER}/installed/usr/local")
else ()
    set(PREBUILT_BASE_DIR "${PREBUILT_DIR}/${VS_PLATFORM}${VS_IOS_SIM_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/installed/usr/local")
endif ()

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
if (VS_PLATFORM STREQUAL "ios")
    if (NOT XCODE_VERSION)
        message(FATAL_ERROR "Xcode only")
    endif ()

    # QT Extra packages
    list(APPEND VS_QT_EXTRA QmlImportScanner Gui)

    # Android
elseif (VS_PLATFORM STREQUAL "android")
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
    else ()
        set(VS_BUILD_NUMBER "0")
    endif ()
    PAD_STRING(TMP_VERS 3 "0" "${VS_BUILD_NUMBER}")
    string(APPEND ANDROID_VERSION_CODE "${TMP_VERS}")
    message(STATUS "Android version name: [${ANDROID_VERSION_NAME}]")
    message(STATUS "Android version code: [${ANDROID_VERSION_CODE}]")

    # Android QT Extra
    string(APPEND VS_QT_EXTRA "AndroidExtras")
    # Android gradle path
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platforms/android")

    # Linux
elseif (VS_PLATFORM STREQUAL "linux")
    string(APPEND VS_QT_EXTRA "Widgets")

    # MacOS
elseif (VS_PLATFORM STREQUAL "macos")
    string(APPEND VS_QT_EXTRA "Widgets")

endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
include(FetchContent)

find_package(Qt5 COMPONENTS Core Quick Sql Network Concurrent Qml Xml ${VS_QT_EXTRA} REQUIRED)

# ---------------------------------------------------------------------------
#   Prepare for build target
# ---------------------------------------------------------------------------
# Add deployers targets
include(deployers)

# Android
if (VS_PLATFORM STREQUAL "android")
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
    endif ()

    message(STATUS "Android OpenSSL root path:    [${SSL_ROOT_PATH}]")
    message(STATUS "Android OpenSSL library path: [${VS_ANDROID_SSL_DIR}]")
    message(STATUS "Android extra libraries:      [${ANDROID_EXTRA_LIBS}]")

    # Linux
elseif (VS_PLATFORM STREQUAL "linux")
    add_executable(${VS_TARGET_NAME})

    # MacOS
elseif (VS_PLATFORM STREQUAL "macos")
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
elseif (VS_PLATFORM STREQUAL "ios")

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

    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,-e,_qt_main_wrapper")
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
    set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)

    add_executable(${VS_TARGET_NAME} MACOSX_BUNDLE)

    # Settings for signing
    if (VS_CODE_SIGN_IDENTITY)
        SET_XCODE_PROPERTY(${VS_TARGET_NAME} CODE_SIGN_IDENTITY "${VS_CODE_SIGN_IDENTITY}")
    endif ()
    if (VS_DEVELOPMENT_TEAM)
        SET_XCODE_PROPERTY(${VS_TARGET_NAME} DEVELOPMENT_TEAM "${VS_DEVELOPMENT_TEAM}")
    endif ()
    if (VS_PROVISIONING_PROFILE_SPECIFIER)
        SET_XCODE_PROPERTY(${VS_TARGET_NAME} PROVISIONING_PROFILE_SPECIFIER "${VS_PROVISIONING_PROFILE_SPECIFIER}")
    endif ()
    SET_XCODE_PROPERTY(${VS_TARGET_NAME} IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")
    SET_XCODE_PROPERTY(${VS_TARGET_NAME} CODE_SIGN_ENTITLEMENTS "${VS_CUSTOMER_DIR}/platforms/ios/Entitlements/VirgilMessenger.entitlements")
    SET_XCODE_PROPERTY(${VS_TARGET_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")

    set_target_properties(${VS_TARGET_NAME}
            PROPERTIES
            XCODE_ATTRIBUTE_INSTALL_PATH "/Applications"
            XCODE_ATTRIBUTE_SKIP_INSTALL "No"
            )
    # Storyboard default
    configure_file("${CMAKE_CURRENT_LIST_DIR}/platforms/ios/LaunchScreen.storyboard.in"
                   "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/LaunchScreen.storyboard" @ONLY)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/platforms/ios/Default-568h@2x.png"
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
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/platforms/ios/Info.plist.in"
            LINK_FLAGS "-framework Foundation -framework UserNotifications"
            RESOURCE "${STORYBOARD_DEFAULT}"
            )

    qt5_import_qml_plugins(${VS_TARGET_NAME})
else ()
    message(FATAL_ERROR "Unknown target platform")
endif ()

# ---------------------------------------------------------------------------
#   Target sources and libraries
# ---------------------------------------------------------------------------
target_sources(${VS_TARGET_NAME}

        PRIVATE

        #
        #  Includes
        #
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQApplication.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQClipboardProxy.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQContactManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/CrashReporter.h
        ${CMAKE_CURRENT_LIST_DIR}/include/FileLoader.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQDiscoveryManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQLastActivityIq.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQLastActivityManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/Messenger.h
        ${CMAKE_CURRENT_LIST_DIR}/include/Settings.h
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQNetworkAnalyzer.h
        ${CMAKE_CURRENT_LIST_DIR}/include/Utils.h
        ${CMAKE_CURRENT_LIST_DIR}/include/android/VSQAndroid.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ui/VSQUiHelper.h
        ${CMAKE_CURRENT_LIST_DIR}/include/KeyboardEventFilter.h
        ${CMAKE_CURRENT_LIST_DIR}/include/Validator.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ContactAvatarLoader.h
        ${CMAKE_CURRENT_LIST_DIR}/include/CustomerEnv.h
        ${CMAKE_CURRENT_LIST_DIR}/include/Contact.h
        ${CMAKE_CURRENT_LIST_DIR}/include/ConfirmationCodeType.h
        ${CMAKE_CURRENT_LIST_DIR}/include/CloudFile.h
        ${CMAKE_CURRENT_LIST_DIR}/include/CloudFileId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/CloudFileUpdate.h
        # Messenger Core
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/AttachmentId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/Chat.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/ChatId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/ChatType.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CloudFsFileId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CloudFsFileInfo.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CloudFsFolder.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CloudFsFolderId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CloudFsFolderInfo.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CloudFsNewFile.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CommKitBridge.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CoreMessenger.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CoreMessengerCloudFs.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/CoreMessengerStatus.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/Group.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/GroupId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/GroupInvitationStatus.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/GroupUpdate.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/IncomingMessage.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/IncomingMessageStage.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/Message.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContent.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentAttachment.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentDownloadStage.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentEncrypted.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentFile.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentJsonUtils.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentPicture.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentText.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentType.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageContentUploadStage.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageGroupChatInfo.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageSender.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageStatus.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageUpdate.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/MessageUpdateable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/OutgoingMessage.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/OutgoingMessageStage.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/User.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/UserId.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messenger/UserImpl.h
        # Controllers
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/Controller.h
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/AttachmentsController.h
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/ChatsController.h
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/CloudFilesController.h
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/Controllers.h
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/MessagesController.h
        ${CMAKE_CURRENT_LIST_DIR}/include/controllers/UsersController.h
        # Helpers
        ${CMAKE_CURRENT_LIST_DIR}/include/helpers/VSQSingleton.h
        ${CMAKE_CURRENT_LIST_DIR}/include/helpers/FutureWorker.h
        ${CMAKE_CURRENT_LIST_DIR}/include/helpers/FileUtils.h
        # Applications states
        ${CMAKE_CURRENT_LIST_DIR}/include/states/AccountSelectionState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/AccountSettingsState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/ApplicationStateManager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/AttachmentPreviewState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/BackupKeyState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/CloudFileListState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/EditProfileState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/VerifyProfileState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/ChatListState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/ChatState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/DownloadKeyState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/OperationState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/NewChatState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/NewGroupChatState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/NameGroupChatState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/SignInAsState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/SignInState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/SignInUsernameState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/SignUpState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/SplashScreenState.h
        ${CMAKE_CURRENT_LIST_DIR}/include/states/StartState.h
        # Database
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/Database.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/DatabaseTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/DatabaseUtils.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/Migration.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/Patch.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/ScopedConnection.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/core/ScopedTransaction.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/AttachmentsTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/ContactsTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/ChatsTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/CloudFilesTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/GroupMembersTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/GroupsTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/MessagesTable.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/UserDatabase.h
        ${CMAKE_CURRENT_LIST_DIR}/include/database/UserDatabaseMigration.h
        # Logging
        ${CMAKE_CURRENT_LIST_DIR}/include/logging/VSQLogging.h
        ${CMAKE_CURRENT_LIST_DIR}/include/logging/VSQLogWorker.h
        ${CMAKE_CURRENT_LIST_DIR}/include/logging/VSQMessageLogContext.h
        # Models
        ${CMAKE_CURRENT_LIST_DIR}/include/models/AccountSelectionModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/ChatsModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/ContactsModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/ContactsProxyModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/DiscoveredContactsModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/DiscoveredContactsProxyModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/CloudFileOperationSource.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/CloudFilesModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/CloudFilesQueue.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/CloudFilesUploader.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/ListModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/ListProxyModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/ListSelectionModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/MessageOperationSource.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/MessagesModel.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/MessagesQueue.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/Models.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/Model.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/OperationQueue.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/OperationSource.h
        ${CMAKE_CURRENT_LIST_DIR}/include/models/UploadDownloadModel.h
        # Operations
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CalculateAttachmentFingerprintOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CalculateFileFingerprintOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CloudFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/ConvertToPngOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CreateAttachmentPreviewOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CreateAttachmentThumbnailOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CreateCloudFolderOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/CreateThumbnailOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/DecryptFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/DeleteCloudFilesOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/DownloadAttachmentOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/DownloadFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/DownloadDecryptFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/EncryptFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/EncryptUploadFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/LoadAttachmentOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/LoadFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/MessageOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/MessageOperationFactory.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/NetworkOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/Operation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/SendMessageOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/UploadAttachmentOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/UploadFileOperation.h
        ${CMAKE_CURRENT_LIST_DIR}/include/operations/UploadCloudFileOperation.h
        # Generated
        ${CMAKE_CURRENT_LIST_DIR}/include/VSQCustomer.h
        # MacOS
        $<$<STREQUAL:"${VS_PLATFORM}","macos">:${CMAKE_CURRENT_LIST_DIR}/include/macos/VSQMacos.h>
        # IOS
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/PushNotifications.h>
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/XmppPushNotifications.h>
        # Android
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/PushNotifications.h>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/XmppPushNotifications.h>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/android/FirebaseListener.h>

        #
        #  Sources
        #
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQClipboardProxy.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQContactManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/CrashReporter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/FileLoader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQDiscoveryManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Messenger.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQLastActivityIq.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQLastActivityManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Settings.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQNetworkAnalyzer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/android/VSQAndroid.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/VSQApplication.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/ui/VSQUiHelper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/KeyboardEventFilter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/Validator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/ContactAvatarLoader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/CustomerEnv.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/CloudFile.cpp
        # Messenger Core
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/AttachmentId.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/Chat.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/ChatId.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/ChatType.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CommKitBridge.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CoreMessenger.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CloudFsFileId.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CloudFsFileInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CloudFsFolder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CloudFsFolderId.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CloudFsFolderInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CloudFsNewFile.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/CoreMessengerCloudFs.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/Group.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/GroupInvitationStatus.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/GroupUpdate.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/IncomingMessage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/IncomingMessageStage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/Message.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentAttachment.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentDownloadStage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentEncrypted.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentFile.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentJsonUtils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentPicture.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentText.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentType.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageContentUploadStage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageGroupChatInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageId.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageStatus.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/MessageUpdate.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/OutgoingMessage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/OutgoingMessageStage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/User.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messenger/UserId.cpp
        # Controllers
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/Controller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/AttachmentsController.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/ChatsController.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/CloudFilesController.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/Controllers.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/MessagesController.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/controllers/UsersController.cpp
        # Helpers
        ${CMAKE_CURRENT_LIST_DIR}/src/helpers/FileUtils.cpp
        # Applications states
        ${CMAKE_CURRENT_LIST_DIR}/src/states/AccountSelectionState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/AccountSettingsState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/ApplicationStateManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/AttachmentPreviewState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/BackupKeyState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/EditProfileState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/VerifyProfileState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/ChatListState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/ChatState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/CloudFileListState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/DownloadKeyState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/NewChatState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/NewGroupChatState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/NameGroupChatState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/SignInAsState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/SignInState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/SignInUsernameState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/SignUpState.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/states/SplashScreenState.cpp
        # Database
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/Database.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/DatabaseTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/DatabaseUtils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/Migration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/Patch.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/ScopedConnection.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/core/ScopedTransaction.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/AttachmentsTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/ContactsTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/ChatsTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/CloudFilesTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/GroupMembersTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/GroupsTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/MessagesTable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/UserDatabase.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/UserDatabaseMigration.cpp
        # Logging
        ${CMAKE_CURRENT_LIST_DIR}/src/logging/VSQLogging.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/logging/VSQLogWorker.cpp
        # Models
        ${CMAKE_CURRENT_LIST_DIR}/src/models/AccountSelectionModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/ChatsModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/CloudFileOperationSource.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/CloudFilesModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/CloudFilesQueue.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/CloudFilesUploader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/ContactsModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/ContactsProxyModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/DiscoveredContactsModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/DiscoveredContactsProxyModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/ListModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/ListProxyModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/ListSelectionModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/MessageOperationSource.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/MessagesModel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/MessagesQueue.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/Models.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/Model.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/OperationQueue.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/models/UploadDownloadModel.cpp
        # Operations
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CalculateAttachmentFingerprintOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CalculateFileFingerprintOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CloudFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/ConvertToPngOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CreateAttachmentPreviewOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CreateAttachmentThumbnailOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CreateCloudFolderOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/CreateThumbnailOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/DecryptFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/DeleteCloudFilesOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/DownloadAttachmentOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/DownloadFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/DownloadDecryptFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/EncryptFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/EncryptUploadFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/LoadAttachmentOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/LoadFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/MessageOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/MessageOperationFactory.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/NetworkOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/Operation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/SendMessageOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/UploadAttachmentOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/UploadFileOperation.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/operations/UploadCloudFileOperation.cpp
        # QT QRC
        ${CMAKE_CURRENT_LIST_DIR}/src/resources.qrc
        ${VS_CUSTOMER_DIR}/src/customer.qrc
        # Linux
        $<$<STREQUAL:"${VS_PLATFORM}","linux">:${CMAKE_CURRENT_LIST_DIR}/platforms/default/CustomerEnvDefault.cpp>
        # MACOS
        $<$<STREQUAL:"${VS_PLATFORM}","macos">:${CMAKE_CURRENT_LIST_DIR}/platforms/apple/CustomerEnvApple.mm>
        $<$<STREQUAL:"${VS_PLATFORM}","macos">:${CMAKE_CURRENT_LIST_DIR}/src/macos/VSQMacos.mm>
        # IOS
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/platforms/apple/CustomerEnvApple.mm>
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/platforms/ios/AppDelegate.mm>
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/PushNotifications.cpp>
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/XmppPushNotifications.cpp>
        # Android
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/PushNotifications.cpp>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/XmppPushNotifications.cpp>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/src/notifications/android/FirebaseListener.cpp>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/platforms/default/CustomerEnvDefault.cpp>

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
        $<$<STREQUAL:"${VS_PLATFORM}","android">:Qt5::AndroidExtras>
        $<$<STREQUAL:"${VS_PLATFORM}","android">:log>
        $<$<STREQUAL:"${VS_PLATFORM}","windows">:Qt5::Widgets>
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:Qt5::Gui>
        $<$<STREQUAL:"${VS_PLATFORM}","ios">:Qt5::QIosOptionalPlugin_NSPhotoLibrary>
        Qt5::Sql
        Qt5::Network
        Qt5::Concurrent
        Qt5::Qml
        Qt5::Xml

        # Other libs
        qxmpp

        vsc_comm_kit
        vsc_comm_kit_pb
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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/helpers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/database>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/models>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/ui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/logging>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/controllers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/states>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/operations>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/messenger>

        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PREBUILT_INCLUDE_DIR}/qxmpp>

        $<BUILD_INTERFACE:${VS_CUSTOMER_INCLUDE_DIR}>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${CMAKE_CURRENT_LIST_DIR}/include/notifications/android>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:${PREBUILT_DIR}/firebase_cpp_sdk/include>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","macos">:${PREBUILT_DIR}/${VS_PLATFORM}/sparkle/Sparkle.framework/Headers>>
        )

target_compile_definitions(${VS_TARGET_NAME}
        PRIVATE

        $<BUILD_INTERFACE:VERSION="${VS_TARGET_VERSION}">
        $<BUILD_INTERFACE:VS_MSGR_ENV_DEV=1>
        $<BUILD_INTERFACE:VS_MSGR_ENV_STG=0>
        $<BUILD_INTERFACE:VS_MSGR_ENV_PROD=0>

        $<BUILD_INTERFACE:INFO_CLIENT=1>
        $<BUILD_INTERFACE:CFG_CLIENT=1>
        $<BUILD_INTERFACE:VERSION_DATABASE_SCHEME=${VERSION_DATABASE_SCHEME}>
        $<BUILD_INTERFACE:VS_CUSTOMER=${VS_CUSTOMER}>

        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:VS_ANDROID=1>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:VS_MOBILE=1>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","android">:VS_PUSHNOTIFICATIONS=1>>

        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","ios">:VS_IOS=1>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","ios">:VS_MOBILE=1>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","ios">:VS_PUSHNOTIFICATIONS=1>>
        $<BUILD_INTERFACE:$<$<BOOL:${VS_IOS_SIMULATOR}>:VS_IOS_SIMULATOR=1>>

        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","macos">:VS_MACOS=1>>
        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","macos">:VS_DESKTOP=1>>

        $<BUILD_INTERFACE:$<$<STREQUAL:"${VS_PLATFORM}","linux">:VS_DESKTOP=1>>

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
