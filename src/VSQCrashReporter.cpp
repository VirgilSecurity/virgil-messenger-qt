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

#include <QCoreApplication>
#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#include <virgil/iot/messenger/messenger.h>

#include "VSQSettings.h"

Q_LOGGING_CATEGORY(lcCrashReporter, "crashreport")

using namespace VirgilIoTKit;

VSQCrashReporter::VSQCrashReporter(VSQSettings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_manager(nullptr)
{
    connect(this, &VSQCrashReporter::send, this, &VSQCrashReporter::sendLogFiles);
}

VSQCrashReporter::~VSQCrashReporter()
{
    m_settings->setRunFlag(false);
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~CrashReporter";
#endif
}

void VSQCrashReporter::check()
{
    qCDebug(lcCrashReporter) << "Checking previous run flag...";
    if (m_settings->runFlag()) {
        qCWarning(lcCrashReporter) << "Previous application run is crashed!";
        emit requested();
    }
    emit checked();
}

bool VSQCrashReporter::sendLogFiles()
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
            qCDebug(lcCrashReporter) << "Read file:" << readFile.fileName() << endl;
            fileData.append(readFile.readAll());
        }
        else {
            qCDebug(lcCrashReporter) << "Can't open " << readFile.fileName() << readFile.errorString() << endl;
        }
    }

    return sendFileToBackendRequest(fileData);
}

void VSQCrashReporter::setUrl(const QString &url)
{
    if (m_url == url)
        return;
    m_url = url;
    qCDebug(lcCrashReporter) << "Crash reporter url changed to:" << url;
}

bool VSQCrashReporter::sendFileToBackendRequest(const QByteArray &fileData)
{
    char *buffBearer = (char *) malloc(1024);
    size_t sizeBearer = 1024;
    vs_messenger_virgil_get_auth_token(buffBearer, sizeBearer);
    qCDebug(lcCrashReporter) << "Backend token:" << buffBearer;

    if (!m_manager)
        m_manager = new QNetworkAccessManager(this);

    QNetworkRequest request;
    QUrl strEndpoint(m_url + QLatin1String("/send-logs"));
    request.setUrl(strEndpoint);
    qCDebug(lcCrashReporter) << "Send report to endpoint:" << strEndpoint;
    request.setHeader(QNetworkRequest::ContentTypeHeader,  QString("application/json"));
    request.setRawHeader("Authorization", buffBearer);
    request.setRawHeader("Version", QCoreApplication::applicationVersion().toUtf8());
    request.setRawHeader("Platform", QSysInfo::kernelType().toUtf8());
    QNetworkReply* reply = m_manager->post(request, fileData);
    connect(reply, &QNetworkReply::finished, this, &VSQCrashReporter::endpointReply);

    return true;
}

void VSQCrashReporter::endpointReply()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError)
        qCDebug(lcCrashReporter) << "Send report OK";
    else
        qCDebug(lcCrashReporter) << "Error sending report:" << reply->errorString();
    reply->deleteLater();
    qCDebug(lcCrashReporter) << "Sending finished";
}
