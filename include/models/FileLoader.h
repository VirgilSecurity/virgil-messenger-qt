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

#include <QXmppHttpUploadIq.h>

#include "VSQCommon.h"

class QNetworkAccessManager;
class QNetworkReply;

class QXmppClient;
class QXmppUploadRequestManager;

Q_DECLARE_LOGGING_CATEGORY(lcFileLoader)

namespace vm
{
class FileLoader : public QObject
{
    Q_OBJECT

public:
    using ConnectionSetup = std::function<void (QNetworkReply *)>;

    FileLoader(QXmppClient *client, QNetworkAccessManager *networkAccessManager, QObject *parent);

    // TODO(fpohtmeh): redesign as signal
    QString requestUploadUrl(const QString &filePath);

signals:
    void ready();

    void startDownload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);
    void startUpload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);

    void slotUrlReceived(const QString &slotId, const QUrl &url);
    void slotUrlErrorOcurrend(const QString &slotId);

private:
    void onServiceFound();
    void onSlotReceived(const QXmppHttpUploadSlotIq &slot);
    void onRequestFailed(const QXmppHttpUploadRequestIq &request);
    void onStartDownload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);
    void onStartUpload(const QUrl &url, QFile *file, const ConnectionSetup &connectionSetup);

    QNetworkAccessManager *m_networkAccessManager;
    QXmppUploadRequestManager *m_xmppManager;
    bool m_isReady = false;
};
}

Q_DECLARE_METATYPE(QXmppHttpUploadSlotIq)
Q_DECLARE_METATYPE(QXmppHttpUploadRequestIq)
Q_DECLARE_METATYPE(vm::FileLoader::ConnectionSetup)

#endif // VS_FILELOADER_H
