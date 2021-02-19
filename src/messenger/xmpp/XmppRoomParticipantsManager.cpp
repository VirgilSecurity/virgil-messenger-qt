//  Copyright (C) 2015-2021 Virgil Security, Inc.
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
//

#include "XmppRoomParticipantsManager.h"

#include <QDomElement>
#include <QLoggingCategory>

using namespace vm;
using Self = XmppRoomParticipantsManager;


Q_LOGGING_CATEGORY(lcXmppRoomParticipantsManager, "xmpp-room-participants-manager");

static const constexpr std::array<QXmppMucItem::Affiliation, 3> kAffiliations{
        QXmppMucItem::OwnerAffiliation, QXmppMucItem::AdminAffiliation, QXmppMucItem::MemberAffiliation};

void Self::requestAll(const QString& roomJid) {


    for (auto affiliation : kAffiliations) {
        QXmppMucItem item;
        item.setAffiliation(affiliation);

        QXmppMucAdminIq iq;
        iq.setTo(roomJid);
        iq.setType(QXmppIq::Type::Get);
        iq.setItems({ item });

        if (m_client && m_client->sendPacket(iq)) {
            qCDebug(lcXmppRoomParticipantsManager) << "Requested group:" << roomJid << QXmppMucItem::affiliationToString(affiliation);
        }
    }
}


bool Self::handleStanza(const QDomElement &element) {
    if (element.tagName() == "iq" && QXmppMucAdminIq::isMucAdminIq(element)) {

        QXmppMucAdminIq iq;
        iq.parse(element);

        auto roomJid = iq.from();

        for (const auto &item : iq.items()) {
            const auto affiliation = item.affiliation();

            if (std::find(kAffiliations.cbegin(), kAffiliations.cend(), affiliation) == kAffiliations.cend()) {
                return false;
            }

            const auto jid = item.jid();
            emit participantReceived(roomJid, jid, affiliation);
        }
    }
    return false;
}


void Self::setClient(QXmppClient *client) {

    QXmppClientExtension::setClient(client);

    m_client = client;
}
