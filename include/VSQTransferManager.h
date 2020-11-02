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

#ifndef VSQ_TRANSFERMANAGER_H
#define VSQ_TRANSFERMANAGER_H

#include <QObject>
#include <QMutex>

#include <QXmppHttpUploadIq.h>

#include "VSQCommon.h"
#include "VSQTransfer.h"

class QNetworkAccessManager;

class QXmppClient;
class QXmppUploadRequestManager;

class VSQDownload;
class VSQSettings;
class VSQUpload;

Q_DECLARE_LOGGING_CATEGORY(lcTransferManager);

class VSQTransferManager : public QObject {
    Q_OBJECT

public:
    VSQTransferManager(QXmppClient *client,
                       QNetworkAccessManager *networkAccessManager,
                       VSQSettings *settings,
                       QObject *parent);
    virtual ~VSQTransferManager();

    VSQUpload *
    startUpload(const QString &id, const QString &filePath);
    VSQDownload *
    startDownload(const QString &id, const QUrl &remoteUrl, const QString &filePath);

    bool
    isReady() const;
    bool
    hasTransfer(const QString &id) const;

signals:
    void
    progressChanged(const QString id, const DataSize bytesReceived, const DataSize bytesTotal);
    void
    statusChanged(const QString id, const Enums::AttachmentStatus status);
    void
    connectionChanged();
    void
    fireReadyToUpload();

    void
    startTransfer(VSQTransfer *transfer, QPrivateSignal);

protected:
    VSQSettings *
    settings();

private:
    bool
    requestUploadUrl(VSQUpload *upload);

    VSQUpload *
    findUploadBySlotId(const QString &slotId) const;
    VSQTransfer *
    findTransfer(const QString &id, bool showWarning = true) const;

    void
    removeTransfer(VSQTransfer *transfer, bool lock);
    void
    abortTransfer(VSQTransfer *transfer, bool lock);
    void
    onStartTransfer(VSQTransfer *transfer);

    void
    onSlotReceived(const QXmppHttpUploadSlotIq &slot);
    void
    onRequestFailed(const QXmppHttpUploadRequestIq &request);

    QXmppClient *m_client;
    QNetworkAccessManager *m_networkAccessManager;
    VSQSettings *m_settings;
    QXmppUploadRequestManager *m_xmppManager;

    QVector<VSQTransfer *> m_transfers;
    mutable QMutex m_transfersMutex;
};

Q_DECLARE_METATYPE(QXmppHttpUploadSlotIq);
Q_DECLARE_METATYPE(QXmppHttpUploadRequestIq);

#endif // VSQ_TRANSFERMANAGER_H
