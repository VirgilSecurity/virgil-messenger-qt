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

#include "XmppLastActivityIq.h"

#include <QLoggingCategory>
#include <QDomElement>

using namespace vm;
using Self = XmppLastActivityIq;

static constexpr const char *const ns_last = "jabber:iq:last";

Q_LOGGING_CATEGORY(lcLastActivity, "lastActivity");

bool Self::isValid() const
{
    return m_valid;
}

std::chrono::seconds Self::seconds() const
{
    return m_seconds;
}

bool Self::needSubscription() const
{
    return type() == QXmppIq::Type::Error && error().code() == 407;
}

bool Self::isLastActivityId(const QDomElement &element)
{
    const QDomElement queryElement = element.firstChildElement("query");
    return queryElement.namespaceURI() == ns_last;
}

QStringList Self::discoveryFeatures()
{
    return { ns_last };
}

void Self::parseElementFromChild(const QDomElement &element)
{
    m_valid = false;
    QXmppIq::parseElementFromChild(element);
    if (type() == QXmppIq::Type::Result) {
        QDomElement queryElement = element.firstChildElement("query");
        const auto secondsStr = queryElement.attribute("seconds");
        m_seconds = std::chrono::seconds(secondsStr.toUInt(&m_valid));
        if (!m_valid) {
            qCWarning(lcLastActivity) << "Convertation error:" << secondsStr;
        }
    } else {
        if (type() == QXmppIq::Type::Error) {
            qCWarning(lcLastActivity) << "Error: " << error().code() << error().text();
        } else {
            qCWarning(lcLastActivity) << "Invalid result type: " << type();
        }
    }
}

void Self::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeDefaultNamespace(ns_last);
    writer->writeEndElement();
}
