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

#include "CrashReporter.h"

#include "Settings.h"
#include "CustomerEnv.h"
#include "LogConfig.h"

#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

using namespace vm;
using Self = CrashReporter;

Q_LOGGING_CATEGORY(lcCrashReporter, "crash-reporter")

Self::CrashReporter(Settings *settings, vm::CoreMessenger *commKitMessenger, QObject *parent)
    : QObject(parent),
      m_settings(settings),
      m_coreMessenger(commKitMessenger),
      m_networkManager(new QNetworkAccessManager(this))
{
}

void Self::checkAppCrash()
{
    qCDebug(lcCrashReporter) << "Checking previous run flag...";
    if (m_settings->runFlag()) {
#ifndef QT_DEBUG
        qCCritical(lcCrashReporter) << "Previous application run is crashed! Sending log files...";
        if (m_settings->autoSendCrashReport()) {
            sendLogFiles("crash-logs auto-send");
        } else {
            emit crashReportRequested();
        }
#endif
    }
    qCDebug(lcCrashReporter) << "Set run flag to true";
    m_settings->setRunFlag(true);
}

bool Self::sendLogFiles(const QString &details)
{
    if (!details.isEmpty()) {
        qCInfo(lcCrashReporter) << "----->" << details << "<-----";
    }
    qCDebug(lcCrashReporter) << "Collecting of logs...";

    const auto logsDir = LogConfig::instance().logsDir();
    if (!logsDir.exists()) {
        qWarning("Directory [%s] not exist.", qPrintable(logsDir.absolutePath()));
        return false;
    }

    qCDebug(lcCrashReporter) << "Lookup logs within directory: " << logsDir.absolutePath();

    QByteArray fileData;
    const auto fileInfos =
            logsDir.entryInfoList(QStringList() << "VirgilMessenger*.log", QDir::NoFilter, QDir::Name | QDir::Reversed);
    for (auto &fileInfo : fileInfos) {
        QFile readFile(fileInfo.filePath());
        if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCDebug(lcCrashReporter) << "Read file:" << readFile.fileName();
            fileData.append(readFile.readAll());
        } else {
            qCDebug(lcCrashReporter) << "Can't open " << readFile.fileName() << readFile.errorString();
        }
    }

    auto endpointUrl = m_coreMessenger->getCrashReportEndpointUrl();
    auto authHeaderValue = m_coreMessenger->getAuthHeaderVaue();

    qCDebug(lcCrashReporter) << "Send crash report to the endpoint: " << endpointUrl << "size:" << fileData.size();
    qCDebug(lcCrashReporter) << "Messenger Backend auth header value: " << authHeaderValue;

    QNetworkRequest request(endpointUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    request.setRawHeader(QString("Authorization").toUtf8(), authHeaderValue.toUtf8());
    request.setRawHeader(QString("Virgil-Agent").toUtf8(),
                         qPrintable(CustomerEnv::version() + ";" + QSysInfo::kernelType()));
    auto reply = m_networkManager->post(request, fileData);
    connect(reply, &QNetworkReply::finished, this, std::bind(&CrashReporter::sendSendCrashReportReply, this, reply));
    for (auto name : request.rawHeaderList()) {
        qCDebug(lcCrashReporter) << "Request header" << name << ':' << request.rawHeader(name);
    }
    qCDebug(lcCrashReporter) << "Crash log is empty: " << fileData.isEmpty();

    return true;
}

void Self::sendSendCrashReportReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        qCDebug(lcCrashReporter) << "Send report OK";
        emit reportSent(tr("Crash report sent"));
    } else {
        qCDebug(lcCrashReporter) << "Error sending report:";
        qCDebug(lcCrashReporter) << "    -> Code:" << static_cast<int>(reply->error());
        qCDebug(lcCrashReporter) << "    -> Name:" << reply->error();
        qCDebug(lcCrashReporter) << "    -> Message:" << reply->errorString();
        qCDebug(lcCrashReporter) << "    -> Error body:" << reply->readAll();
        emit reportErrorOccurred(tr("Crash report failed to send"));
    }
    qCDebug(lcCrashReporter) << "Sending finished";
    reply->deleteLater();
}
