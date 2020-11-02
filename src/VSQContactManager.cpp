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

#include "VSQContactManager.h"

#include <qxmpp/QXmppClient.h>

Q_LOGGING_CATEGORY(lcContactManager, "contactman");

VSQContactManager::VSQContactManager(QXmppClient *client, QObject *parent)
    : QObject(parent), m_client(client), m_manager(client->findExtension<QXmppRosterManager>()) {
}

VSQContactManager::~VSQContactManager() {
}

bool
VSQContactManager::addContact(const QString &jid, const QString &name, const QString &reason) {
    if (m_client->state() != QXmppClient::ConnectedState) {
        return setLastErrorText(tr("No connection"));
    }

    const auto status = find(jid);
    if (status.exists) {
        qCWarning(lcContactManager) << "Contact already added:" << jid;
    } else if (!m_manager->addItem(jid, name)) {
        return setLastErrorText(tr("Contact adding failed"));
    }

    if (status.subscriptionType == SubscriptionType::Both) {
        qCWarning(lcContactManager) << "Contact already subscribed" << jid;
    } else if (!m_manager->subscribe(jid, reason)) {
        return setLastErrorText(tr("Contact subscription failed"));
    }

    return true;
}

bool
VSQContactManager::removeContact(const QString &jid) {
    if (m_client->state() != QXmppClient::ConnectedState) {
        return setLastErrorText(tr("No connection"));
    }

    const auto status = find(jid);
    if (!status.exists) {
        return setLastErrorText(tr("Contact doesn't exist"));
    }

    if (status.subscriptionType == SubscriptionType::None) {
        qCWarning(lcContactManager) << "Contact wasn't subscribed" << jid;
    } else if (!m_manager->unsubscribe(jid)) {
        return setLastErrorText(tr("Contact unsubscription failed"));
    }

    if (!m_manager->removeItem(jid)) {
        return setLastErrorText(tr("Contact removing failed"));
    }

    return true;
}

bool
VSQContactManager::renameContact(const QString &jid, const QString &newName) {
    if (m_client->state() != QXmppClient::ConnectedState) {
        return setLastErrorText(tr("No connection"));
    }

    if (!m_manager->renameItem(jid, newName)) {
        return setLastErrorText(tr("Contact renaming failed"));
    }

    return true;
}

QString
VSQContactManager::lastErrorText() const {
    return m_lastErrorText;
}

VSQContactManager::ContactInfo
VSQContactManager::find(const QString &jid) const {
    ContactInfo status;
    status.exists = m_manager->getRosterBareJids().contains(jid);
    if (status.exists) {
        status.subscriptionType = m_manager->getRosterEntry(jid).subscriptionType();
        qCDebug(lcContactManager) << "Subsription type:" << status.subscriptionType << "jid:" << jid;
    }
    return status;
}

bool
VSQContactManager::setLastErrorText(const QString &text) {
    m_lastErrorText = text;
    qCWarning(lcContactManager) << text;
    return false;
}
