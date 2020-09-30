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

#include "logging/VSQLogWorker.h"

#include <virgil/iot/logger/logger.h>

VSQLogWorker::VSQLogWorker(QObject *parent)
    : QObject(parent)
{
#ifdef QT_NO_DEBUG // release
    m_logToFile = true;
#endif
}

VSQLogWorker::~VSQLogWorker()
{
}

void VSQLogWorker::start()
{
    if (m_logToFile) {
        vs_logger_init(VirgilIoTKit::VS_LOGLEV_DEBUG);
    }
}

void VSQLogWorker::processMessage(QtMsgType type, const VSQMessageLogContext &context, const QString &message)
{
    consoleMessageHandler(type, context, message);
    if (m_logToFile) {
        fileMessageHandler(type, context, message);
    }
}

void VSQLogWorker::fileMessageHandler(QtMsgType type, const VSQMessageLogContext &context, const QString &message)
{
    const QByteArray localMsg = message.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_DEBUG), qPrintable(context.fileName), context.line, qPrintable(localMsg));
        break;
    case QtInfoMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_INFO), qPrintable(context.fileName), context.line, qPrintable(localMsg));
        break;
    case QtWarningMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_WARNING), qPrintable(context.fileName), context.line, qPrintable(localMsg));
        break;
    case QtCriticalMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_CRITICAL), qPrintable(context.fileName), context.line, qPrintable(localMsg));
        break;
    case QtFatalMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_FATAL), qPrintable(context.fileName), context.line, qPrintable(localMsg));
        abort();
    }
}

void VSQLogWorker::consoleMessageHandler(QtMsgType type, const VSQMessageLogContext &context, const QString &message)
{
    Q_UNUSED(type)
    fprintf(stderr, "[%s] %s\n", qPrintable(context.category), qPrintable(message));
}
