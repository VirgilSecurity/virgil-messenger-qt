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

#ifndef VM_CRASH_REPORTER_H
#define VM_CRASH_REPORTER_H

#include <QObject>

#include "CoreMessenger.h"

class QNetworkAccessManager;
class QNetworkReply;

class Settings;

Q_DECLARE_LOGGING_CATEGORY(lcCrashReporter)

namespace vm {
class CrashReporter : public QObject
{
    Q_OBJECT

public:
    CrashReporter(Settings *settings, vm::CoreMessenger *commKitMessenger, QObject *parent = nullptr);

    virtual ~CrashReporter() noexcept = default;

    void checkAppCrash();
    Q_INVOKABLE bool sendLogFiles(const QString &details);

signals:
    void crashReportRequested();
    void reportSent(const QString &msg);
    void reportErrorOccurred(const QString &msg);

private:
    bool sendFileToBackendRequest(QByteArray fileData);
    void sendSendCrashReportReply(QNetworkReply *reply);

    Settings *m_settings;
    vm::CoreMessenger *m_coreMessenger;
    QNetworkAccessManager *m_networkManager;

    static const QString s_endpointSendReport;
};
} // namespace vm

#endif // VM_CRASH_REPORTER_H
