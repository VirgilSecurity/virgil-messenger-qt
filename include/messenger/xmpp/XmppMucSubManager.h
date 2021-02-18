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

#ifndef VM_XMPP_MUC_SUB_MANAGER_H
#define VM_XMPP_MUC_SUB_MANAGER_H

#include <qxmpp/QXmppClientExtension.h>
#include <qxmpp/QXmppClient.h>

#include "XmppMucSubIq.h"

#include <list>
#include <memory>

namespace vm {

//
//  Manages "Multi-User Chat Subscriptions" - ejabberd extension.
//  https://docs.ejabberd.im/developer/xmpp-clients-bots/extensions/muc-sub
//
class XmppMucSubManager : public QXmppClientExtension
{
    Q_OBJECT

public:

public:
    XmppMucSubManager();

    //
    //  Subscribe to given events.
    //  Return operation id.
    //
    QString subscribe(const std::list<XmppMucSubEvent>& events,
            const QString& from, const QString& to, const QString& nickName);

    //
    //  Unsubscribe from a MUC Room.
    //  A room moderator can unsubscribe another room user from MUC Room events by providing the user JID.
    //  Return operation id.
    //
    QString unsubscribe(const QString& jid = {});

    //
    //  Get list of the subscribed rooms.
    //  Signal subscribedRoomReceived() is emitted for each room.
    //  Signal subscribedRoomsProcessed() is emitted when all rooms were processed.
    //  Return operation id.
    //
    QString retreiveSubscribedRooms(const QString& from);

    //
    //  Get list of the subscribed rooms.
    //  Signal roomSubscriberReceived() is emitted for each room subscriber.
    //  Signal roomSubscribersProcessed() is emitted when all room subscribers were processed.
    //  Return operation id.
    //
    QString retreiveRoomSubscribers(const QString& from, const QString& to);

    //
    //  Handle IQs with subscription events.
    //
    bool handleStanza(const QDomElement &element) override;

Q_SIGNALS:

    void messageReceived(const QXmppMessage& message);
    void subscribeReceived(const QString& roomJid, const QString& subscriberJid, const QString& nickname);
    void unsubscribeReceived();

    void subscribedRoomReceived(const QString& id, const QString& roomJid, const QString& subscriberJid,
                const std::list<XmppMucSubEvent>& events);

    void subscribedRoomsProcessed(const QString& id);

    void roomSubscriberReceived(const QString& id, const QString& roomJid, const QString& subscriberJid,
                const std::list<XmppMucSubEvent>& events);

    void roomSubscribersProcessed(const QString& id, const QString& roomJid);

protected:
    void setClient(QXmppClient *client) override;

private:
    void onMessageReceived(const QXmppMessage &message);

private:
    class Impl;
    std::shared_ptr<Impl> m_impl;;
};

} // vm

#endif // VM_XMPP_MUC_SUB_MANAGER_H
