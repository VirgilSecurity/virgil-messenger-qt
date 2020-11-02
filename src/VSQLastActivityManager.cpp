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

#include "VSQLastActivityManager.h"

#include <QDomElement>

#include <QXmppClient.h>
#include <QXmppRosterManager.h>

#include "VSQSettings.h"
#include "VSQUtils.h"

VSQLastActivityManager::VSQLastActivityManager(VSQSettings *settings, QObject *parent)
    : QXmppClientExtension(), m_settings(settings) {
    setParent(parent);
    connect(this, &VSQLastActivityManager::lastActivityMissing, this, &VSQLastActivityManager::lastActivityTextChanged);
    connect(this, &VSQLastActivityManager::lastActivityDetected, this, [this](const Seconds &seconds) {
        emit lastActivityTextChanged(
                VSQUtils::formattedLastSeenActivity(seconds, m_settings->lastSeenActivityInterval()));
    });
    connect(this, &VSQLastActivityManager::errorOccured, this, &VSQLastActivityManager::onErrorOccured);
}

VSQLastActivityManager::~VSQLastActivityManager() {
    stopUpdates(true);
}

void
VSQLastActivityManager::setCurrentJid(const QString &jid) {
    if (m_jid == jid) {
        return;
    }
    m_jid = jid;
    canStart() ? startUpdates(true) : stopUpdates(true);
}

void
VSQLastActivityManager::setEnabled(bool enabled) {
    if (enabled == m_enabled) {
        return;
    }
    m_enabled = enabled;
    canStart() ? startUpdates(false) : stopUpdates(false);
}

QStringList
VSQLastActivityManager::discoveryFeatures() const {
    return VSQLastActivityIq::discoveryFeatures();
}

bool
VSQLastActivityManager::handleStanza(const QDomElement &element) {
    if (element.tagName() == "iq" && VSQLastActivityIq::isLastActivityId(element)) {
        VSQLastActivityIq lastActivityIq(m_debugCounter > 0);
        lastActivityIq.parse(element);
        if (m_jid != lastActivityIq.from()) {
            return false;
        }
        if (lastActivityIq.isValid()) {
            emit lastActivityDetected(lastActivityIq.seconds());
        } else if (lastActivityIq.needSubscription()) {
            emit lastActivityMissing(VSQUtils::formattedLastSeenNoActivity());
        } else {
            emit errorOccured(tr("Failed to find last activity"));
        }
        --m_debugCounter;
        return true;
    }
    return false;
}

void
VSQLastActivityManager::timerEvent(QTimerEvent *) {
    requestInfo();
}

QString
VSQLastActivityManager::requestInfo() {
    if (m_jid.isEmpty()) {
        return QString();
    }
    if (m_debugCounter > 0) {
        qCDebug(lcLastActivity) << "Requesting info for:" << m_jid;
    }
    VSQLastActivityIq request(m_debugCounter > 0);
    request.setType(QXmppIq::Get);
    request.setTo(m_jid);
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        qCWarning(lcLastActivity) << "Last activity request failed";
        return QString();
    }
}

bool
VSQLastActivityManager::canStart() const {
    return !m_jid.isEmpty() && m_enabled;
}

void
VSQLastActivityManager::startUpdates(bool reset) {
    stopUpdates(reset);
    if (canStart()) {
        m_debugCounter = 3; // debug few records only
        requestInfo();
        m_timerId = startTimer(m_settings->lastSeenActivityInterval() * 1000);
        if (m_timerId == 0) {
            emit errorOccured(tr("Failed to start timer"));
        }
    }
}

void
VSQLastActivityManager::stopUpdates(bool reset) {
    if (m_timerId == 0) {
        return;
    }
    killTimer(m_timerId);
    m_timerId = 0;

    if (reset) {
        emit lastActivityMissing(QChar(' '));
    }
}

void
VSQLastActivityManager::onErrorOccured(const QString &errorText) {
    qCWarning(lcLastActivity) << errorText;
    stopUpdates(true);
}
