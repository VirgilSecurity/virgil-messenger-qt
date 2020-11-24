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

#include "operations/SendMessageOperation.h"

#include <QXmppClient.h>
#include <QXmppMessage.h>

#include "Core.h"
#include "Utils.h"
#include "operations/MessageOperation.h"

using namespace vm;

SendMessageOperation::SendMessageOperation(MessageOperation *parent, QXmppClient *xmpp, const QString &xmppUrl)
    : NetworkOperation(parent)
    , m_parent(parent)
    , m_xmpp(xmpp)
    , m_xmppUrl(xmppUrl)
{
    setName(QLatin1String("SendMessage"));
}

void SendMessageOperation::run()
{
    const auto message = m_parent->message();
    const auto encryptedStr = Core::encryptMessage(*message, message->recipientId);
    if (!encryptedStr) {
        qCDebug(lcOperation) << "Failed to encrypt message";
        invalidate(tr("Failed to encrypt message"));
    }
    else {
        const auto fromJID = Utils::createJid(message->senderId, m_xmppUrl);
        const auto toJID = Utils::createJid(message->recipientId, m_xmppUrl);

        QXmppMessage msg(fromJID, toJID, *encryptedStr);
        msg.setReceiptRequested(true);
        msg.setId(message->id);

        if (m_xmpp->sendPacket(msg)) {
            qCDebug(lcOperation) << "Message sent:" << message->id;
            finish();
        } else {
            qCDebug(lcOperation) << "Message NOT sent:" << message->id;
            fail();
        }
    }
}
