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

#include "VSQLogging.h"

#include <QCoreApplication>

#include <virgil/iot/qt/VSQIoTKit.h>
#include <virgil/iot/logger/logger.h>
#include <virgil/iot/messenger/messenger.h>
#include <virgil/iot/qt/VSQIoTKit.h>

#include "VSQSettings.h"

Q_LOGGING_CATEGORY(lcCrashReport, "crashreport")

using namespace VirgilIoTKit;

VSQLogging::VSQLogging(VSQSettings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
    connect(this, &VSQLogging::sendCrashReport, this, &VSQLogging::sendLogFiles);
}

VSQLogging::~VSQLogging()
{
}

void VSQLogging::initialize()
{
    qInstallMessageHandler(&VSQLogging::handler);
}

void VSQLogging::checkCrashReport()
{
    qCDebug(lcCrashReport) << "Checking previous run flag...";
    if (m_settings->runFlag()) {
        qCritical(lcCrashReport) << "Previous application run is crashed! Sending log files...";
        emit crashReportFound();
    }
    emit crashReportChecked();
}

bool VSQLogging::sendLogFiles()
{
    QByteArray fileData;
    const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!writeDir.exists()) {
        qFatal("Directory [%s] doesn't exist.", qPrintable(writeDir.absolutePath()));
        return false;
    }

    QDirIterator fileIterator(writeDir.absolutePath(), QStringList() << "virgil-messenger.log*");
    while (fileIterator.hasNext()) {
        QFile readFile(fileIterator.next());
        if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCDebug(lcCrashReport) << "Read file:" << readFile.fileName() << endl;
            fileData.append(readFile.readAll());
        }
        else {
            qCDebug(lcCrashReport) << "Can't open " << readFile.fileName() << readFile.errorString() << endl;
        }
    }

    return sendFileToBackendRequest(fileData);
}

void VSQLogging::setVirgilUrl(const QString &url)
{
    m_virgilUrl = url;
    qCDebug(lcCrashReport) << "Virgil url changed to:" << url;
}

bool VSQLogging::sendFileToBackendRequest(const QByteArray &fileData)
{
    char *buffBearer = (char *) malloc(1024);
    size_t sizeBearer = 1024;
    vs_messenger_virgil_get_auth_token(buffBearer, sizeBearer);
    qCDebug(lcCrashReport) << "Backend token:" << buffBearer;

    if (!m_manager)
        m_manager = new QNetworkAccessManager(this);

    QNetworkRequest request;
    QUrl strEndpoint(m_virgilUrl + QLatin1String("/send-logs"));
    request.setUrl(strEndpoint);
    qCDebug(lcCrashReport) << "Send report to endpoint:" << strEndpoint;
    request.setHeader(QNetworkRequest::ContentTypeHeader,  QString("application/json"));
    request.setRawHeader("Authorization", buffBearer);
    request.setRawHeader("Version", QCoreApplication::applicationVersion().toUtf8());
    request.setRawHeader("Platform", QSysInfo::kernelType().toUtf8());
    QNetworkReply* reply = m_manager->post(request, fileData);
    connect(reply, &QNetworkReply::finished, this, &VSQLogging::endpointReply);

    return true;
}

void VSQLogging::endpointReply()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError)
        qCDebug(lcCrashReport) << "Send report OK";
    else
        qCDebug(lcCrashReport) << "Error sending report:" << reply->errorString();
    reply->deleteLater();
    qCDebug(lcCrashReport) << "Sending finished";
}

void VSQLogging::handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        vs_logger_message(VS_LOGLEV_DEBUG, context.file, context.line, localMsg.constData());
        break;
    case QtInfoMsg:
        vs_logger_message(VS_LOGLEV_INFO, context.file, context.line, localMsg.constData());
        break;
    case QtWarningMsg:
        vs_logger_message(VS_LOGLEV_WARNING, context.file, context.line, localMsg.constData());
        break;
    case QtCriticalMsg:
        vs_logger_message(VS_LOGLEV_CRITICAL, context.file, context.line, localMsg.constData());
        break;
    case QtFatalMsg:
        vs_logger_message(VS_LOGLEV_FATAL, context.file, context.line, localMsg.constData());
        abort();
    }
}
