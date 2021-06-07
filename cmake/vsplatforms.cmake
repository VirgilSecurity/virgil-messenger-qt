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
# This little macro lets you set any XCode specific property
# ---------------------------------------------------------------------------
macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property(TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro(set_xcode_property)

# ---------------------------------------------------------------------------
# Set CMAKE_FIND_ROOT_PATH for QT cmake include directory
# ---------------------------------------------------------------------------
function(PREPARE_QT_SDK CMAKE_PREFIX_PATH_OUT CMAKE_FIND_ROOT_PATH_OUT QT_QMAKE_EXECUTABLE_OUT QT_RELATIVE_PATH_OUT)
    if(DEFINED ENV{QTDIR})
        get_filename_component(QT_RELATIVE_PATH "$ENV{QTDIR}/../" ABSOLUTE)
        set(${CMAKE_FIND_ROOT_PATH_OUT} "${QT_RELATIVE_PATH}/${QT_PREFIX_PATH}" PARENT_SCOPE)
        set(${CMAKE_PREFIX_PATH_OUT} "${QT_RELATIVE_PATH}/${QT_PREFIX_PATH}" PARENT_SCOPE)
        set(${QT_QMAKE_EXECUTABLE_OUT} "${QT_RELATIVE_PATH}/${QT_PREFIX_PATH}/bin/qmake" PARENT_SCOPE)
        set(${QT_RELATIVE_PATH_OUT} "${QT_RELATIVE_PATH}" PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Error detecting QT SDK (Env QTDIR not set)")
    endif()
endfunction()

# ---------------------------------------------------------------------------
#   Qt creator runs cmake several time for Multi ABI configuration, so
#   next configurable variables must be preserved:
#       - VS_CUSTOMER
#       - VS_ENV
#       - CMAKE_BUILD_TYPE
#
#   For this purpose file "transitive-args.cmake" is used.
#   Function ADD_TRANSITIVE_ARG() writes variable and it's value to the file.
# ---------------------------------------------------------------------------
set(TRANSITIVE_ARGS_FILE "${CMAKE_SOURCE_DIR}/transitive-args.cmake")

function(ADD_TRANSITIVE_ARG path name value)
    if(NOT value)
        return()
    endif()

    if(NOT EXISTS "${path}")
        file(WRITE "${path}" "")
    endif ()

    file(STRINGS "${path}" file_content)

    if(NOT file_content MATCHES "${name}")
        file(APPEND "${path}" "set(${name} \"${value}\" CACHE STRING \"\" FORCE)\n")
    endif()
endfunction()

if(EXISTS "${TRANSITIVE_ARGS_FILE}")
    include("${TRANSITIVE_ARGS_FILE}")
    message(STATUS "Load cached configuration from ${TRANSITIVE_ARGS_FILE}")
endif()

#
#   Define: VS_PLATFORM
#
if(NOT VS_PLATFORM)
    if(MINGW32 OR MINGW64 OR CYGWIN OR WIN32)
        message(STATUS "Detected target platform: [Windows]")
        set(PLATFORM "windows")

    elseif(ANDROID OR ANDROID_NDK OR ANDROID_PLATFORM OR ANDROID_ABI)
        message(STATUS "Detected target platform: [Android]")
        set(PLATFORM "android")

    elseif(IOS)
        message(STATUS "Detected target platform: [iOS]")
        set(PLATFORM "ios")

    elseif(APPLE)
        message(STATUS "Detected target platform: [macOS]")
        set(PLATFORM "macos")

    elseif(UNIX)
        message(STATUS "Detected target platform: [Unix]")
        set(PLATFORM "linux")

    else()
        message(FATAL_ERROR "Can not detect target platform. Expected VS_PLATFORM={macos, ios, windows, android}")
    endif()

    set(VS_PLATFORM "${PLATFORM}" CACHE STRING "Target platform: macos, ios, windows, android")
endif()

#
#   Define: VS_ENV
#
set(VS_ENV "prod" CACHE STRING "Target environment: prod, env, dev")
message(STATUS "Current environment : [${VS_ENV}]")

#
#   Write options to the "transitive-args.cmake".
#
add_transitive_arg("${TRANSITIVE_ARGS_FILE}" VS_ENV "${VS_ENV}")
add_transitive_arg("${TRANSITIVE_ARGS_FILE}" VS_CUSTOMER "${VS_CUSTOMER}")
add_transitive_arg("${TRANSITIVE_ARGS_FILE}" CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}")

# ---------------------------------------------------------------------------
# Prepare target platform
# ---------------------------------------------------------------------------
list(APPEND VS_PLATFORM_LIST "linux" "android" "ios" "macos" "windows")
set(VS_DESKTOP_PLATFORM_LIST "linux" "macos" "windows")
set(VS_MOBILE_PLATFORM_LIST "ios" "android")


if(VS_PLATFORM)
    message(STATUS "Autodetecting environment for target platform: [${VS_PLATFORM}]")
    # Check platform name
    if(NOT VS_PLATFORM IN_LIST VS_PLATFORM_LIST)
        message(FATAL_ERROR "-- Target platform if wrong. Set VS_PLATFORM to [ ${VS_PLATFORM_LIST} ]")
    endif()
    # -- Linux
    if(VS_PLATFORM STREQUAL "linux")
        set(QT_PREFIX_PATH "gcc_64")

    # -- Android
    elseif(VS_PLATFORM STREQUAL "android")
        set(QT_PREFIX_PATH "android")
        #   Android NDK ABIs
        set(ANDROID_ABI "x86_64" CACHE STRING "Android default ABI")
        set(ANDROID_BUILD_ABI_arm64-v8a ON CACHE BOOL "Build arm64-v8a architecture")
        set(ANDROID_BUILD_ABI_armeabi-v7a ON CACHE BOOL "Build armeabi-v7a architecture")
        set(ANDROID_BUILD_ABI_x86 ON CACHE BOOL "Build x86 architecture")
        set(ANDROID_BUILD_ABI_x86_64 ON CACHE BOOL "Build x86_64 architecture")

        set(ANDROID_MIN_SDK_VERSION "26")
        set(ANDROID_TARGET_SDK_VERSION "29")
        set(ANDROID_SDK_PLATFORM "${ANDROID_TARGET_SDK_VERSION}")
        set(ANDROID_NATIVE_API_LEVEL "${ANDROID_MIN_SDK_VERSION}")

        message(STATUS "Android API level: [${ANDROID_NATIVE_API_LEVEL}]")
        message(STATUS "Android ABI: [${ANDROID_ABI}]")
        message(STATUS "ANDROID_BUILD_ABI_arm64-v8a: ${ANDROID_BUILD_ABI_arm64-v8a}")
        message(STATUS "ANDROID_BUILD_ABI_armeabi-v7a: ${ANDROID_BUILD_ABI_armeabi-v7a}")
        message(STATUS "ANDROID_BUILD_ABI_x86: ${ANDROID_BUILD_ABI_x86}")
        message(STATUS "ANDROID_BUILD_ABI_x86_64: ${ANDROID_BUILD_ABI_x86_64}")

        #  Android NDK
        if(NOT CMAKE_TOOLCHAIN_FILE)
            if(ANDROID_NDK)
                set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake" CACHE PATH "")
            elseif(DEFINED ENV{ANDROID_NDK})
                set(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake" CACHE PATH "")
            else ()
                message(FATAL_ERROR "-- Enviroment variable ANDROID_NDK not set")
            endif()
        endif()
        message(STATUS "Android toolchain file: [${CMAKE_TOOLCHAIN_FILE}]")
        #  Android SDK
        if(ANDROID_SDK)
            message(STATUS "ANDROID_SDK: [${ANDROID_SDK}]")
        elseif(DEFINED ENV{ANDROID_SDK})
            set(ANDROID_SDK "$ENV{ANDROID_SDK}")
            message(STATUS "Set ANDROID_SDK from environment value: [ ${ANDROID_SDK} ]")
        else ()
            message(FATAL_ERROR "-- Enviroment variable ANDROID_SDK not set")
        endif()

        execute_process(COMMAND bash -c "cd ${ANDROID_SDK}/build-tools && ls -rd -- */ | head -n 1 | cut -d'/' -f1 | tr -d '\n'" OUTPUT_VARIABLE ANDROID_SDK_BUILD_TOOLS_REVISION)
        message(STATUS "Android SDK build tool version: [${ANDROID_SDK_BUILD_TOOLS_REVISION}]")
        set(QT_ANDROID_DEPLOYMENT_DEPENDENCIES "\"sdkBuildToolsRevision\": \"${ANDROID_SDK_BUILD_TOOLS_REVISION}\",")

    # -- MacOS
    elseif(VS_PLATFORM STREQUAL "macos")
        set(QT_PREFIX_PATH "clang_64")

    # -- IOS
    elseif(VS_PLATFORM STREQUAL "ios" AND NOT VS_IOS_SIMULATOR)
        set(QT_PREFIX_PATH "ios")
        set(CMAKE_SYSTEM_NAME "iOS")

    # -- IOS-SIM
    elseif(VS_PLATFORM STREQUAL "ios" AND VS_IOS_SIMULATOR)
        set(QT_PREFIX_PATH "ios")
        set(CMAKE_SYSTEM_NAME "iOS")
        execute_process(COMMAND xcodebuild -version -sdk iphonesimulator Path
                OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "Using SDK: ${CMAKE_OSX_SYSROOT} for platform: IOS Simulator")
        if(NOT EXISTS ${CMAKE_OSX_SYSROOT})
            message(FATAL_ERROR "Invalid CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT} does not exist.")
        endif()

    # -- Windows
    elseif(VS_PLATFORM STREQUAL "windows")
        set(QT_PREFIX_PATH "mingw81_64")

    else()
        message(FATAL_ERROR "Unhandled VS_PLATFORM=${VS_PLATFORM}.")
    endif()

    prepare_qt_sdk(CMAKE_PREFIX_PATH CMAKE_FIND_ROOT_PATH QT_QMAKE_EXECUTABLE QT_RELATIVE_PATH)
endif()

message(STATUS "CMAKE_PREFIX_PATH: [${CMAKE_PREFIX_PATH}]")
message(STATUS "CMAKE_FIND_ROOT_PATH: [${CMAKE_FIND_ROOT_PATH}]")
