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
# Check target platform variable
# ---------------------------------------------------------------------------
list(APPEND VS_PLATFORM_LIST "linux" "android" "ios" "ios-sim" "macos" "windows")

if(VS_PLATFORM)
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/../transitive-args.cmake" "set(VS_PLATFORM \"${VS_PLATFORM}\" CACHE \"STRING\" \"Target build platform\")")
    message(STATUS "Create transitive-args.cmake") 
else()
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../transitive-args.cmake")
	include("${CMAKE_CURRENT_LIST_DIR}/../transitive-args.cmake")
	message(STATUS "Set VS_PLATFORM from  transitive-args.cmake") 
    endif()
endif()    

if(VS_PLATFORM)
    if(NOT VS_PLATFORM IN_LIST VS_PLATFORM_LIST)
	message(FATAL_ERROR "-- Target platform if wrong. Set VS_PLATFORM to [ ${VS_PLATFORM_LIST} ]")
    endif()
else()
    message(FATAL_ERROR "-- Target platform not set. Set VS_PLATFORM to [ ${VS_PLATFORM_LIST} ]")
endif()
message("-- Target platform VS_PLATFORM = ${VS_PLATFORM}")

# ---------------------------------------------------------------------------
# Prepare target platform
# ---------------------------------------------------------------------------

# -- Linux
if(VS_PLATFORM STREQUAL "linux")
    set(QT_PREFIX_PATH "gcc_64")

# -- Android
elseif(VS_PLATFORM STREQUAL "android")
    set(QT_PREFIX_PATH "android")
    #   Android NDK ABIs    
    set(ANDROID_NATIVE_API_LEVEL "28" CACHE "STRING" "Android API level")
    set(ANDROID_ABI "x86" CACHE "STRING" "Android default ABI")
    set(ANDROID_BUILD_ABI_arm64-v8a ON CACHE "BOOL" "Build arm64-v8a architecture")
    set(ANDROID_BUILD_ABI_armeabi-v7a ON CACHE "BOOL" "Build armeabi-v7a architecture")
    set(ANDROID_BUILD_ABI_x86 ON CACHE "BOOL" "Build x86 architecture")
    set(ANDROID_BUILD_ABI_x86_64 ON CACHE "BOOL" "Build x86_64 architecture")

    message(STATUS "Android API level: [${ANDROID_NATIVE_API_LEVEL}]")    
    message(STATUS "Android default ABI: [${ANDROID_ABI}]")    
    message(STATUS "ANDROID_BUILD_ABI_arm64-v8a: ${ANDROID_BUILD_ABI_arm64-v8a}")		
    message(STATUS "ANDROID_BUILD_ABI_armeabi-v7a: ${ANDROID_BUILD_ABI_armeabi-v7a}")		    	
    message(STATUS "ANDROID_BUILD_ABI_x86: ${ANDROID_BUILD_ABI_x86}")		    	
    message(STATUS "ANDROID_BUILD_ABI_x86_64: ${ANDROID_BUILD_ABI_x86_64}")		    	
    #  Android NDK
    if (NOT DEFINED ENV{ANDROID_NDK} AND NOT CMAKE_TOOLCHAIN_FILE)    
	message(FATAL_ERROR "-- Enviroment variable ANDROID_NDK not set")        
    elseif(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{ANDROID_NDK})
	set(VS_CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake")
    endif()
    #  Android SDK
    if (DEFINED ENV{ANDROID_SDK} AND NOT ANDROID_SDK)
        set(ANDROID_SDK "$ENV{ANDROID_SDK}")
	message("-- Set ANDROID_SDK from enviroment value: [ ${ANDROID_SDK} ]")        
    else()
	message(FATAL_ERROR "-- Enviroment variable ANDROID_SDK not set")    
    endif()    

# -- MacOS
elseif(VS_PLATFORM STREQUAL "macos")
    set(QT_PREFIX_PATH "clang_64")

# -- IOS
elseif(VS_PLATFORM STREQUAL "ios")
    set(QT_PREFIX_PATH "ios")
    set(CMAKE_SYSTEM_NAME "iOS")
    set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
    set(CMAKE_IOS_INSTALL_COMBINED YES)

# -- Windows
elseif(VS_PLATFORM STREQUAL "windows")
    set(QT_PREFIX_PATH "mingw32")
endif()

# ---------------------------------------------------------------------------
# Set CMAKE_FIND_ROOT_PATH for QT cmake include directory
# ---------------------------------------------------------------------------
if (NOT CMAKE_FIND_ROOT_PATH AND NOT CMAKE_PREFIX_PATH AND DEFINED ENV{QTDIR})
    get_filename_component(QT_RELATIVE_PATH "$ENV{QTDIR}/../" ABSOLUTE)
    list(APPEND CMAKE_FIND_ROOT_PATH "${QT_RELATIVE_PATH}/${QT_PREFIX_PATH}")
    list(APPEND CMAKE_PREFIX_PATH "${QT_RELATIVE_PATH}/${QT_PREFIX_PATH}")
    message("-- Set CMAKE_FIND_ROOT_PATH from QTDIR enviroment value: [ ${CMAKE_FIND_ROOT_PATH} ]")
endif()

# ---------------------------------------------------------------------------
# Set build toolchain
# ---------------------------------------------------------------------------
if(VS_CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${VS_CMAKE_TOOLCHAIN_FILE}")
    message("-- Set CMAKE_TOOLCHAIN_FILE from ANDROID_NDK enviroment value: [ ${CMAKE_TOOLCHAIN_FILE} ]")
endif()	
