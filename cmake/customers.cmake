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

function(vs_customer_prepare)
    message(STATUS "Prepare confuiguration from templates...")
    foreach(VS_TMP IN LISTS VS_TEMPLATES)
        string(REPLACE "=" ";" VS_TMP_LIST ${VS_TMP})
        list(GET VS_TMP_LIST 0 VS_TEMPLATE_SRC)
        list(GET VS_TMP_LIST 1 VS_TEMPLATE_DST)    
        if(NOT EXISTS "${VS_TEMPLATE_SRC}")
        	message(FATAL_ERROR "Template [${VS_TEMPLATE_SRC}] not found !")
        endif()
        message(STATUS "Processing template file: [${VS_TEMPLATE_SRC}]")
        configure_file("${VS_TEMPLATE_SRC}" "${VS_TEMPLATE_DST}" @ONLY)
    endforeach()
    
    message(STATUS "Copy customer files...")
    foreach(VS_TMP IN LISTS VS_FILES)
        string(REPLACE "=" ";" VS_TMP_LIST ${VS_TMP})
        list(GET VS_TMP_LIST 0 VS_FILE_SRC)
        list(GET VS_TMP_LIST 1 VS_FILE_DST)    
        if(NOT EXISTS "${VS_FILE_SRC}")
        	message(FATAL_ERROR "File [${VS_FILE_SRC}] not found !")
        endif()
        message(STATUS "[${VS_FILE_SRC}] => [${VS_FILE_DST}]")
        configure_file("${VS_FILE_SRC}" "${VS_FILE_DST}" COPYONLY)
    endforeach()    
endfunction()