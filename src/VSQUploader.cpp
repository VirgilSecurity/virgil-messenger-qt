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

#include "VSQUploader.h"

#include <QXmppClient.h>
#include <QXmppUploadRequestManager.h>

Q_LOGGING_CATEGORY(lcUploader, "uploader");

VSQUploader::VSQUploader(QXmppClient *client, QObject *parent)
    : QObject(parent)
    , m_xmppManager(new QXmppUploadRequestManager())
{
    connect(this, &VSQUploader::upload, this, &VSQUploader::onUpload);

    client->addExtension(m_xmppManager);
    connect(m_xmppManager, &QXmppUploadRequestManager::slotReceived, this, &VSQUploader::onSlotReceived);
    connect(m_xmppManager, &QXmppUploadRequestManager::requestFailed, this, &VSQUploader::onRequestFailed);
}

VSQUploader::~VSQUploader()
{
    delete m_xmppManager;
}

void VSQUploader::onUpload(const QString &messageId, const Attachment &attachment)
{
    // FIXME(fpohtmeh): finish
    //m_xmppManager->requestUploadSlot();
    QUrl url("https://raw.githubusercontent.com/VirgilSecurity/virgil-messenger-qt/develop/src/qml/resources/icons/Logo.png");
    emit uploadUrlReceived(messageId, url);
}

void VSQUploader::onSlotReceived(const QXmppHttpUploadSlotIq &slot)
{
    qDebug() << "onSlotReceived";
}

void VSQUploader::onRequestFailed(const QXmppHttpUploadRequestIq &request)
{
    qDebug() << "onRequestFailed";
}
