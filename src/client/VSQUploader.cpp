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

#include "client/VSQUploader.h"

#include <QXmppClient.h>

#include <QEventLoop> // FIXME(fpohtmeh): remove upload emulation code
#include <QThread> // FIXME(fpohtmeh): remove upload emulation code

VSQUploader::VSQUploader(QXmppClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    // FIXME(fpohtmeh): finish uploader initialization
    client->addExtension(&m_manager);
}

VSQUploader::~VSQUploader()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Uploader";
#endif
}

void VSQUploader::upload(const ExtMessage &message)
{
    const auto id = message.id;
    const DataSize total = message.attachment->size;

    emit uploadStarted(message);
    QEventLoop loop;
    DataSize u = 0;
    for (; u <= total; u += total / 30) {
        emit uploadProgressChanged(message, u, total);
        loop.processEvents();
        QThread::currentThread()->msleep(100);
    }
    if (u != total) {
        emit uploadProgressChanged(message, total, total);
        loop.processEvents();
    }
    // FIXME(fpohtmeh): implement real upload
    emit uploaded(message);
    loop.processEvents();
    // FIXME(fpohtmeh): remove upload emulation code
    if (m_client->sendPacket(message.xmpp))
        emit messageSent(message);
    else
        emit sendMessageFailed(message, QLatin1String("Message sending failed"));
}
