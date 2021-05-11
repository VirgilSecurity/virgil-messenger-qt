//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#ifndef VM_LOG_WORKER
#define VM_LOG_WORKER

#include "LogContext.h"

#include <QObject>
#include <QFile>
#include <QDir>

#include <optional>

namespace vm {
class LogWorker : public QObject
{
    Q_OBJECT

public:
    explicit LogWorker(QObject *parent = nullptr);
    ~LogWorker() override = default;

    void processMessage(QtMsgType type, const LogContext &context, const QString &message);

private:
    static QString formatLogType(QtMsgType type);
    static QString getLogFileName(int logIndex);

    void fileMessageHandler(QtMsgType type, const LogContext &context, const QString &message);
    void consoleMessageHandler(QtMsgType type, const LogContext &context, const QString &message);

    //
    //  Prepare log file for writing of message with size messageLen
    //  Return false if message can't be written
    //
    bool prepareLogFile(qint64 messageLen);

    //
    //  Rotate log files
    //
    void rotateLogFiles();

    void logToFile(const QString &formattedMessage);
    void logToConsole(const QString &formattedMessage);

private:
    QFile m_logFile; // automatically closed in destructor
    bool m_isFirstMessage = true;
};
} // namespace vm

#endif // VM_LOG_WORKER
