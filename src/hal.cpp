//  Copyright (C) 2015-2020 Virgil Security, Inc.
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

#include <iostream>
#include <string>
#include <QStandardPaths>
#include <qdir.h>
#include <QCoreApplication>
#include <QFileInfo>

#include <stdio.h>
#include <time.h>
#ifndef LOG_ROTATE_LEVEL
#define LOG_ROTATE_LEVEL 0     // Log rotate num (num -1)
#endif
#ifndef LOG_MAX_FILESIZE
#define LOG_MAX_FILESIZE 20 * 1024 * 1024
#endif

QFile vsLogFile;
bool vsLogOpeningFailed = false;

bool vs_logger_rotate(QString FileName, int LogNums) {
    QString NewFilePath = FileName + "." + QString::number(LogNums + 1);
    QFile HLogFile;
    HLogFile.remove(NewFilePath);
    for(int TmpLogNum = LogNums; TmpLogNum >= 0; TmpLogNum--) {
        if(TmpLogNum > 0 ) {
            HLogFile.setFileName(FileName + "." + QString::number(TmpLogNum));
        }
        else {
            HLogFile.setFileName(FileName);
        }
        NewFilePath = FileName + "." + QString::number(TmpLogNum + 1);
        if(HLogFile.exists()) {
            if(!HLogFile.rename(NewFilePath)) {
                return false;
            }
        }
    }
    return true;
}

bool vs_logger_rotate_active_file() {
    vsLogFile.close();
    vs_logger_rotate(vsLogFile.fileName(), LOG_ROTATE_LEVEL);
    return vsLogFile.open(QIODevice::WriteOnly | QIODevice::Text);
}

bool vs_logger_file_opened() {
    if (vsLogOpeningFailed) {
        return false;
    }

    if (!vsLogFile.isOpen()) {
        // First run
        if (vsLogFile.fileName().isEmpty()) {
            const QDir appDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            if (!appDir.mkpath(".")) {
                vsLogOpeningFailed = true;
                return false;
            }
            vsLogFile.setFileName(appDir.absolutePath() + '/' + QCoreApplication::applicationName() + ".log");
        }
        // Rotation
        if (!vs_logger_rotate_active_file()) {
            vsLogOpeningFailed = true;
            return false;
        }
    }

    return true;
}

extern "C" bool
vs_logger_output_hal(const char *buffer) {
    (void)buffer;
    if(vs_logger_file_opened()) {
        vsLogFile.write(buffer,strlen(buffer));
        vsLogFile.flush();
        const bool isEOL = buffer[0] != 0 && strlen(buffer) && buffer[strlen(buffer) - 1] == '\n';
        if (isEOL && vsLogFile.size() >= LOG_MAX_FILESIZE && !vs_logger_rotate_active_file()) {
            vsLogOpeningFailed = true;
        }
    } else {
        std::cout << buffer << std::flush;
    }
    return true;
}

extern "C" bool
vs_logger_current_time_hal(char *time_buf) {
    time_t result = time(NULL);
    if(result != -1) {
        strftime(time_buf, 17, "%Y-%m-%d %H:%M", localtime(&result));
        return true;
    }
    return false;
}


extern "C" void
vs_impl_msleep(size_t msec) {
    (void)msec;
}
