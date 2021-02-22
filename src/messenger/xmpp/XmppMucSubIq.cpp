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

#include "XmppMucSubIq.h"

#include "qxmpp/QXmppUtils.h"

#include <QDomElement>

using namespace vm;


static const QLatin1String ns_muc_sub("urn:xmpp:mucsub:0");

static const QLatin1String ns_muc_sub_node_presence("urn:xmpp:mucsub:nodes:presence");
static const QLatin1String ns_muc_sub_node_messages("urn:xmpp:mucsub:nodes:messages");
static const QLatin1String ns_muc_sub_node_affiliations("urn:xmpp:mucsub:nodes:affiliations");
static const QLatin1String ns_muc_sub_node_subscribers("urn:xmpp:mucsub:nodes:subscribers");
static const QLatin1String ns_muc_sub_node_config("urn:xmpp:mucsub:nodes:config");
static const QLatin1String ns_muc_sub_node_subject("urn:xmpp:mucsub:nodes:subject");
static const QLatin1String ns_muc_sub_node_system("urn:xmpp:mucsub:nodes:system");

// --------------------------------------------------------------------------
//  Helpers.
// --------------------------------------------------------------------------
static QString xmppMucSubEventToNodeString(XmppMucSubEvent event) {

    switch (event) {
    case XmppMucSubEvent::Unspecified:
        return QStringLiteral("");

    case XmppMucSubEvent::Presence:
        return ns_muc_sub_node_presence;

    case XmppMucSubEvent::Messages:
        return ns_muc_sub_node_messages;

    case XmppMucSubEvent::Affiliations:
        return ns_muc_sub_node_affiliations;

    case XmppMucSubEvent::Subscribers:
        return ns_muc_sub_node_subscribers;

    case XmppMucSubEvent::Config:
        return ns_muc_sub_node_config;

    case XmppMucSubEvent::Subject:
        return ns_muc_sub_node_subject;

    case XmppMucSubEvent::System:
        return ns_muc_sub_node_system;
    }
}


static XmppMucSubEvent xmppMucSubEventFromNodeString(const QString& eventString) {

    if (eventString == ns_muc_sub_node_presence) {
        return XmppMucSubEvent::Presence;

    } else if (eventString == ns_muc_sub_node_messages) {
        return XmppMucSubEvent::Messages;

    } else if (eventString == ns_muc_sub_node_affiliations) {
        return XmppMucSubEvent::Affiliations;

    } else if (eventString == ns_muc_sub_node_subscribers) {
        return XmppMucSubEvent::Subscribers;

    } else if (eventString == ns_muc_sub_node_config) {
        return XmppMucSubEvent::Config;

    } else if (eventString == ns_muc_sub_node_subject) {
        return XmppMucSubEvent::Subject;

    } else if (eventString == ns_muc_sub_node_system) {
        return XmppMucSubEvent::System;

    } else {
        return XmppMucSubEvent::Unspecified;
    }
}


// --------------------------------------------------------------------------
//  XmppMucSubscribeItem
// --------------------------------------------------------------------------
QString XmppMucSubscribeItem::jid() const {
    return m_jid;
}

void XmppMucSubscribeItem::setJid(const QString& jid) {
    m_jid = jid;
}


QString XmppMucSubscribeItem::nickName() const {
    return m_nickName;
}

void XmppMucSubscribeItem::setNickName(const QString& nickName) {
    m_nickName = nickName;
}


QString XmppMucSubscribeItem::password() const {
    return m_password;
}

void XmppMucSubscribeItem::setPassword(const QString& password) {
    m_password = password;
}


std::list<XmppMucSubEvent> XmppMucSubscribeItem::events() const {
    return m_events;
}

void XmppMucSubscribeItem::setEvents(const std::list<XmppMucSubEvent>& events) {
    m_events = events;
}


void XmppMucSubscribeItem::parse(const QDomElement &element) {

    m_jid = element.attribute(QStringLiteral("jid"));
    m_nickName = element.attribute(QStringLiteral("nick"));
    m_password = element.attribute(QStringLiteral("password"));

    auto child = element.firstChildElement(QStringLiteral("event"));
    for(m_events.clear(); !child.isNull(); child = child.nextSiblingElement(QStringLiteral("event"))) {

        auto node = element.attribute(QStringLiteral("node"));
        auto event = xmppMucSubEventFromNodeString(node);

        if (event != XmppMucSubEvent::Unspecified) {
            m_events.push_back(event);
        }
    }
}


void XmppMucSubscribeItem::toXml(QXmlStreamWriter *writer) const {
    writer->writeStartElement(QStringLiteral("subscribe"));
    writer->writeDefaultNamespace(ns_muc_sub);

    helperToXmlAddAttribute(writer, QStringLiteral("jid"), m_jid);
    helperToXmlAddAttribute(writer, QStringLiteral("nick"), m_nickName);
    helperToXmlAddAttribute(writer, QStringLiteral("password"), m_password);

    for (const auto& event: m_events) {
        writer->writeStartElement(QStringLiteral("event"));
        helperToXmlAddAttribute(writer, QStringLiteral("node"), xmppMucSubEventToNodeString(event));
        writer->writeEndElement();
    }

    writer->writeEndElement();
}


// --------------------------------------------------------------------------
//  XmppMucSubscriptionItem
// --------------------------------------------------------------------------

QString XmppMucSubscriptionItem::jid() const {
    return m_jid;
}

std::list<XmppMucSubEvent> XmppMucSubscriptionItem::events() const {
    return m_events;
}

void XmppMucSubscriptionItem::parse(const QDomElement &element) {
    m_jid = element.attribute(QStringLiteral("jid"));

    auto child = element.firstChildElement(QStringLiteral("event"));
    for(m_events.clear(); !child.isNull(); child = child.nextSiblingElement(QStringLiteral("event"))) {

        auto node = element.attribute(QStringLiteral("node"));
        auto event = xmppMucSubEventFromNodeString(node);

        if (event != XmppMucSubEvent::Unspecified) {
            m_events.push_back(event);
        }
    }
}


void XmppMucSubscriptionItem::toXml(QXmlStreamWriter *writer) const {

    writer->writeStartElement(QStringLiteral("subscription"));

    helperToXmlAddAttribute(writer, QStringLiteral("jid"), m_jid);

    for (const auto& event: m_events) {
        writer->writeStartElement(QStringLiteral("event"));
        helperToXmlAddAttribute(writer, QStringLiteral("node"), xmppMucSubEventToNodeString(event));
        writer->writeEndElement();
    }
}

// --------------------------------------------------------------------------
//  XmppMucSubscriptionsIq
// --------------------------------------------------------------------------
std::list<XmppMucSubscriptionItem> XmppMucSubscriptionsIq::items() const {
    return m_items;
}

bool XmppMucSubscriptionsIq::isMySubscriptions() const {
    return from().indexOf(QChar('@')) < 0;
}


bool XmppMucSubscriptionsIq::isRoomSubscriptions() const {
    return from().indexOf(QChar('@')) > 0;
}


bool XmppMucSubscriptionsIq::isMucSubscriptionsIq(const QDomElement &element) {
    QDomElement queryElement = element.firstChildElement(QStringLiteral("subscriptions"));
    return (queryElement.namespaceURI() == ns_muc_sub);
}


void XmppMucSubscriptionsIq::parseElementFromChild(const QDomElement &element) {

    auto queryElement = element.firstChildElement(QStringLiteral("subscriptions"));

    auto child = queryElement.firstChildElement(QStringLiteral("subscription"));
    for (m_items.clear(); !child.isNull(); child = child.nextSiblingElement(QStringLiteral("subscription"))) {

        XmppMucSubscriptionItem item;
        item.parse(child);
        m_items.push_back(std::move(item));
    }
}


void XmppMucSubscriptionsIq::toXmlElementFromChild(QXmlStreamWriter *writer) const {

    writer->writeStartElement(QStringLiteral("subscriptions"));
    writer->writeDefaultNamespace(ns_muc_sub);

    for (const auto &item : m_items) {
        item.toXml(writer);
    }

    writer->writeEndElement();
}


// --------------------------------------------------------------------------
//  XmppMucSubscriptionsIq
// --------------------------------------------------------------------------
XmppMucSubscribeItem XmppMucSubscribeIq::item() const {
    return m_item;
}


void XmppMucSubscribeIq::setItem(const XmppMucSubscribeItem &item) {
    m_item = item;
}


bool XmppMucSubscribeIq::isMucSubscribeIq(const QDomElement &element) {
    QDomElement queryElement = element.firstChildElement(QStringLiteral("subscribe"));
    return (queryElement.namespaceURI() == ns_muc_sub);
}


void XmppMucSubscribeIq::parseElementFromChild(const QDomElement &element) {
    QDomElement queryElement = element.firstChildElement(QStringLiteral("subscribe"));
    m_item.parse(queryElement);
}


void XmppMucSubscribeIq::toXmlElementFromChild(QXmlStreamWriter *writer) const {
    m_item.toXml(writer);
}

