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

#include "Settings.h"
#include "Utils.h"

#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppRosterManager.h>

#include <QLoggingCategory>
#include <QDomElement>

using namespace vm;
using Self = VSQLastActivityManager;

Q_LOGGING_CATEGORY(lcLastActivityManager, "last-activity-manager");

Self::VSQLastActivityManager(Settings *settings) : QXmppClientExtension(), m_settings(settings)
{
    connect(this, &Self::lastActivityMissing, this, &Self::lastActivityTextChanged);
    connect(this, &Self::lastActivityDetected, this, [this](std::chrono::seconds seconds) {
        emit lastActivityTextChanged(vm::Utils::formattedLastSeenActivity(seconds, m_settings->nowInterval()));
    });
    connect(this, &Self::errorOccured, this, &Self::onErrorOccured);
}

Self::~VSQLastActivityManager()
{
    stopUpdates(true);
}

void Self::setCurrentJid(const QString &jid)
{
    if (m_jid == jid) {
        return;
    }
    m_jid = jid;
    canStart() ? startUpdates(true) : stopUpdates(true);
}

void Self::setEnabled(bool enabled)
{
    if (enabled == m_enabled) {
        return;
    }
    m_enabled = enabled;
    canStart() ? startUpdates(false) : stopUpdates(false);
}

QStringList Self::discoveryFeatures() const
{
    return VSQLastActivityIq::discoveryFeatures();
}

bool Self::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq" && VSQLastActivityIq::isLastActivityId(element)) {
        VSQLastActivityIq lastActivityIq;
        lastActivityIq.parse(element);
        if (m_jid != lastActivityIq.from()) {
            return false;
        }
        if (lastActivityIq.isValid()) {
            emit lastActivityDetected(lastActivityIq.seconds());
        } else if (lastActivityIq.needSubscription()) {
            emit lastActivityMissing(vm::Utils::formattedLastSeenNoActivity());
        } else {
            emit errorOccured(tr("Failed to find last activity"));
        }
        return true;
    }
    return false;
}

void Self::timerEvent(QTimerEvent *)
{
    requestInfo();
}

QString Self::requestInfo()
{
    if (m_jid.isEmpty()) {
        return QString();
    }

    VSQLastActivityIq request;
    request.setType(QXmppIq::Get);
    request.setTo(m_jid);
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        qCWarning(lcLastActivityManager) << "Last activity request failed";
        return QString();
    }
}

bool Self::canStart() const
{
    return !m_jid.isEmpty() && m_enabled;
}

void Self::startUpdates(bool reset)
{
    stopUpdates(reset);
    if (canStart()) {
        requestInfo();
        m_timerId = startTimer(m_settings->nowInterval());
        if (m_timerId == 0) {
            emit errorOccured(tr("Failed to start timer"));
        }
    }
}

void Self::stopUpdates(bool reset)
{
    if (m_timerId == 0) {
        return;
    }
    killTimer(m_timerId);
    m_timerId = 0;

    if (reset) {
        emit lastActivityMissing(QString());
    }
}

void Self::onErrorOccured(const QString &errorText)
{
    qCWarning(lcLastActivityManager) << errorText;
    stopUpdates(true);
}
