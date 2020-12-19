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

#ifndef VS_FILELOADER_H
#define VS_FILELOADER_H

#include "CoreMessenger.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(lcFileLoader);


namespace vm
{

class FileLoader : public QObject
{
    Q_OBJECT

public:
    using ConnectionSetup = std::function<void (QNetworkReply *)>;

    FileLoader(CoreMessenger *client, QObject *parent);

    bool isServiceFound() const;

signals:
    void uploadServiceFound(const bool found);
    void uploadSlotReceived(const QString &slotId, const QUrl &putUrl, const QUrl &getUrl);
    void uploadSlotErrorOccurred(const QString &slotId, const QString &errorText);
    void requestUploadSlotFailed(const QString &filePath);
    void ready();

    void fireStartDownload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);
    void fireStartUpload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);
    void fireRequestUploadSlot(const QString &filePath);


private:
    void onServiceFound(bool found);
    void onStartDownload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);
    void onStartUpload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);
    void onRequestUploadSlot(const QString &filePath);

    QPointer<CoreMessenger> m_coreMessenger;
    QPointer<QNetworkAccessManager> m_networkAccessManager;
};
}

Q_DECLARE_METATYPE(vm::FileLoader::ConnectionSetup);

#endif // VS_FILELOADER_H
