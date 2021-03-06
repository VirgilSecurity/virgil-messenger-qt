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

#include "XmppDiscoveryManager.h"

#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppCarbonManager.h>

#include <QLoggingCategory>

using namespace vm;
using Self = XmppDiscoveryManager;

Q_LOGGING_CATEGORY(lcDiscoveryManager, "discovery-manager");

Self::XmppDiscoveryManager(QXmppClient *client, QObject *parent)
    : QObject(parent), m_client(client), m_manager(client->findExtension<QXmppDiscoveryManager>())
{
    connect(client, &QXmppClient::connected, this, &Self::onClientConnected);
    connect(m_manager, &QXmppDiscoveryManager::infoReceived, this, &Self::onInfoReceived);
    connect(m_manager, &QXmppDiscoveryManager::itemsReceived, this, &Self::onItemsReceived);
}

Self::~XmppDiscoveryManager() { }

void Self::onClientConnected()
{
    const auto domain = m_client->configuration().domain();

    auto infoId = m_manager->requestInfo(domain);
    qCDebug(lcDiscoveryManager) << "Discovery manager info requested" << domain << "id:" << infoId;

    auto itemsId = m_manager->requestItems(domain);
    qCDebug(lcDiscoveryManager) << "Discovery manager items requested" << domain << "id:" << itemsId;
}

void Self::onInfoReceived(const QXmppDiscoveryIq &info)
{
    if (info.from() != m_client->configuration().domain()) {
        return;
    }

    if (info.features().contains("urn:xmpp:carbons:2")) {
        qCDebug(lcDiscoveryManager) << "Set carbon manager to enabled";

        QXmppElement carbonsElement;
        carbonsElement.setTagName("enable");
        carbonsElement.setAttribute("xmlns", "urn:xmpp:carbons:2");

        QXmppIq carbonsIq(QXmppIq::Set);
        carbonsIq.setExtensions(QXmppElementList() << carbonsElement);

        if (m_client->sendPacket(carbonsIq)) {
            qCDebug(lcDiscoveryManager) << "Carbon manager was enabled";
        } else {
            qCDebug(lcDiscoveryManager) << "Carbon manager was not enabled (failed to send packet)";
        }
    }

    for (const auto &feature : info.features()) {
        qCDebug(lcDiscoveryManager) << "Discovered XMPP feature:" << feature;
    }
}

void Self::onItemsReceived(const QXmppDiscoveryIq &info)
{
    const auto domain = m_client->configuration().domain();
    const QList<QXmppDiscoveryIq::Item> items = info.items();
    for (const QXmppDiscoveryIq::Item &item : items) {
        if (item.jid() == domain) {
            continue;
        }

        qCDebug(lcDiscoveryManager) << "Discovery manager info requested" << item.jid();
        m_manager->requestInfo(item.jid());
    }
}
