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

#include "logging/VSQLogging.h"

#include <QThread>

#include <virgil/iot/logger/logger.h>

#include "logging/VSQLogWorker.h"

VSQLogging *VSQLogging::m_instance = nullptr;

VSQLogging::VSQLogging(QObject *parent)
    : QObject(parent)
{
    // Set instance
    if (m_instance) {
        qFatal("Instance of logger worker already exists!");
    }
    else {
        m_instance = this;
    }

    // Register
    qRegisterMetaType<QtMsgType>();
    qRegisterMetaType<VSQMessageLogContext>();

    // Internal connections
    connect(this, &VSQLogging::messageCreated, this, &VSQLogging::formatMessage);

    // Setup thread, worker and connections
    m_workerThread = std::make_unique<QThread>();
    auto worker = new VSQLogWorker();
    worker->moveToThread(m_workerThread.get());
    connect(this, &VSQLogging::messageCreated, worker, &VSQLogWorker::processMessage);
    connect(m_workerThread.get(), &QThread::started, worker, &VSQLogWorker::start);
    connect(m_workerThread.get(), &QThread::finished, worker, &VSQLogWorker::deleteLater);
    m_workerThread->start();

    // Install handler
    qInstallMessageHandler(&VSQLogging::messageHandler);
}

VSQLogging::~VSQLogging()
{
    // Unset handler
    qInstallMessageHandler(0);

    // Cleanup thread
    m_workerThread->quit();
    m_workerThread->wait();

    // Unset instance
    m_instance = nullptr;
}

void VSQLogging::formatMessage(QtMsgType type, const VSQMessageLogContext &context, const QString &message)
{
    QChar preffix;
    QString color;
    switch (type) {
    case QtDebugMsg:
        preffix = QLatin1Char('D');
        break;
    case QtInfoMsg:
        preffix = QLatin1Char('I');
        color = QLatin1String("#00aa00");
        break;
    case QtWarningMsg:
        preffix = QLatin1Char('W');
        color = QLatin1String("#ff8800");
        break;
    case QtCriticalMsg:
        preffix = QLatin1Char('C');
        color = QLatin1String("#aa0000");
        break;
    case QtFatalMsg:
        preffix = QLatin1Char('F');
        color = QLatin1String("#aa0000");
        break;
    }
    auto formattedMessage = QString("%1: [%2] %3").arg(preffix, context.category, message);
    if (!color.isEmpty()) {
        formattedMessage = QString("<span style='color:%1;'>%2</span>").arg(color, formattedMessage.toHtmlEscaped());
    }
    emit formattedMessageCreated(formattedMessage + QLatin1String("<br/>"));
}

void VSQLogging::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    VSQMessageLogContext logContext { context.category, context.file, context.line };
    emit m_instance->messageCreated(type, logContext, message);
}
