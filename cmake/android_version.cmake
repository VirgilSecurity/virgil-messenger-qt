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

function(PAD_STRING OUT_VARIABLE DESIRED_LENGTH FILL_CHAR VALUE)
    string(LENGTH "${VALUE}" VALUE_LENGTH)
    math(EXPR REQUIRED_PADS "${DESIRED_LENGTH} - ${VALUE_LENGTH}")
    set(PAD ${VALUE})
    if (REQUIRED_PADS GREATER 0)
        math(EXPR REQUIRED_MINUS_ONE "${REQUIRED_PADS} - 1")
        foreach (FOO RANGE ${REQUIRED_MINUS_ONE})
            SET(PAD "${FILL_CHAR}${PAD}")
        endforeach()
    endif()
    set(${OUT_VARIABLE} "${PAD}" PARENT_SCOPE)
endfunction()


function(SET_ANDROID_VERSION ANDROID_VERSION_NAME ANDROID_VERSION_CODE TARGET_VERSION)
    # Set Android version name
    set(${ANDROID_VERSION_NAME} "${TARGET_VERSION}" PARENT_SCOPE)

    # Fill Android version code
    string(REPLACE "." ";" VS_VERSION_LIST ${VS_TARGET_VERSION})

    list(GET VS_VERSION_LIST 0 TMP_VERS)
    pad_string(CODE 3 "0" "1${TMP_VERS}")

    list(GET VS_VERSION_LIST 1 TMP_VERS)
    pad_string(TMP_VERS 2 "0" "${TMP_VERS}")
    string(APPEND CODE "${TMP_VERS}")

    list(GET VS_VERSION_LIST 2 TMP_VERS)
    pad_string(TMP_VERS 2 "0" "${TMP_VERS}")
    string(APPEND CODE "${TMP_VERS}")

    # Get build number (or zero)
    if (DEFINED ENV{BUILD_NUMBER})
        set(VS_BUILD_NUMBER "$ENV{BUILD_NUMBER}")
    else ()
        set(VS_BUILD_NUMBER "0")
    endif ()
    pad_string(TMP_VERS 3 "0" "${VS_BUILD_NUMBER}")
    string(APPEND CODE "${TMP_VERS}")

    set(${ANDROID_VERSION_CODE} "${CODE}" PARENT_SCOPE)
endfunction()
