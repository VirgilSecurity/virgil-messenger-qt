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

#include "VSQCrashReporter.h"

#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#include "Settings.h"

#include <virgil/iot/messenger/internal/virgil.h>

const QString VSQCrashReporter::s_endpointSendReport = "/send-logs";

using namespace vm;

Q_LOGGING_CATEGORY(lcCrashReporter, "crashReporter")

VSQCrashReporter::VSQCrashReporter(Settings *settings, QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_manager(networkAccessManager)
{}

VSQCrashReporter::~VSQCrashReporter()
{
    m_settings->setRunFlag(false);
}

void VSQCrashReporter::setVirgilUrl(QString VirgilUrl)
{
    m_currentVirgilUrl = VirgilUrl;
    qCDebug(lcCrashReporter) << "Send report URL set to" << m_currentVirgilUrl;
}

void VSQCrashReporter::setkVersion(QString AppVersion)
{
    m_version = AppVersion;
}

void VSQCrashReporter::setkOrganization(QString strkOrganization)
{
    m_organization = strkOrganization;
}

void VSQCrashReporter::setkApp(QString strkApp)
{
    m_app = strkApp;
}

void VSQCrashReporter::checkAppCrash()
{
    qCDebug(lcCrashReporter) << "Checking previous run flag...";
    if(m_settings->runFlag()) {
#ifndef VS_DEVMODE
        qCCritical(lcCrashReporter) << "Previous application run is crashed ! Sending log files...";
        emit crashReportRequested();
#endif
    }
    qCDebug(lcCrashReporter) << "Set run flag to true";
    m_settings->setRunFlag(true);
}

bool VSQCrashReporter::sendLogFiles()
{
    const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!writeDir.exists()) {
        qFatal("Directory [%s] not exist.", qPrintable(writeDir.absolutePath()));
        return false;
    }

    qCDebug(lcCrashReporter) << "Collecting of logs:" << writeDir.absolutePath();
    QByteArray fileData;
    QDirIterator fileIterator(writeDir.absolutePath(), QStringList() << "VirgilMessenger.log*");
    while (fileIterator.hasNext()) {
        QFile readFile(fileIterator.next());
        if ( readFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
            qCDebug(lcCrashReporter) << "Read file:" << readFile.fileName();
            fileData.append(readFile.readAll());
        }
        else {
            qCDebug(lcCrashReporter) << "Can't open " << readFile.fileName() << readFile.errorString();
        }
    }

    sendFileToBackendRequest(fileData);
    return true;
}

bool VSQCrashReporter::sendFileToBackendRequest(QByteArray fileData)
{
    char *buffBearer = (char *)malloc(1024);
    size_t sizeBearer = 1024;

    VirgilIoTKit::vs_messenger_virgil_get_auth_token(buffBearer, sizeBearer);
    qCDebug(lcCrashReporter) << "Backend token:" << buffBearer;

    QString strEndpoint = m_currentVirgilUrl + s_endpointSendReport;
    QNetworkRequest req(strEndpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    req.setRawHeader(QString("Authorization").toUtf8(), buffBearer);
    req.setRawHeader(QString("Version").toUtf8(), qPrintable(m_version));
    req.setRawHeader(QString("Platform").toUtf8(), qPrintable(QSysInfo::kernelType()));
    auto reply = m_manager->post(req, fileData);
    qCDebug(lcCrashReporter) << "Send report to endpoint:" << strEndpoint;
    for (auto name : req.rawHeaderList()) {
        qCDebug(lcCrashReporter) << "Request header" << name << ':' << req.rawHeader(name);
    }
    qCDebug(lcCrashReporter) << "File data empty:" << fileData.isEmpty();
    connect(reply, &QNetworkReply::finished, this, std::bind(&VSQCrashReporter::endpointReply, this, reply));

    return true;
}

void VSQCrashReporter::endpointReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        qCDebug(lcCrashReporter) << "Send report OK";
        emit reportSent(tr("Report send OK"));
    }
    else {
        qCDebug(lcCrashReporter) << "Error sending report. Code:" << static_cast<int>(reply->error())
                                 << ". Name:" << reply->error()
                                 << ". Message:" << reply->errorString();
        emit reportErrorOccurred(tr("Report send error"));
    }
    qCDebug(lcCrashReporter) << "Sending finished";
}
