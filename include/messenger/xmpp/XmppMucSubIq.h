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

#ifndef VM_XMPP_MUC_SUB_IQ_H
#define VM_XMPP_MUC_SUB_IQ_H

#include <qxmpp/QXmppIq.h>

#include <list>

namespace vm {

//
//
//
enum class XmppMucSubEvent {
    Unspecified,
    Presence,
    Messages,
    Affiliations,
    Subscribers,
    Config,
    Subject,
    System,
};

//
//  Represents XML element: <subscribe xmlns='urn:xmpp:mucsub:0'/>
//
class XmppMucSubscribeItem
{
public:
    QString jid() const;
    void setJid(const QString &jid);

    QString nickName() const;
    void setNickName(const QString &nickName);

    QString password() const;
    void setPassword(const QString &password);

    std::list<XmppMucSubEvent> events() const;
    void setEvents(const std::list<XmppMucSubEvent> &events);

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

private:
    QString m_jid;
    QString m_nickName;
    QString m_password;
    std::list<XmppMucSubEvent> m_events;
};

//
//  Represents XML element: <subscription/> from the  <subscriptions xmlns='urn:xmpp:mucsub:0'/>
//
class XmppMucSubscriptionItem
{
public:
    QString jid() const;

    std::list<XmppMucSubEvent> events() const;

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

private:
    QString m_jid;
    std::list<XmppMucSubEvent> m_events;
};

//
//  IQ for subscribing to MUC/Sub subscription list.
//
class XmppMucSubscriptionsIq : public QXmppIq
{
public:
    using QXmppIq::QXmppIq;

    //
    //  Return subscriptions.
    //
    std::list<XmppMucSubscriptionItem> items() const;

    //
    //  Return true if IQ handles subscriptions of my rooms.
    //
    bool isMySubscriptions() const;

    //
    //  Return true if IQ handles subscribers subscriptions of specific room.
    //
    bool isRoomSubscriptions() const;

    static bool isMucSubscriptionsIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    std::list<XmppMucSubscriptionItem> m_items;
};

//
//  IQ for subscribing to MUC/Sub events.
//
class XmppMucSubscribeIq : public QXmppIq
{
public:
    using QXmppIq::QXmppIq;

    XmppMucSubscribeItem item() const;
    void setItem(const XmppMucSubscribeItem &item);

    static bool isMucSubscribeIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    XmppMucSubscribeItem m_item;
};

} // namespace vm

Q_DECLARE_METATYPE(vm::XmppMucSubEvent);
Q_DECLARE_METATYPE(std::list<vm::XmppMucSubEvent>);

#endif // VM_XMPP_MUC_SUB_IQ_H
