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
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#include "NetworkAnalyzer.h"

#include <QDateTime>
#include <QDebug>
#include <QNetworkSession>
#include <QThread>
#include <QTimer>
#include <QRegularExpression>
#include <QNetworkSession>
#include <QLoggingCategory>

using namespace vm;
using Self = NetworkAnalyzer;

#if !defined(DEBUG_NETWORK)
#    define DEBUG_NETWORK 0
#endif

Q_LOGGING_CATEGORY(lcNetwork, "network");

#if VS_MACOS || VS_LINUX
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

Self::NetworkAnalyzer(QObject *parent) : QObject(parent), m_nwManager(this), m_isConnected(false)
{
    // To disable issue: QNetworkAccessManager shows "Network access is disabled."
    qputenv("QT_BEARER_POLL_TIMEOUT", QByteArray::number(-1));

    m_thread = new QThread(this);
    moveToThread(m_thread);
    m_timer.moveToThread(m_thread);

    connect(&m_nwManager, SIGNAL(updateCompleted()), this, SLOT(onUpdateCompleted()));
    connect(m_thread, SIGNAL(started()), this, SLOT(onStart()));

    m_thread->start();
}

Self::~NetworkAnalyzer()
{
    // Stop execuion of Self::onAnalyzeNetwork() if it's running
    m_thread->requestInterruption();

    // Do quit in correct thread
    QMetaObject::invokeMethod(m_thread, "quit", Qt::QueuedConnection);

    // Wait for termination
    m_thread->wait(2000);

    // Hard stop if it's still running
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

void Self::onStart()
{
    m_timer.setInterval(kTimerInterval);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), &m_nwManager, SLOT(updateConfigurations()));

    onUpdateCompleted();
}

bool Self::isConnected() const noexcept
{
    return m_isConnected;
}

void Self::onUpdateCompleted()
{
    onAnalyzeNetwork();

    m_timer.start();
};

bool Self::checkIsNeedStop()
{
    bool needStop = QThread::currentThread()->isInterruptionRequested();
    if (needStop) {
        m_timer.stop();
    }
    return needStop;
}

void Self::onAnalyzeNetwork()
{
    bool currentState = false;
    static bool initialized = false;
    bool stateChanged = false;

    VSQNetworkInterfaceData currenNetworkInterfaceData;

#if !VS_ANDROID
    QList<QNetworkConfiguration> networkConfigurations = m_nwManager.allConfigurations();
    for (const QNetworkConfiguration &configuration : networkConfigurations) {

        if (checkIsNeedStop()) {
            return;
        }

        QNetworkSession session(configuration, this);
        session.open();
        session.waitForOpened(kSessionTimeoutMs);

        if (!session.isOpen()) {
            continue;
        }

        if (!configuration.isValid()) {
            qCDebug(lcNetwork).noquote() << "NetworkAnalyzer: Network configuration is not valid, skipped...";
            continue;
        }

        printConfiguration(configuration);
        printSession(session);

        if (session.state() == QNetworkSession::Connected) {
            QNetworkInterface networkInterface = session.interface();

            if (networkInterface.isValid()) {
                printNetworkInterface(networkInterface);

                QString ipV4Address;
                auto networkAddresses = networkInterface.allAddresses();
                for (const auto &entry : networkAddresses) {
                    if (entry.isLoopback() || entry.isBroadcast() || entry.isLinkLocal()
                        || entry.protocol() != QAbstractSocket::IPv4Protocol) {
                        continue;
                    }
                    ipV4Address = entry.toString();
                    break;
                }

#    if DEBUG_NETWORK
                qCDebug(lcNetwork).noquote().nospace() << "NetworkAnalyzer: QNetworkAddressEntry, Ip: " << ipV4Address;
#    endif
                if (!ipV4Address.isEmpty()) {
                    currenNetworkInterfaceData.insert(networkInterface.index(), ipV4Address);
                }
            }
        }

        if (session.isOpen() && (session.state() == QNetworkSession::Connected)) {
            currentState = true;
        }
    }

#else
    auto networkAddresses = QNetworkInterface::allAddresses();
    currentState = true;
    int i = 0;
    for (const auto &entry : networkAddresses) {
        if (entry.isLoopback() || entry.isBroadcast() || entry.isLinkLocal()
            || entry.protocol() != QAbstractSocket::IPv4Protocol) {
            continue;
        }
        QString ipV4Address = entry.toString();
#    if DEBUG_NETWORK
        qCDebug(lcNetwork).noquote().nospace() << "NetworkAnalyzer: QNetworkAddressEntry, Ip: " << ipV4Address;
#    endif
        if (!ipV4Address.isEmpty()) {
            currenNetworkInterfaceData.insert(i++, ipV4Address);
        }
    }

#endif

    if (currenNetworkInterfaceData.isEmpty()) {
        qCDebug(lcNetwork).noquote().nospace() << "NetworkAnalyzer: Network is not ready";
        stateChanged = true;
        emit connectedChanged(false);
        m_networkInterfaceData.clear();
        checkIsNeedStop();
        return;
    }

    if ((!initialized) || (m_isConnected != currentState)) {
        m_isConnected = currentState;
        initialized = true;

        stateChanged = true;
        emit connectedChanged(m_isConnected);

        qCDebug(lcNetwork).noquote().nospace() << "NetworkAnalyzer: Online status: " << m_isConnected;
    }

#if DEBUG_NETWORK
    qCDebug(lcNetwork) << "currenNetworkInterfaceData : " << currenNetworkInterfaceData;
    qCDebug(lcNetwork) << "m_networkInterfaceData : " << m_networkInterfaceData;
#endif

    if (currenNetworkInterfaceData != m_networkInterfaceData) {

        qCDebug(lcNetwork).noquote().nospace() << "NetworkAnalyzer: Network changed";

        printMap(m_networkInterfaceData);
        printMap(currenNetworkInterfaceData);

        m_networkInterfaceData = currenNetworkInterfaceData;
        stateChanged = true;
        if (stateChanged) {
            emit connectedChanged(m_isConnected);
        }
    }

    if (!stateChanged && m_isConnected) {
        emit heartBeat();
    }

    checkIsNeedStop();
}

void Self::printNetworkInterface(const QNetworkInterface &interface) const
{
#if DEBUG_NETWORK
    qCDebug(lcNetwork).noquote().nospace()
            << QString("\tIndex: %1, HW address: %2, name: %3, valid: %4, type: %5, flags: %6")
                       .arg(interface.index())
                       .arg(interface.hardwareAddress())
                       .arg(interface.humanReadableName())
                       .arg(interface.isValid())
                       .arg(interface.type())
                       .arg(interface.flags());
#else
    Q_UNUSED(interface)
#endif
}

void Self::printMap(const VSQNetworkInterfaceData &nid) const
{
#if DEBUG_NETWORK
    QMapIterator<int, QString> i(nid);
    while (i.hasNext()) {
        i.next();
        qCDebug(lcNetwork).noquote() << "VSQNetworkInterfaceData: Key: " << i.key() << ", value: " << i.value();
    }
#else
    Q_UNUSED(nid)
#endif
}

void Self::printSession(const QNetworkSession &session) const
{
#if DEBUG_NETWORK
    bool sessionConnected = session.state() == QNetworkSession::Connected ? true : false;

    qCDebug(lcNetwork).noquote().nospace()
            << QString("NetworkAnalyzer: Network session, identifier: %1 (%2, %3), activeTime: "
                       "%4, error: %5, state: %6")
                       .arg(session.sessionProperty("ActiveConfiguration").toString())
                       .arg((session.isOpen() ? "Opened" : "Closed"))
                       .arg((sessionConnected ? "Connected" : "Not connected"))
                       .arg(session.activeTime())
                       .arg(session.errorString())
                       .arg(session.state());
#else
    Q_UNUSED(session)
#endif
}

void Self::printConfiguration(const QNetworkConfiguration &configuration) const
{
#if DEBUG_NETWORK
    qCDebug(lcNetwork).noquote().nospace()
            << QString("NetworkAnalyzer: Network configuration: identifier: %1, name: %2, bearerTypeName: %3, valid: "
                       "%4, timeout: %5, roaming: %6, state: %7, type: %8, children: %9")
                       .arg(configuration.identifier())
                       .arg(configuration.name())
                       .arg(configuration.bearerTypeName())
                       .arg(configuration.isValid())
                       .arg(configuration.connectTimeout())
                       .arg(configuration.isRoamingAvailable())
                       .arg(configuration.state())
                       .arg(configuration.type())
                       .arg(configuration.children().size());
#else
    Q_UNUSED(configuration)
#endif
}

#if VS_MACOS || VS_LINUX
#    pragma GCC diagnostic pop
#endif
