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

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# ---------------------------------------------------------------------------
#   System introspection
# ---------------------------------------------------------------------------
include("${ANDROID_SDK}/android_openssl/CMakeLists.txt")

message(STATUS "Prepare sources for Android ...")
message(STATUS "Android OpenSSL root path:    [${SSL_ROOT_PATH}]")
message(STATUS "Android extra libraries:      [${ANDROID_EXTRA_LIBS}]")

if(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/arm")

elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/arm64")

elseif(ANDROID_ABI STREQUAL "x86")
    set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/x86")

elseif(ANDROID_ABI STREQUAL "x86_64")
    set(VS_ANDROID_SSL_DIR "${SSL_ROOT_PATH}/latest/x86_64")
endif()

message(STATUS "Android OpenSSL library path: [${VS_ANDROID_SSL_DIR}]")

get_firebase("${PROJECT_SOURCE_DIR}/ext")

find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)

# ---------------------------------------------------------------------------
#   Libraries
# ---------------------------------------------------------------------------
#
#   Library: platform-deps.
#
add_library(platform-deps INTERFACE)
target_link_libraries(platform-deps
        INTERFACE
        Qt5::AndroidExtras
        log

        curl
        ssl_1_1
        crypto_1_1
        )

target_include_directories(platform-deps
        SYSTEM INTERFACE
        "$<BUILD_INTERFACE:${PREBUILT_DIR}/firebase_cpp_sdk/include>"
        )

target_link_directories(platform-deps
        INTERFACE
        "$<BUILD_INTERFACE:${PREBUILT_DIR}/firebase_cpp_sdk/libs/android/${ANDROID_ABI}/c++>"
        "$<BUILD_INTERFACE:${VS_ANDROID_SSL_DIR}>"
        )

#
#   Library: platform.
#
add_library(platform STATIC)

target_sources(platform
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/native/include/PlatformAndroid.h"
        "${CMAKE_CURRENT_LIST_DIR}/native/include/FirebaseListener.h"

        "${CMAKE_CURRENT_LIST_DIR}/native/src/PlatformAndroid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/native/src/FirebaseListener.cpp"
        )

target_include_directories(platform
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/native/include>"
        )

target_link_libraries(platform
        PUBLIC
        platform-deps
        platform-base
        customer

        firebase_messaging
        firebase_app
        firebase_auth
        )

#
#   Library: platform-fs.
#
add_library(platform-fs STATIC)

target_sources(platform-fs
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/native/include/PlatformFsAndroid.h"
        "${CMAKE_CURRENT_LIST_DIR}/native/src/PlatformFsAndroid.cpp"
        )

target_include_directories(platform-fs
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/native/include>"
        )

target_link_libraries(platform-fs
        PUBLIC
        Qt5::Core
        Qt5::AndroidExtras

        platform-definitions
        platform-fs-api
        )

#
#   Library: platform-gui.
#
add_library(platform-gui STATIC)

target_sources(platform-gui
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/native/include/PlatformDocumentInteractionControllerAndroid.h"
        "${CMAKE_CURRENT_LIST_DIR}/native/src/PlatformDocumentInteractionControllerAndroid.cpp"
        )

target_include_directories(platform-gui
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/native/include>"
        )

target_link_libraries(platform-gui
        PUBLIC
        platform
        platform-fs
        platform-gui-api

        Qt5::Gui
        )

# ---------------------------------------------------------------------------
#   Configure.
# ---------------------------------------------------------------------------
#
#  Check.
#
if(ANDROID_PACKAGE_SOURCE_DIR)
    set(GRADLE_BUILD_DIR "${ANDROID_PACKAGE_SOURCE_DIR}")
else()
    message(FATAL_ERROR "CMake variable ANDROID_PACKAGE_SOURCE_DIR is undefined, but should point to a Gradle project")
endif()


#
#  Copy customer's files first.
#
file(COPY "${VS_CUSTOMER_DIR}/platforms/android/" DESTINATION "${GRADLE_BUILD_DIR}")

#
#  Copy Gradle support files.
#
file(COPY "${CMAKE_CURRENT_LIST_DIR}/gradle" DESTINATION "${GRADLE_BUILD_DIR}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/build.gradle" DESTINATION "${GRADLE_BUILD_DIR}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/gradlew" DESTINATION "${GRADLE_BUILD_DIR}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/gradlew.bat" DESTINATION "${GRADLE_BUILD_DIR}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/project.properties" DESTINATION "${GRADLE_BUILD_DIR}")

#
#  Copy Source file.
#
file(COPY "${CMAKE_CURRENT_LIST_DIR}/src" DESTINATION "${GRADLE_BUILD_DIR}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/res" DESTINATION "${GRADLE_BUILD_DIR}")

#
#  Configure Gradle build.
#
configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/AndroidManifest.xml.in"
        "${GRADLE_BUILD_DIR}/AndroidManifest.xml"
        @ONLY
        )

configure_file(
        "${GRADLE_BUILD_DIR}/google-services.json.in"
        "${GRADLE_BUILD_DIR}/google-services.json"
        @ONLY
        )

set(VS_FIREBASE_DIR "${PREBUILT_DIR}/firebase_cpp_sdk")
configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/gradle.properties.in"
        "${GRADLE_BUILD_DIR}/gradle.properties"
        @ONLY
        )

# ---------------------------------------------------------------------------
#   Main target.
# ---------------------------------------------------------------------------
add_library(${VS_TARGET_NAME} SHARED)

add_custom_command(TARGET ${VS_TARGET_NAME} POST_BUILD
      COMMAND touch "${CMAKE_BINARY_DIR}/CMakeCache.txt"
      COMMENT "Touch CMakeCache.txt"
)

# ---------------------------------------------------------------------------
#   Deploy
# ---------------------------------------------------------------------------
find_program(DEPLOY_QT androiddeployqt)

if(DEFINED ENV{JAVA_HOME})
    set(JAVA_HOME $ENV{JAVA_HOME} CACHE INTERNAL "Saved JAVA_HOME variable")
elseif(EXISTS "/etc/alternatives/java_sdk")
    set(JAVA_HOME "/etc/alternatives/java_sdk" CACHE INTERNAL "Saved JAVA_HOME variable")
endif()

if(JAVA_HOME)
    set(android_deploy_qt_jdk "--jdk ${JAVA_HOME}")
    message(STATUS "Java home: [${JAVA_HOME}]")
endif()

if(ANDROID_SDK_PLATFORM)
    set(android_deploy_qt_platform "--android-platform ${ANDROID_SDK_PLATFORM}")
    message(STATUS "Android deploy QT platform: [${ANDROID_SDK_PLATFORM}]")
endif()

set(QT_ANDROID_APPLICATION_BINARY ${VS_TARGET_NAME})

add_custom_target(apk_release
        COMMENT "Making APK release..."
        COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${DEPLOY_QT}
        --input "${CMAKE_BINARY_DIR}/android_deployment_settings.json"
        --output "${CMAKE_BINARY_DIR}/android-build"
        --apk "${CMAKE_BINARY_DIR}/android-build/${VS_TARGET_NAME}.apk"
        ${android_deploy_qt_platform}
        ${android_deploy_qt_jdk}
        --gradle
        --sign "${VS_KEYCHAIN}" "${VS_KEY_ALIAS}"
        --storepass "${VS_KEYCHAIN_PASSWORD}"
        --keypass "${VS_KEY_PASSWORD}"
        --no-gdbserver
        VERBATIM
        )

add_custom_target(aab_release
        COMMENT "Making AAB release..."
        COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${DEPLOY_QT}
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
        VERBATIM
        )
