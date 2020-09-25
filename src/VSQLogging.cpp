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

#include <VSQLogging.h>

#include <virgil/iot/logger/logger.h>

VSQLogging *VSQLogging::m_instance = nullptr;

VSQLogging::VSQLogging() : QObject()
{
    if (m_instance) {
        qFatal("Instance of logging already exists!");
    }
    else {
        m_instance = this;
    }

    vs_logger_init(VirgilIoTKit::VS_LOGLEV_DEBUG);
    qInstallMessageHandler(&VSQLogging::messageHandler);
}

VSQLogging::~VSQLogging()
{
    m_instance = nullptr;
}

void VSQLogging::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    consoleMessageHandler(type, context, message);
#ifdef QT_NO_DEBUG // release
    fileMessageHandler(type, context, message);
#endif
#ifdef VS_DEVMODE
    signalMessageHandler(type, context, msg);
#endif
}

void VSQLogging::signalMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Q_UNUSED(type)
    emit VSQLogging::m_instance->newMessage(QString("[%1] %2").arg(context.category, message));
}

void VSQLogging::fileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const QByteArray localMsg = message.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_DEBUG), context.file, context.line,localMsg.constData());
        break;
    case QtInfoMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_INFO), context.file, context.line,localMsg.constData());
        break;
    case QtWarningMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_WARNING), context.file, context.line,localMsg.constData());
        break;
    case QtCriticalMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_CRITICAL), context.file, context.line,localMsg.constData());
        break;
    case QtFatalMsg:
        vs_logger_message(VS_LOG_MAKE_LEVEL(VS_LOGLEV_FATAL), context.file, context.line,localMsg.constData());
        abort();
    }
}

void VSQLogging::consoleMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Q_UNUSED(type)
    fprintf(stderr, "[%s] %s\n", context.category, qPrintable(message));
}
