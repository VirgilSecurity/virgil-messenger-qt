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

#include "XmppMucSubManager.h"

#include <qxmpp/QXmppMessage.h>

#include <QPointer>
#include <QDomElement>
#include <QLoggingCategory>

using namespace vm;
using Self = XmppMucSubManager;


Q_LOGGING_CATEGORY(lsXmppMucSubManager, "xmpp-muc-sub");


class Self::Impl {
public:
    QPointer<QXmppClient> client;
};

namespace {
class MetaTypeInitilazer {
public:
    MetaTypeInitilazer() {
        qRegisterMetaType<vm::XmppMucSubEvent>();
        qRegisterMetaType<std::list<vm::XmppMucSubEvent>>();
    }
};
}


Self::XmppMucSubManager() : m_impl(std::make_shared<Self::Impl>()) {
    static MetaTypeInitilazer _;
}


QString Self::subscribe(const std::list<XmppMucSubEvent>& events,
            const QString& from, const QString& to, const QString& nickName) {

    XmppMucSubscribeItem subscribeItem;
    subscribeItem.setEvents(events);
    subscribeItem.setNickName(nickName);

    XmppMucSubscribeIq iq(QXmppIq::Type::Set);
    iq.setTo(to);
    iq.setFrom(from);
    iq.setItem(subscribeItem);

    if (m_impl->client && m_impl->client->sendPacket(iq)) {
        qCDebug(lsXmppMucSubManager) << "User:" << from << "nickname:" << nickName <<  "subscribed to the room:" << to;
    }

    return iq.id();
}


bool Self::handleStanza(const QDomElement &element) {
    if (element.tagName() == "iq") {

        if (XmppMucSubscriptionsIq::isMucSubscriptionsIq(element)) {
            XmppMucSubscriptionsIq iq;
            iq.parse(element);

            if (iq.isMySubscriptions()) {
                for (const auto& subscribedRoom : iq.items()) {
                    emit subscribedRoomReceived(iq.id(), subscribedRoom.jid(), iq.to(), subscribedRoom.events());
                }

                emit subscribedRoomsProcessed(iq.id());
            }

            if (iq.isRoomSubscriptions()) {
                for (const auto& subscriber : iq.items()) {
                    emit roomSubscriberReceived(iq.id(), iq.from(), subscriber.jid(), subscriber.events());
                }

                emit roomSubscribersProcessed(iq.id(), iq.from());
            }

        } else if (XmppMucSubscribeIq::isMucSubscribeIq(element)) {
            XmppMucSubscribeIq iq;
            iq.parse(element);

            emit subscribedRoomReceived(iq.id(), iq.from(), iq.to(), iq.item().events());
            emit subscribedRoomsProcessed(iq.id());
        }
    }
    return false;
}


void Self::setClient(QXmppClient *client) {

    QXmppClientExtension::setClient(client);

    connect(client, &QXmppClient::messageReceived, this, &Self::onMessageReceived);

    m_impl->client = client;
}


void Self::onMessageReceived(const QXmppMessage &mucSubMessage) {

    if (mucSubMessage.type() != QXmppMessage::Normal) {
        return;
    }

    for (const auto& mucSubEvent : mucSubMessage.extensions()) {
        if (mucSubEvent.attribute(QStringLiteral("xmlns")) !=
                    QStringLiteral("http://jabber.org/protocol/pubsub#event")) {
            continue;
        }

        auto items = mucSubEvent.firstChildElement(QStringLiteral("items"));
        if (items.isNull()) {
            continue;
        }

        auto node = items.attribute(QStringLiteral("node"));

        //
        //  Process messages.
        //
        if (node == QStringLiteral("urn:xmpp:mucsub:nodes:messages")) {

            auto item = items.firstChildElement(QStringLiteral("item"));
            for(;!item.isNull(); item = items.nextSiblingElement(QStringLiteral("item"))) {
                auto messageElement = item.firstChildElement(QStringLiteral("message"));
                if (!messageElement.isNull()) {
                    QXmppMessage message;
                    message.parse(messageElement.sourceDomElement());
                    qCDebug(lsXmppMucSubManager) << "Got message:" << item.attribute("id") << message.id();
                    emit messageReceived(message);
                }
            }
        }

        //
        //  Process subscribers.
        //
        if (node == QStringLiteral("urn:xmpp:mucsub:nodes:subscribers")) {

            auto item = items.firstChildElement(QStringLiteral("item"));
            auto id = item.attribute("id");
            for(;!item.isNull(); item = items.nextSiblingElement(QStringLiteral("item"))) {
                auto subscribeElement = item.firstChildElement(QStringLiteral("subscribe"));
                if (!subscribeElement.isNull()) {
                    XmppMucSubscribeItem subscribeItem;
                    subscribeItem.parse(subscribeElement.sourceDomElement());
                    qCDebug(lsXmppMucSubManager) << "Got subscriber:" << id << subscribeItem.jid();
                    emit subscribeReceived(mucSubMessage.from(), subscribeItem.jid(), subscribeItem.nickName());
                }
            }
        }
    }
}
