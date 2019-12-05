//
// Created by Oleksandr Nemchenko on 26.11.2019.
//

#ifndef VIRGIL_IOTKIT_CPP_LOGGER_H
#define VIRGIL_IOTKIT_CPP_LOGGER_H

#include <app.h>

#include <sstream>
#include <virgil/iot/logger/logger.h>

#undef VS_LOG_INFO
#undef VS_LOG_FATAL
#undef VS_LOG_ALERT
#undef VS_LOG_CRITICAL
#undef VS_LOG_ERROR
#undef VS_LOG_WARNING
#undef VS_LOG_NOTICE
#undef VS_LOG_TRACE
#undef VS_LOG_DEBUG

template <typename... T>
extern inline void log(VirgilIoTKit::vs_log_level_t log_lev, const char *cur_filename, uint32_t line_num, T... args){
    std::stringstream sstr;
    ( sstr << ... << std::forward<T>(args) );

    VirgilIoTKit::vs_logger_message( log_lev, cur_filename, line_num, sstr.str().c_str() );
}

#define VS_LOG_INFO(FRMT, ...)  log(VirgilIoTKit::VS_LOGLEV_INFO, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_FATAL(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_FATAL, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_ALERT(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_ALERT, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_CRITICAL(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_CRITICAL, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_ERROR(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_ERROR, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_WARNING(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_WARNING, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_NOTICE(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_NOTICE, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_TRACE(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_TRACE, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)
#define VS_LOG_DEBUG(FRMT, ...) log(VirgilIoTKit::VS_LOGLEV_DEBUG, __FILENAME__, __LINE__, (FRMT), ##__VA_ARGS__)

#endif //VIRGIL_IOTKIT_CPP_LOGGER_H
