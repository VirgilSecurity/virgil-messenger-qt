#
# Copyright (C) 2016 Virgil Security Inc.
#
# Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     (1) Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#     (2) Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#     (3) Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from
#     this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

project (virgil_test)

# Configure path to custom modules
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Define enviroment parameters
include (check_pointer_size)
check_pointer_size (POINTER_SIZE)
if (POINTER_SIZE)
    message (STATUS "Compiler pointer size: " ${POINTER_SIZE} " bytes")
else ()
    message (STATUS "Compiler pointer size: UNDEFINED")
endif ()

set(VIRGIL_PATCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patches")

# Enable C++11
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Inspect system
set (SYSTEM_ARCH x86)
if (POINTER_SIZE EQUAL 8)
    set (SYSTEM_ARCH x64)
endif ()
string (TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME)

# Find dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package (CURL REQUIRED)
include_directories (${CURL_INCLUDE_DIRS})

include (virgil)
virgil_add_dependency ("crypto"
    VIRGIL_CRYPTO_DEPENDENCIES
    VIRGIL_CRYPTO_INCLUDE_DIRS
    VIRGIL_CRYPTO_LIBRARIES
)
include_directories (${VIRGIL_CRYPTO_INCLUDE_DIRS})

virgil_add_dependency ("virgil-sdk"
    VIRGIL_SDK_DEPENDENCIES
    VIRGIL_SDK_INCLUDE_DIRS
    VIRGIL_SDK_LIBRARIES
)
include_directories (${VIRGIL_SDK_INCLUDE_DIRS})

include (json)
include_directories (${JSON_INCLUDE_DIRS})

# Use tclap lib
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/ext")
message (STATUS "TCLAP library: ON")

# Pass toolchain to subprojects


list (APPEND CMAKE_ARGS
	-DVIRGIL_PATCH_DIR:PATH=${VIRGIL_PATCH_DIR}
)

if (CMAKE_TOOLCHAIN_FILE)
	list (APPEND CMAKE_ARGS
			-DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
	)
endif (CMAKE_TOOLCHAIN_FILE)

# Client part
add_executable (virgil_client 
				"src/client/main.cpp"
				"src/client/VirgilClient.cpp"
				"src/VirgilCryptoHelper.cpp")
target_link_libraries (virgil_client
    ${VIRGIL_SDK_LIBRARIES}
    ${VIRGIL_CRYPTO_LIBRARIES}
    ${CURL_LIBRARIES}
)

add_dependencies (virgil_client
    json
    ${VIRGIL_CRYPTO_DEPENDENCIES}
    ${VIRGIL_SDK_DEPENDENCIES}
)

# Server part
add_executable (virgil_server
		 		"src/server/main.cpp"
				"src/server/VirgilServer.cpp"
				"src/VirgilCryptoHelper.cpp")
target_link_libraries (virgil_server
    ${VIRGIL_SDK_LIBRARIES}
    ${VIRGIL_CRYPTO_LIBRARIES}
    ${CURL_LIBRARIES}
    Threads::Threads
)

add_dependencies (virgil_server
    json
    ${VIRGIL_CRYPTO_DEPENDENCIES}
    ${VIRGIL_SDK_DEPENDENCIES}
    Threads::Threads
)
