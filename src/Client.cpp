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

#include "Client.h"

#include <QDeadlineTimer>
#include <QEventLoop>
#include <QSslSocket>

#include <QXmppConfiguration.h>
#include <QXmppLogger.h>
#include <QXmppMessage.h>
#include <QXmppMessageReceiptManager.h>
#include <QXmppPushEnableIq.h>

#include "Utils.h"

#ifndef USE_XMPP_LOGS
#define USE_XMPP_LOGS 1
#endif

#if VS_ANDROID
const int kKeepAliveTimeSec = 10;
#endif
const int kConnectionWaitMs = 10000;

#include "PushNotifications.h"

// TODO(fpohtmeh): refactor
const QString kPushNotificationsProxy = "push-notifications-proxy";
const QString kPushNotificationsNode = "node";
const QString kPushNotificationsService = "service";
const QString kPushNotificationsFCM = "fcm";
const QString kPushNotificationsDeviceID = "device_id";
const QString kPushNotificationsFormType = "FORM_TYPE";
const QString kPushNotificationsFormTypeVal = "http://jabber.org/protocol/pubsub#publish-options";

Client::Client(Settings *settings, QObject *parent)
    : QObject(parent)
    , m_core(settings)
    , m_client(this)
    , m_lastErrorText()
    , m_waitingForConnection(false)
{}

void Client::start()
{
    // Register types
    qRegisterMetaType<QAbstractSocket::SocketState>();
    // Sign-in connections
    connect(this, &Client::signIn, this, &Client::onSignIn);
    connect(this, &Client::signOut, this, &Client::onSignOut);
    connect(this, &Client::signUp, this, &Client::onSignUp);
    connect(this, &Client::backupKey, this, &Client::onBackupKey);
    connect(this, &Client::signInWithKey, this, &Client::onSignInWithKey);
    // XMPP signals
    connect(&m_client, &QXmppClient::connected, this, &Client::onConnected);
    connect(&m_client, &QXmppClient::disconnected, this, &Client::onDisconnected);
    connect(&m_client, &QXmppClient::error, this, &Client::onError);
    connect(&m_client, &QXmppClient::messageReceived, this, &Client::onMessageReceived);
    connect(&m_client, &QXmppClient::presenceReceived, this, &Client::onPresenceReceived);
    connect(&m_client, &QXmppClient::iqReceived, this, &Client::onIqReceived);
    connect(&m_client, &QXmppClient::stateChanged, this, &Client::onStateChanged);
    connect(&m_client, &QXmppClient::sslErrors, this, &Client::onSslErrors);
    // Other signals
    connect(this, &Client::addContact, this, &Client::onAddContact);
    connect(this, &Client::sendMessage, this, &Client::onSendMessage);
    connect(this, &Client::checkConnectionState, this, &Client::onCheckConnectionState);
    connect(this, &Client::setOnlineStatus, this, &Client::onSetOnlineStatus);

    // XMPP receipt manager
    auto receiptManager = new QXmppMessageReceiptManager();
    m_client.addExtension(receiptManager);
    connect(receiptManager, &QXmppMessageReceiptManager::messageDelivered, this, &Client::onMessageDelivered);
    // XMPP logger
    auto logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);
    logger->setMessageTypes(QXmppLogger::AnyMessage);
#if USE_XMPP_LOGS
    // FIXME(fpohtmeh): restore
    //connect(logger, &QXmppLogger::message, this, &Client::onXmppLoggerMessage);
    m_client.setLogger(logger);
#endif

    // Connection timer
    connect(&m_client, &QXmppClient::connected, this, &Client::stopWaitForConnection);
    connect(&m_client, &QXmppClient::disconnected, this, &Client::stopWaitForConnection);
    connect(&m_client, &QXmppClient::error, this, &Client::stopWaitForConnection);
}

bool Client::xmppConnect()
{
    const auto password = m_core.xmppPassword();
    if (!password) {
        m_lastErrorText = m_core.lastErrorText();
        return false;
    }

    QXmppConfiguration config;
    config.setJid(m_core.xmppJID());
    config.setHost(m_core.xmppURL());
    config.setPassword(*password);
    config.setAutoReconnectionEnabled(false);
#if VS_ANDROID
    config.setKeepAliveInterval(kKeepAliveTimeSec);
    config.setKeepAliveTimeout(kKeepAliveTimeSec - 1);
#endif
    qDebug() << QLatin1String("SSL: ") << QSslSocket::supportsSsl();
    qDebug() << QLatin1String(">>>> Connecting...");
    m_client.connectToServer(config);
    waitForConnection();
    return m_client.isConnected();
}

void Client::xmppDisconnect()
{
    if (!m_client.isConnected()) {
        qDebug() << "Client is already disconnected";
    }
    else {
        m_client.disconnectFromServer();
        waitForConnection();
    }
}

void Client::xmppReconnect()
{
    if (!m_core.isSignedIn())
        qWarning() << "User is not signed in";
    else if (m_client.isConnected())
        qWarning() << "Client is already connected. Reconnect was skipped";
    else
        xmppConnect();
}

void Client::waitForConnection()
{
    QDeadlineTimer timer(kConnectionWaitMs);
    QEventLoop loop;
    m_waitingForConnection = true;
    do {
        loop.processEvents();
    }
    while (m_waitingForConnection && !timer.hasExpired());
    m_waitingForConnection = false;
}

void Client::stopWaitForConnection()
{
    m_waitingForConnection = false;
}

void Client::subscribeOnPushNotifications(bool enable)
{
#if VS_PUSHNOTIFICATIONS
    // Subscribe Form Type
    QXmppDataForm::Field subscribeFormType;
    subscribeFormType.setKey(kPushNotificationsFormType);
    subscribeFormType.setValue(kPushNotificationsFormTypeVal);

    // Subscribe service
    QXmppDataForm::Field subscribeService;
    subscribeService.setKey(kPushNotificationsService);
    subscribeService.setValue(kPushNotificationsFCM);

    // Subscribe device
    QXmppDataForm::Field subscribeDevice;
    subscribeDevice.setKey(kPushNotificationsDeviceID);
    subscribeDevice.setValue(VSQPushNotifications::instance().token());

    // Create a Data Form
    QList<QXmppDataForm::Field> fields;
    fields << subscribeFormType << subscribeService << subscribeDevice;

    QXmppDataForm dataForm;
    dataForm.setType(QXmppDataForm::Submit);
    dataForm.setFields(fields);

    // Create request
    QXmppPushEnableIq xmppPush;
    xmppPush.setType(QXmppIq::Set);
    xmppPush.setMode(enable ? QXmppPushEnableIq::Enable : QXmppPushEnableIq::Disable);
    xmppPush.setJid(kPushNotificationsProxy);
    xmppPush.setNode(kPushNotificationsNode);
    xmppPush.setDataForm(dataForm);

    m_client.sendPacket(xmppPush);
#else
    Q_UNUSED(enable)
#endif // VS_PUSHNOTIFICATIONS
}

void Client::onSignIn(const QString &userWithEnv)
{
    if (!m_core.signIn(userWithEnv))
        emit signInFailed(userWithEnv, m_core.lastErrorText());
    if (!xmppConnect())
        emit signInFailed(userWithEnv, m_lastErrorText);
    emit signedIn(userWithEnv);
}

void Client::onSignOut()
{
    subscribeOnPushNotifications(false);
    xmppDisconnect();
    m_core.signOut();
    emit signedOut();
}

void Client::onSignUp(const QString &userWithEnv)
{
    if (m_core.signUp(userWithEnv))
        emit signedUp(userWithEnv);
    else
        emit signUpFailed(userWithEnv, m_core.lastErrorText());
}

void Client::onBackupKey(const QString &password)
{
    if (m_core.backupKey(password))
        emit keyBackuped(password);
    else
        emit backupKeyFailed(password, m_core.lastErrorText());
}

void Client::onSignInWithKey(const QString &user, const QString &password)
{
    if (!m_core.signInWithKey(user, password))
        emit signInFailed(user, m_core.lastErrorText());
    if (!xmppConnect())
        emit signInFailed(user, m_lastErrorText);
    emit signedIn(user);
}

void Client::onAddContact(const QString &contact)
{
    if (m_core.userExists(contact))
        emit contactAdded(contact);
    else
        emit addContactFailed(contact, QString("Contact %1 doesn't exist").arg(contact));
}

void Client::onConnected()
{
    VS_LOG_DEBUG("onConnected");
    qDebug() << "onConnected";
    subscribeOnPushNotifications(true); // TODO(fpohtmeh): why subscription is here?
    // TODO(fpohtmeh): send all messages with status failed
}

void Client::onDisconnected()
{
    VS_LOG_DEBUG("onDisconnected");
    qDebug() << "onDisconnected state:" << m_client.state();
}

void Client::onError(QXmppClient::Error error)
{
    VS_LOG_DEBUG("onError");
    qDebug() << "onError:" << error << "state:" << m_client.state();
    xmppReconnect();
}

void Client::onMessageReceived(const QXmppMessage &message)
{
    // FIXME(fpohtmeh): restore
    //qDebug() << "Message received:" << message.from() << message.body();
    // Get sender
    QString from = message.from();
    QStringList pieces = from.split("@");
    if (pieces.size() < 1) {
        VS_LOG_WARNING("Wrong sender");
        return;
    }
    QString sender = pieces.first();
    // Get encrypted message
    QString encryptedBody = message.body();
    // Decrypt message
    const auto body = m_core.decryptMessage(sender, encryptedBody);
    if (!body) {
        emit receiveMessageFailed(m_core.lastErrorText());
        return;
    }
    // Build message
    Message msg;
    msg.id = Utils::createUuid();
    msg.timestamp = QDateTime::currentDateTime();
    msg.body = *body;
    msg.contact = sender;
    msg.author = Message::Author::Contact;
    // FIXME(fpohtmeh): get attachment
    msg.status = Message::Status::Received;
    //
    emit messageReceived(msg);
}

void Client::onPresenceReceived(const QXmppPresence &presence)
{
    // TODO(fpohtmeh): remove?
    Q_UNUSED(presence)
}

void Client::onIqReceived(const QXmppIq &iq)
{
    // TODO(fpohtmeh): remove?
    Q_UNUSED(iq)
}

void Client::onStateChanged(QXmppClient::State state)
{
    // TODO(fpohtmeh): remove?
    Q_UNUSED(state)
}

void Client::onSendMessage(const Message &message)
{
    auto msg = m_core.encryptMessage(message);
    if (!msg)
        emit sendMessageFailed(message, m_core.lastErrorText());
    else if (m_client.sendPacket(*msg))
        emit messageSent(message);
    else
        emit sendMessageFailed(message, QLatin1String("Message sending failed"));
}

void Client::onCheckConnectionState()
{
    if (m_client.state() == QXmppClient::DisconnectedState) {
#if VS_ANDROID
        // TODO(fpohtmeh): remove this warning
        qWarning() << QLatin1String("Disconnected ...");
#endif
        xmppReconnect();
    }
}

void Client::onSetOnlineStatus(bool online)
{
    const auto presenceType = online ? QXmppPresence::Available : QXmppPresence::Unavailable;
    m_client.setClientPresence(QXmppPresence(presenceType));
}

void Client::onMessageDelivered(const QString &jid, const QString &messageId)
{
    // FIXME(fpohtmeh): restore
    Q_UNUSED(jid);
    //qDebug() << QString("Message with id: %1 delivered to %2").arg(jid, messageId);
    emit messageDelivered(messageId);
}

void Client::onXmppLoggerMessage(QXmppLogger::MessageType type, const QString &message)
{
    const auto msg = QString("XMPP log: %1").arg(message);
    if (type == QXmppLogger::WarningMessage)
        qWarning().noquote() << msg;
    else
        qDebug().noquote() << msg;
}

void Client::onSslErrors(const QList<QSslError> &errors)
{
    for (auto &error : errors)
        qWarning() << QLatin1String("SSL error:") << error;
    m_lastErrorText = QLatin1String("SSL connection errors...");
    xmppDisconnect();
}
