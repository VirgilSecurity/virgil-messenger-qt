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

#include "client/VSQClient.h"

#include <QDeadlineTimer>
#include <QEventLoop>
#include <QSslSocket>
#include <QTimer>

#include <QXmppCarbonManager.h>
#include <QXmppConfiguration.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppLogger.h>
#include <QXmppMessage.h>
#include <QXmppMessageReceiptManager.h>
#include <QXmppPushEnableIq.h>

#include "VSQPushNotifications.h"
#include "VSQUtils.h"

#ifndef USE_XMPP_LOGS
#define USE_XMPP_LOGS 1
#endif

#if VS_ANDROID
const int kKeepAliveTimeSec = 10;
#endif
const int kConnectionWaitMs = 10000;

// TODO(fpohtmeh): refactor
const QString kPushNotificationsProxy = "push-notifications-proxy";
const QString kPushNotificationsNode = "node";
const QString kPushNotificationsService = "service";
const QString kPushNotificationsFCM = "fcm";
const QString kPushNotificationsDeviceID = "device_id";
const QString kPushNotificationsFormType = "FORM_TYPE";
const QString kPushNotificationsFormTypeVal = "http://jabber.org/protocol/pubsub#publish-options";

Q_LOGGING_CATEGORY(lcClient, "client")
Q_LOGGING_CATEGORY(lcXmpp, "xmpp")

VSQClient::VSQClient(VSQSettings *settings, QObject *parent)
    : QObject(parent)
    , m_core(settings)
    , m_client(this)
    , m_uploader(&m_client, this)
    , m_lastErrorText()
    , m_waitingForConnection(false)
    , m_needReconnection(false)
{}

VSQClient::~VSQClient()
{
#ifdef VS_DEVMODE
    qCDebug(lcDev) << "~Client";
#endif
}

void VSQClient::start()
{
    // Sign-in connections
    connect(this, &VSQClient::signIn, this, &VSQClient::onSignIn);
    connect(this, &VSQClient::signOut, this, &VSQClient::onSignOut);
    connect(this, &VSQClient::signUp, this, &VSQClient::onSignUp);
    connect(this, &VSQClient::backupKey, this, &VSQClient::onBackupKey);
    connect(this, &VSQClient::signInWithKey, this, &VSQClient::onSignInWithKey);
    connect(this, &VSQClient::signedIn, this, &VSQClient::updateVirgilUrl);
    // XMPP connections
    connect(&m_client, &QXmppClient::connected, this, &VSQClient::onConnected);
    connect(&m_client, &QXmppClient::disconnected, this, &VSQClient::onDisconnected);
    connect(&m_client, &QXmppClient::error, this, &VSQClient::onError);
    connect(&m_client, &QXmppClient::messageReceived, this, &VSQClient::onMessageReceived);
    connect(&m_client, &QXmppClient::presenceReceived, this, &VSQClient::onPresenceReceived);
    connect(&m_client, &QXmppClient::iqReceived, this, &VSQClient::onIqReceived);
    connect(&m_client, &QXmppClient::stateChanged, this, &VSQClient::onStateChanged);
    connect(&m_client, &QXmppClient::sslErrors, this, &VSQClient::onSslErrors);
    // Other connections
    connect(this, &VSQClient::addContact, this, &VSQClient::onAddContact);
    connect(this, &VSQClient::sendMessage, this, &VSQClient::onSendMessage);
    connect(this, &VSQClient::checkConnectionState, this, &VSQClient::onCheckConnectionState);
    connect(this, &VSQClient::setOnlineStatus, this, &VSQClient::onSetOnlineStatus);

    // XMPP receipt manager
    auto receiptManager = new QXmppMessageReceiptManager();
    receiptManager->setParent(this);
    m_client.addExtension(receiptManager);
    connect(receiptManager, &QXmppMessageReceiptManager::messageDelivered, this, &VSQClient::onMessageDelivered);
    // XMPP carbon manager
    m_carbonManager = new QXmppCarbonManager();
    m_carbonManager->setParent(this);
    m_client.addExtension(m_carbonManager);
    // messages sent to our account (forwarded from another client)
    connect(m_carbonManager, &QXmppCarbonManager::messageReceived, &m_client, &QXmppClient::messageReceived);
    // messages sent from our account (but another client)
    connect(m_carbonManager, &QXmppCarbonManager::messageSent, &m_client, &QXmppClient::messageReceived);
    // XMPP discovery manager
    // TODO(fpohtmeh): remove discover routines
    //auto discoveryManager = new QXmppDiscoveryManager();
    //connect(discoveryManager, &QXmppDiscoveryManager::infoReceived, this, &Client::onDiscoveryInfoReceived);

    // XMPP logger
    auto logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);
    logger->setMessageTypes(QXmppLogger::AnyMessage);
#if USE_XMPP_LOGS
    connect(logger, &QXmppLogger::message, this, &VSQClient::onXmppLoggerMessage);
    m_client.setLogger(logger);
#endif

    // Wait for connection connections
    connect(&m_client, &QXmppClient::connected, this, &VSQClient::stopWaitForConnection);
    connect(&m_client, &QXmppClient::disconnected, this, &VSQClient::stopWaitForConnection);
    connect(&m_client, &QXmppClient::error, this, &VSQClient::stopWaitForConnection);

    // Uploading: uploader-to-client
    connect(&m_uploader, &VSQUploader::messageSent, this, &VSQClient::messageSent);
    connect(&m_uploader, &VSQUploader::sendMessageFailed, this, &VSQClient::sendMessageFailed);
    connect(&m_uploader, &VSQUploader::uploadStarted, this, &VSQClient::uploadStarted);
    connect(&m_uploader, &VSQUploader::uploadProgressChanged, this, &VSQClient::uploadProgressChanged);
    connect(&m_uploader, &VSQUploader::uploaded, this, &VSQClient::uploaded);
    connect(&m_uploader, &VSQUploader::uploadFailed, this, &VSQClient::uploadFailed);
}

bool VSQClient::xmppConnect()
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
    qCDebug(lcClient) << "SSL:" << QSslSocket::supportsSsl();
    qCDebug(lcClient) << ">>>> Connecting...";

    // Activate reconnections during wait
    bool needReconnectionPrev;
    needReconnectionPrev = m_needReconnection;
    m_needReconnection = true;

    // Start waiting for connection
    m_waitingForConnection = true;

    // Start connection
    m_client.connectToServer(config);

    // Wait until connected or timed out
    waitForConnection();

    // Stop waiting for connection
    m_waitingForConnection = false;

    // TODO: Is it correct ?
    m_carbonManager->setCarbonsEnabled(true);

    // Set up need in reconnections
    if (m_client.isConnected()) {
        m_needReconnection = true;
    } else {
         m_needReconnection = needReconnectionPrev;
    }

    return m_client.isConnected();
}

void VSQClient::xmppDisconnect()
{
    if (!m_client.isConnected()) {
        qCDebug(lcClient) << "Client is already disconnected";
    }
    else {
        m_client.disconnectFromServer();
        waitForConnection();
    }
}

void VSQClient::xmppReconnect()
{
    if (!m_core.isSignedIn())
        qWarning() << "User is not signed in";
    else if (m_client.isConnected())
        qWarning() << "Client is already connected. Reconnect was skipped";
    else
        xmppConnect();
}

void VSQClient::waitForConnection()
{
    QDeadlineTimer timer(kConnectionWaitMs);
    QEventLoop loop;
    do {
        loop.processEvents();
    }
    while (m_waitingForConnection && !timer.hasExpired());
}

void VSQClient::stopWaitForConnection()
{
    m_waitingForConnection = false;
}

void VSQClient::subscribeOnPushNotifications(bool enable)
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

Optional<ExtMessage> VSQClient::createExtMessage(const Message &message)
{
    auto encryptedBody = m_core.encryptMessageBody(message.contact, message.body);
    if (!encryptedBody) {
        m_lastErrorText = m_core.lastErrorText();
        return NullOptional;
    }

    QXmppMessage xmppMessage;
    xmppMessage.setFrom(m_core.xmppJID());
    xmppMessage.setTo(message.contact + QLatin1Char('@') + m_core.xmppURL());
    xmppMessage.setBody(*encryptedBody);
    xmppMessage.setId(message.id);
    xmppMessage.setReceiptRequested(true);

    ExtMessage extMessage(message);
    extMessage.xmpp = xmppMessage;
    return extMessage;
}

void VSQClient::onSignIn(const QString &userWithEnv)
{
    if (!m_core.signIn(userWithEnv))
        emit signInFailed(userWithEnv, m_core.lastErrorText());
    else if (!xmppConnect())
        emit signInFailed(userWithEnv, m_lastErrorText);
    else
        emit signedIn(userWithEnv);
}

void VSQClient::onSignOut()
{
    subscribeOnPushNotifications(false);
    xmppDisconnect();
    m_core.signOut();
    emit signedOut();
}

void VSQClient::onSignUp(const QString &userWithEnv)
{
    if (m_core.signUp(userWithEnv))
        emit signedUp(userWithEnv);
    else
        emit signUpFailed(userWithEnv, m_core.lastErrorText());
}

void VSQClient::onBackupKey(const QString &password)
{
    if (m_core.backupKey(password))
        emit keyBackuped(password);
    else
        emit backupKeyFailed(password, m_core.lastErrorText());
}

void VSQClient::onSignInWithKey(const QString &user, const QString &password)
{
    if (!m_core.signInWithKey(user, password))
        emit signInFailed(user, m_core.lastErrorText());
    else if (!xmppConnect())
        emit signInFailed(user, m_lastErrorText);
    else
        emit signedIn(user);
}

void VSQClient::onAddContact(const QString &contact)
{
    if (m_core.userExists(contact))
        emit contactAdded(contact);
    else
        emit addContactFailed(contact, QString("Contact %1 doesn't exist").arg(contact));
}

void VSQClient::onConnected()
{
    qCDebug(lcClient) << "onConnected";
    subscribeOnPushNotifications(true); // TODO(fpohtmeh): why subscription is here?
    // TODO(fpohtmeh): send all messages with status failed
}

void VSQClient::onDisconnected()
{
    qCDebug(lcClient) << "onDisconnected state:" << m_client.state();
}

void VSQClient::onError(QXmppClient::Error error)
{
    qCDebug(lcClient) << "onError:" << error << "state:" << m_client.state();

    if (!m_needReconnection) {
        qCDebug(lcClient) << "Do not try to reconnect because there is no need in it.";
    }

    if (m_waitingForConnection) {
        qCDebug(lcClient) << "Delayed check of connection because we're connecting now.";
        QTimer::singleShot(1000, this, &VSQClient::checkConnectionState);
    } else {
        qCDebug(lcClient) << "Attempt to reconnect";
        xmppReconnect();
    }
}

void VSQClient::onMessageReceived(const QXmppMessage &message)
{
    QString sender = message.from().split("@").first();
    QString recipient = message.to().split("@").first();
    qCInfo(lcClient) << "Sender:" << sender << " Recipient:" << recipient;

    // Get encrypted message
    QString encryptedBody = message.body();
    // Decrypt message
    const auto body = m_core.decryptMessageBody(sender, encryptedBody);
    if (!body) {
        emit receiveMessageFailed(m_core.lastErrorText());
        return;
    }

    Message msg;
    msg.id = VSQUtils::createUuid();
    msg.timestamp = QDateTime::currentDateTime();
    msg.body = *body;
    // FIXME(fpohtmeh): get attachment from xmpp message?
    if (sender == m_core.user()) {
        // Message from self sent from another device
        msg.contact = recipient;
        msg.author = Message::Author::User;
        msg.status = Message::Status::Sent;
    }
    else {
        // Receive message from other user
        msg.contact = sender;
        msg.author = Message::Author::Contact;
        msg.status = Message::Status::Received;
    }
    emit messageReceived(msg);
}

void VSQClient::onPresenceReceived(const QXmppPresence &presence)
{
    // TODO(fpohtmeh): remove?
    Q_UNUSED(presence)
}

void VSQClient::onIqReceived(const QXmppIq &iq)
{
    // TODO(fpohtmeh): remove?
    Q_UNUSED(iq)
}

void VSQClient::onStateChanged(QXmppClient::State state)
{
    // TODO(fpohtmeh): remove?
    Q_UNUSED(state)
}

void VSQClient::onSendMessage(const Message &message)
{
    auto msg = createExtMessage(message);
    if (!msg) {
        emit sendMessageFailed(message, m_core.lastErrorText());
        return;
    }
    if (message.attachment)
        m_uploader.upload(*msg);
    else if (m_client.sendPacket(msg->xmpp))
        emit messageSent(message);
    else
        emit sendMessageFailed(message, QLatin1String("Message sending failed"));
}

void VSQClient::onCheckConnectionState()
{
    if (m_client.state() == QXmppClient::DisconnectedState) {
#if VS_ANDROID
        qCWarning(lcClient) << "Disconnected ...";
#endif
        xmppReconnect();
    }
}

void VSQClient::onSetOnlineStatus(bool online)
{
    const auto presenceType = online ? QXmppPresence::Available : QXmppPresence::Unavailable;
    m_client.setClientPresence(QXmppPresence(presenceType));
}

void VSQClient::onMessageDelivered(const QString &jid, const QString &messageId)
{
    qCDebug(lcClient) << QString("Message with id: %1 delivered to %2").arg(jid, messageId);
    emit messageDelivered(messageId);
}

void VSQClient::onDiscoveryInfoReceived(const QXmppDiscoveryIq &info)
{
    qCInfo(lcClient) << info.features();
    if (info.from() != m_client.configuration().domain())
        return;
    // enable carbons, if feature found
    if (info.features().contains("urn:xmpp:carbons:2"))
        m_carbonManager->setCarbonsEnabled(true);
}

void VSQClient::onXmppLoggerMessage(QXmppLogger::MessageType type, const QString &message)
{
    if (type == QXmppLogger::WarningMessage)
        qCWarning(lcXmpp).noquote() << message;
    else
        qCDebug(lcXmpp).noquote() << message;
}

void VSQClient::onSslErrors(const QList<QSslError> &errors)
{
    for (auto &error : errors)
        qCWarning(lcClient) << "SSL error:" << error;
    m_lastErrorText = QLatin1String("SSL connection errors...");
    xmppDisconnect();
}

void VSQClient::updateVirgilUrl()
{
    const auto url = m_core.virgilURL();
    if (url != m_virgilUrl) {
        m_virgilUrl = url;
        emit virgilUrlChanged(url);
    }
}
