//  Copyright (C) 2015-2019 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#ifndef _VIRGIL_IOTKIT_QT_LOGGER_H
#define _VIRGIL_IOTKIT_QT_LOGGER_H

#include <sstream>
#include <virgil/iot/logger/logger.h>

template <typename... T>
extern inline void log(VirgilIoTKit::vs_log_level_t log_lev, const char *cur_filename, uint32_t line_num, T... args){
    std::stringstream sstr;
    ( sstr << ... << std::forward<T>(args) );

    VirgilIoTKit::vs_logger_message( log_lev, cur_filename, line_num, sstr.str().c_str() );
}

#define VSLogInfo(FRMT, ...)  log(VirgilIoTKit::VS_LOGLEV_INFO, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogFatal(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_FATAL, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogAlert(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_ALERT, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogCritical(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_CRITICAL, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogError(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_ERROR, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogWarning(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_WARNING, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogNotice(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_NOTICE, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogTrace(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_TRACE, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VSLogDebug(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_DEBUG, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)

#endif //_VIRGIL_IOTKIT_QT_LOGGER_H
