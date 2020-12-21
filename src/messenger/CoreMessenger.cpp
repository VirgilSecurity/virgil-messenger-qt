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

#include "CoreMessenger.h"

#include "Utils.h"
#include "CustomerEnv.h"
#include "UserImpl.h"
#include "CommKitBridge.h"
#include "MessageContentJsonUtils.h"
#include "IncomingMessage.h"

#include "VSQNetworkAnalyzer.h"
#include "VSQDiscoveryManager.h"
#include "VSQContactManager.h"
#include "VSQLastActivityManager.h"

#if VS_PUSHNOTIFICATIONS
#include "XmppPushNotifications.h"
using namespace notifications;
using namespace notifications::xmpp;
#endif // VS_PUSHNOTIFICATIONS

#include <virgil/sdk/core/vssc_json_object.h>
#include <virgil/sdk/comm-kit/vssq_messenger.h>
#include <virgil/sdk/comm-kit/vssq_error_message.h>

#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppMessageReceiptManager.h>
#include <qxmpp/QXmppCarbonManager.h>
#include <qxmpp/QXmppUploadRequestManager.h>

#include <QCryptographicHash>
#include <QMap>
#include <QXmlStreamWriter>
#include <QtConcurrent>
#include <QJsonDocument>

#include <memory>

using namespace vm;
using Self = vm::CoreMessenger;

Q_LOGGING_CATEGORY(lcCommKitMessenger, "comm-kit");

// --------------------------------------------------------------------------
// C Helpers.
// --------------------------------------------------------------------------
template<typename CType>
using vsc_unique_ptr = std::unique_ptr<CType, void(*)(const CType*)>;

using vsc_buffer_unique_ptr_t = vsc_unique_ptr<vsc_buffer_t>;
using vssq_messenger_creds_unique_ptr_t = vsc_unique_ptr<vssq_messenger_creds_t>;
using vssq_messenger_unique_ptr_t = vsc_unique_ptr<vssq_messenger_t>;
using vssc_json_object_unique_ptr_t = vsc_unique_ptr<vssc_json_object_t>;


static vsc_buffer_unique_ptr_t vsc_buffer_wrap_ptr(vsc_buffer_t *ptr) {
    return vsc_buffer_unique_ptr_t{ptr, vsc_buffer_delete};
}

static vssq_messenger_creds_unique_ptr_t vssq_messenger_creds_wrap_ptr(vssq_messenger_creds_t *ptr) {
    return vssq_messenger_creds_unique_ptr_t{ptr, vssq_messenger_creds_delete};
}

static vssq_messenger_unique_ptr_t vssq_messenger_wrap_ptr(vssq_messenger_t *ptr) {
    return vssq_messenger_unique_ptr_t{ptr, vssq_messenger_delete};
}

static vssc_json_object_unique_ptr_t vssc_json_object_wrap_ptr(vssc_json_object_t *ptr) {
    return vssc_json_object_unique_ptr_t{ptr, vssc_json_object_delete};
}


// --------------------------------------------------------------------------
// Configuration.
// --------------------------------------------------------------------------
class Self::Impl {
public:
    vssq_messenger_unique_ptr_t messenger = vssq_messenger_wrap_ptr(nullptr);

    VSQNetworkAnalyzer *networkAnalyzer;

    std::unique_ptr<QXmppClient> xmpp;
    std::unique_ptr<VSQDiscoveryManager> discoveryManager;
    std::unique_ptr<VSQContactManager> contactManager;

    QXmppCarbonManager *xmppCarbonManager;
    QXmppUploadRequestManager *xmppUploadManager;
    VSQLastActivityManager *lastActivityManager;

    std::map<QString, std::shared_ptr<User>> identityToUser;
    std::map<QString, std::shared_ptr<User>> usernameToUser;

    bool isActive = true;
};


Self::CoreMessenger(Settings *settings, QObject *parent)
        : QObject(parent), m_impl(std::make_unique<Self::Impl>()), m_settings(settings) {

    //
    // Register QML types.
    //
    qRegisterMetaType<QXmppClient::Error>();
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();

    //
    //  Register self signals-slots
    //
    connect(this, &Self::fireConnectXmppServer, this, &Self::onConnectXmppServer);
    connect(this, &Self::fireReconnectIfNeeded, this, &Self::onReconnectIfNeeded);
    connect(this, &Self::connectionStateChanged, this, &Self::onLogConnectionStateChanged);

    //
    //  Configure Network Analyzer.
    //
    m_impl->networkAnalyzer = new VSQNetworkAnalyzer(nullptr); // will be moved to the thread
    connect(m_impl->networkAnalyzer, &VSQNetworkAnalyzer::connectedChanged, this, &Self::onProcessNetworkState);
}


Self::~CoreMessenger() noexcept = default;


Self::Result
Self::resetCommKitConfiguration() {

    qCDebug(lcCommKitMessenger) << "Reset Comm Kit configuration";

    auto messengerServiceUrl = CustomerEnv::messengerServiceUrl();
    auto xmppServiceUrl = CustomerEnv::xmppServiceUrl();
    auto contactDiscoveryServiceUrl = CustomerEnv::contactDiscoveryServiceUrl();

    qCDebug(lcCommKitMessenger) << "Messenger Service URL: "<< messengerServiceUrl;
    qCDebug(lcCommKitMessenger) << "XMPP Service URL     : "<< xmppServiceUrl;
    qCDebug(lcCommKitMessenger) << "Contact Discovery URL: "<< contactDiscoveryServiceUrl;

    auto messengerServiceUrlStd = messengerServiceUrl.toStdString();
    auto xmppServiceUrlStd = xmppServiceUrl.toStdString();
    auto contactDiscoveryServiceUrlStd = contactDiscoveryServiceUrl.toStdString();

    vssq_messenger_config_t *config = vssq_messenger_config_new_with(
            vsc_str_from(messengerServiceUrlStd),  vsc_str_from(contactDiscoveryServiceUrlStd), vsc_str_from(xmppServiceUrlStd));

    auto caBundlePath = CustomerEnv::caBundlePath().toStdString();
    if (!caBundlePath.empty()) {
        vssq_messenger_config_set_ca_bundle(config, vsc_str_from(caBundlePath));
    }

    m_impl->messenger = vssq_messenger_wrap_ptr(vssq_messenger_new_with_config(config));

    auto status = vssq_messenger_setup_defaults(m_impl->messenger.get());
    vssq_messenger_config_destroy(&config);

    if (status != vssq_status_SUCCESS) {
        qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_status(status));
        return Self::Result::Error_CryptoInit;
    }

    return Self::Result::Success;
}


void
Self::resetXmppConfiguration() {

    qCDebug(lcCommKitMessenger) << "Reset XMPP configuration";

    if (m_impl->xmpp) {
        qCDebug(lcCommKitMessenger) << "Disconnecting from XMPP server...";
        m_impl->xmpp->disconnect();
        m_impl->xmpp->disconnectFromServer();
    }

    m_impl->xmpp = std::make_unique<QXmppClient>();

    // Add receipt messages extension
    m_impl->discoveryManager = std::make_unique<VSQDiscoveryManager>(m_impl->xmpp.get(), this);
    m_impl->contactManager = std::make_unique<VSQContactManager>(m_impl->xmpp.get(), this);

    //  Create & connect extensions.
    m_impl->xmppCarbonManager = new QXmppCarbonManager();
    m_impl->xmppUploadManager = new QXmppUploadRequestManager();
    m_impl->lastActivityManager = new VSQLastActivityManager(m_settings);

    // Parent is implicitly changed to the QXmppClient within addExtension()
    m_impl->xmpp->addExtension(new QXmppMessageReceiptManager());
    m_impl->xmpp->addExtension(m_impl->xmppCarbonManager);
    m_impl->xmpp->addExtension(m_impl->xmppUploadManager);
    m_impl->xmpp->addExtension(m_impl->lastActivityManager);

    // Connect XMPP signals
    connect(m_impl->lastActivityManager, &VSQLastActivityManager::lastActivityTextChanged, this, &Self::lastActivityTextChanged);

    connect(m_impl->xmppUploadManager, &QXmppUploadRequestManager::serviceFoundChanged,
                this, &Self::xmppOnUploadServiceFound);

    connect(m_impl->xmppUploadManager, &QXmppUploadRequestManager::slotReceived,
                this, &Self::xmppOnUploadSlotReceived);

    connect(m_impl->xmppUploadManager, &QXmppUploadRequestManager::requestFailed,
                this, &Self::xmppOnUploadRequestFailed);

    connect(m_impl->xmpp.get(), &QXmppClient::connected, this, &Self::xmppOnConnected);
    connect(m_impl->xmpp.get(), &QXmppClient::disconnected, this, &Self::xmppOnDisconnected);
    connect(m_impl->xmpp.get(), &QXmppClient::stateChanged, this, &Self::xmppOnStateChanged);
    connect(m_impl->xmpp.get(), &QXmppClient::error, this, &Self::xmppOnError);
    connect(m_impl->xmpp.get(), &QXmppClient::presenceReceived, this, &Self::xmppOnPresenceReceived);
    connect(m_impl->xmpp.get(), &QXmppClient::iqReceived, this, &Self::xmppOnIqReceived);
    connect(m_impl->xmpp.get(), &QXmppClient::sslErrors, this, &Self::xmppOnSslErrors);
    connect(m_impl->xmpp.get(), &QXmppClient::messageReceived, this, &Self::xmppOnMessageReceived);

    auto receipt = m_impl->xmpp->findExtension<QXmppMessageReceiptManager>();
    connect(receipt, &QXmppMessageReceiptManager::messageDelivered, this, &Self::xmppOnMessageDelivered);
}


// --------------------------------------------------------------------------
// Statuses.
// --------------------------------------------------------------------------
bool
Self::isOnline() const noexcept {
    return isNetworkOnline() && isXmppConnected();
}


bool
Self::isSignedIn() const noexcept {
    return m_impl->messenger && vssq_messenger_is_authenticated(m_impl->messenger.get());
}


bool
Self::isNetworkOnline() const noexcept {
    return m_impl->networkAnalyzer->isConnected();
}


bool
Self::isXmppConnected() const noexcept {
    return m_impl->xmpp && m_impl->xmpp->state() == QXmppClient::ConnectedState;
}


bool
Self::isXmppConnecting() const noexcept {
    return m_impl->xmpp && m_impl->xmpp->state() == QXmppClient::ConnectingState;
}


// --------------------------------------------------------------------------
// State controls.
// --------------------------------------------------------------------------
void
Self::activate() {
    m_impl->isActive = true;

    if (m_impl->xmpp) {
        m_impl->xmpp->setActive(true);

        QXmppPresence presenceOnline(QXmppPresence::Available);
        presenceOnline.setAvailableStatusType(QXmppPresence::Online);
        m_impl->xmpp->setClientPresence(presenceOnline);
    }

    fireReconnectIfNeeded();
}


void
Self::deactivate() {
    m_impl->isActive = false;

    if (m_impl->xmpp) {
        m_impl->xmpp->setActive(false);

        QXmppPresence presenceAway(QXmppPresence::Available);
        presenceAway.setAvailableStatusType(QXmppPresence::Away);
        m_impl->xmpp->setClientPresence(presenceAway);
    }
}


// --------------------------------------------------------------------------
// User authorization.
// --------------------------------------------------------------------------
QFuture<Self::Result>
Self::signIn(const QString& username) {
    return QtConcurrent::run([this, username = username]() -> Result {
        qCInfo(lcCommKitMessenger) << "Trying to sign in user";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCommKitMessenger) << "Can initialize C CommKit module";
            return result;
        }

        qCInfo(lcCommKitMessenger) << "Load user credentials";
        auto credentialsString = m_settings->userCredential(username).toStdString();
        if (credentialsString.empty()) {
            qCWarning(lcCommKitMessenger) << "User credentials are not found locally";
            return Self::Result::Error_NoCred;
        }

        qCInfo(lcCommKitMessenger) << "Parse user credentials";
        vssq_error_t error;
        vssq_error_reset(&error);
        auto creds = vssq_messenger_creds_wrap_ptr(vssq_messenger_creds_from_json_str(vsc_str_from(credentialsString), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ImportCredentials;
        }

        qCInfo(lcCommKitMessenger) << "Sign in user";
        error.status = vssq_messenger_authenticate(m_impl->messenger.get(), creds.get());

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_Signin;
        }

        emit fireConnectXmppServer();

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::signUp(const QString& username) {
    return QtConcurrent::run([this, username = username.toStdString()]() -> Result {
        qCInfo(lcCommKitMessenger) << "Trying to sign up";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCommKitMessenger) << "Can initialize C CommKit module";
            return result;
        }

        vssq_error_t error;
        vssq_error_reset(&error);
        error.status = vssq_messenger_register(m_impl->messenger.get(), vsc_str_from(username));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_Signup;
        }

        qCInfo(lcCommKitMessenger) << "User has been successfully signed up";

        qCInfo(lcCommKitMessenger) << "Save user credentials";
        auto creds = vssq_messenger_creds(m_impl->messenger.get());
        auto credsJson = vssc_json_object_wrap_ptr(vssq_messenger_creds_to_json(creds, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ExportCredentials;
        }

        auto credentials = vsc_str_to_qstring(vssc_json_object_as_str(credsJson.get()));
        m_settings->setUserCredential(QString::fromStdString(username), credentials);

        emit fireConnectXmppServer();

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::backupKey(const QString &password) {
    return QtConcurrent::run([this, password = password.toStdString()]() -> Result {
        qCInfo(lcCommKitMessenger) <<  "Upload current user key to the cloud";

        const vssq_status_t status = vssq_messenger_backup_creds(m_impl->messenger.get(), vsc_str_from(password));
        if (vssq_status_SUCCESS != status) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_status(status));
            return Self::Result::Error_MakeKeyBackup;
        }

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::signInWithBackupKey(const QString& username, const QString& password) {
    return QtConcurrent::run([this, username = username.toStdString(),  password = password.toStdString()]() -> Result {
        qCInfo(lcCommKitMessenger) <<  "Load user key from the cloud";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCommKitMessenger) << "Can initialize C CommKit module";
            return result;
        }

        const vssq_status_t status = vssq_messenger_authenticate_with_backup_creds(m_impl->messenger.get(), vsc_str_from(username), vsc_str_from(password));

        if (vssq_status_SUCCESS != status) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_status(status));
            return Self::Result::Error_RestoreKeyBackup;
        }

        qCInfo(lcCommKitMessenger) << "Save user credentials";
        vssq_error_t error;
        vssq_error_reset(&error);

        auto creds = vssq_messenger_creds(m_impl->messenger.get());
        auto credsJson = vssc_json_object_wrap_ptr(vssq_messenger_creds_to_json(creds, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ExportCredentials;
        }

        auto credentials = vsc_str_to_qstring(vssc_json_object_as_str(credsJson.get()));
        m_settings->setUserCredential(QString::fromStdString(username), credentials);

        emit fireConnectXmppServer();

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::signOut() {
    return QtConcurrent::run([this]() -> Result {
        qCInfo(lcCommKitMessenger) << "Signing out";

        if (m_impl->messenger) {
            m_impl->messenger = nullptr;
        }

        if (m_impl->xmpp) {
            // TODO: Discuss: do we really need to unregister from the push notifications?
            this->deregisterFromNotifications();

            m_impl->xmpp->disconnect();
            m_impl->xmpp->disconnectFromServer();
            m_impl->xmpp = nullptr;
        }


        return Self::Result::Success;
    });
}


QString
Self::currentUserJid() const {
    auto user = vssq_messenger_user(m_impl->messenger.get());
    vsc_str_t userIdentity = vssq_messenger_user_identity(user);
    return vsc_str_to_qstring(userIdentity) + "@" + CustomerEnv::xmppServiceUrl() + "/" + m_settings->deviceId();
}


UserId
Self::userIdFromJid(const QString& jid) const {
    return UserId(jid.split("@").first());
}


QString
Self::userIdToJid(const UserId& userId) const {
    return userId + "@" + CustomerEnv::xmppServiceUrl();
}


bool
Self::registerForNotifications() {
#if VS_PUSHNOTIFICATIONS
    if (!isOnline()) {
        qCWarning(lcCommKitMessenger) << "Can not subscribe for push notifications, no connection. Will try it later.";
        return false;
    }

    qCInfo(lcCommKitMessenger) << "Register for push notifications on XMPP server.";

    auto xmppPush = XmppPushNotifications::instance().buildEnableIq();

    xmppPush.setNode(currentUserJid());

#ifdef QT_DEBUG
    QString xml;
    QXmlStreamWriter xmlWriter(&xml);
    xmlWriter.setAutoFormatting(true);
    xmppPush.toXml(&xmlWriter);
    qCDebug(lcCommKitMessenger).noquote() << "Subscribe XMPP request:" << xml;
#endif

    const bool sentStatus = m_impl->xmpp->sendPacket(xmppPush);

    qCInfo(lcCommKitMessenger) << "Register for push notifications on XMPP server status: " << sentStatus;

    return sentStatus;
#else
    return true;
#endif // VS_PUSHNOTIFICATIONS
}


bool
Self::deregisterFromNotifications() {
#if VS_PUSHNOTIFICATIONS

    if (!isOnline()) {
        qCWarning(lcCommKitMessenger) << "Can not unsubscribe from the push notifications, no connection.";
        return false;
    }

    auto xmppPush = XmppPushNotifications::instance().buildDisableIq();

    xmppPush.setNode(currentUserJid());

#ifdef QT_DEBUG
    QString xml;
    QXmlStreamWriter xmlWriter(&xml);
    xmlWriter.setAutoFormatting(true);
    xmppPush.toXml(&xmlWriter);
    qCDebug(lcCommKitMessenger).noquote() << "Unsubscribe XMPP request:" << xml;
#endif

    const bool sentStatus = m_impl->xmpp->sendPacket(xmppPush);

    qCDebug(lcCommKitMessenger) << "Unsubscribe from push notifications status: " << (sentStatus ? "success" : "failed");

    return sentStatus;
#else
    return true;
#endif // VS_PUSHNOTIFICATIONS
}


bool
Self::subscribeToUser(const User &user) {

    auto userJid = userIdToJid(user.id());

    return m_impl->contactManager->addContact(userJid, user.id(), QString());
}


QUrl
Self::getCrashReportEndpointUrl() const {
    auto urlStr = CustomerEnv::messengerServiceUrl() + "/send-logs";

    return QUrl(urlStr);
}


QString
Self::getAuthHeaderVaue() const {

    qCInfo(lcCommKitMessenger) << "Generate auth header for the Messenger Backend Service";

    vssq_error_t error;
    vssq_error_reset(&error);

    const vssq_messenger_auth_t *auth = vssq_messenger_auth(m_impl->messenger.get());
    vssc_http_header_t *auth_header = vssq_messenger_auth_generate_messenger_auth_header(auth, &error);

    if (vssq_error_has_error(&error)) {
        qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return QString();
    }

    auto authHeaderValue = vsc_str_to_qstring(vssc_http_header_value(auth_header));

    vssc_http_header_destroy(&auth_header);

    qCDebug(lcCommKitMessenger) << "Auth header for the Messenger Backend Service: " << authHeaderValue;

    return authHeaderValue;
}


void
Self::onConnectXmppServer() {
    //
    //  We need to reset XMPP before a new connection.
    //  It prevents XMPP Client being in the "disconnecting" state, or other.
    //
    resetXmppConfiguration();

    vssq_error_t error;
    vssq_error_reset(&error);

    const vssq_messenger_auth_t *auth = vssq_messenger_auth(m_impl->messenger.get());
    const vssq_ejabberd_jwt_t *jwt = vssq_messenger_auth_ejabberd_jwt(auth, &error);

    if (vssq_error_has_error(&error)) {
        qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        emit connectionStateChanged(Self::ConnectionState::Disconnected);
        return;
    }

    QString xmppPass = vsc_str_to_qstring(vssq_ejabberd_jwt_as_string(jwt));

    QXmppConfiguration config{};
    config.setJid(currentUserJid());
    config.setHost(CustomerEnv::xmppServiceUrl());
    config.setPassword(xmppPass);
    config.setAutoReconnectionEnabled(false);
    config.setAutoAcceptSubscriptions(true);
#if VS_ANDROID
    config.setKeepAliveInterval(kKeepAliveTimeSec);
    config.setKeepAliveTimeout(kKeepAliveTimeSec - 1);
#endif


// #if USE_XMPP_LOGS
    auto logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);
    logger->setMessageTypes(QXmppLogger::AnyMessage);

    connect(logger, &QXmppLogger::message, [=](QXmppLogger::MessageType, const QString &text){
        qDebug() << text;
    });

    m_impl->xmpp->setLogger(logger);
// #endif

    qCDebug(lcCommKitMessenger) << "Connecting to XMPP server...";
    m_impl->xmpp->connectToServer(config);
}

void
Self::onReconnectIfNeeded() {
    if (isSignedIn() && isNetworkOnline() && !isXmppConnected() && !isXmppConnecting()) {
        emit fireConnectXmppServer();
    }
}

// --------------------------------------------------------------------------
// Find users.
// --------------------------------------------------------------------------
std::shared_ptr<User>
Self::findUserByUsername(const QString &username) const {
    qDebug(lcCommKitMessenger) << "Trying to find user with username: " << username;

    //
    //  Check cache first.
    //
    auto userIt = m_impl->usernameToUser.find(username);
    if (userIt != m_impl->usernameToUser.end()) {
        qDebug(lcCommKitMessenger) << "User found in the cache";
        return userIt->second;
    }

    //
    //  Search on-line.
    //
    if (!isOnline()) {
        return nullptr;
    }

    vssq_error_t error;
    vssq_error_reset(&error);

    auto usernameStdStr = username.toStdString();

    auto user = vssq_messenger_find_user_with_username(m_impl->messenger.get(), vsc_str_from(usernameStdStr), &error);

    if (vssq_error_has_error(&error)) {
        qDebug(lcCommKitMessenger) << "User not found";
        qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return nullptr;
    }

    qDebug(lcCommKitMessenger) << "User found in the cloud";

    //
    //  Cache and return.
    //
    auto commKitUserImpl = std::make_unique<UserImpl>(user);
    auto commKitUser = std::make_shared<User>(std::move(commKitUserImpl));

    m_impl->usernameToUser[username] = commKitUser;
    m_impl->identityToUser[commKitUser->id()] = commKitUser;

    return commKitUser;
}


std::shared_ptr<User>
Self::findUserById(const UserId &userId) const {
    qDebug(lcCommKitMessenger) << "Trying to find user with id: " << userId;

    //
    //  Check cache first.
    //
    auto userIt = m_impl->identityToUser.find(userId);
    if (userIt != m_impl->identityToUser.end()) {
        qDebug(lcCommKitMessenger) << "User found in the cache";
        return userIt->second;
    }

    //
    //  Search on-line.
    //
    if (!isOnline()) {
        return nullptr;
    }

    vssq_error_t error;
    vssq_error_reset(&error);

    auto userIdStdStr = QString(userId).toStdString();

    auto user = vssq_messenger_find_user_with_identity(m_impl->messenger.get(), vsc_str_from(userIdStdStr), &error);

    if (vssq_error_has_error(&error)) {
        qDebug(lcCommKitMessenger) << "User not found";
        qCWarning(lcCommKitMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return nullptr;
    }

    qDebug(lcCommKitMessenger) << "User found in the cloud";

    //
    //  Cache and return.
    //
    auto commKitUserImpl = std::make_unique<UserImpl>(user);
    auto commKitUser = std::make_shared<User>(std::move(commKitUserImpl));

    m_impl->identityToUser[userId] = commKitUser;
    m_impl->usernameToUser[userId] = commKitUser;

    return commKitUser;
}


std::shared_ptr<User>
Self::currentUser() const {
    if (!isSignedIn()) {
        return nullptr;
    }

    auto user = vssq_messenger_user(m_impl->messenger.get());

    auto commKitUserImpl = std::make_unique<UserImpl>(user);

    auto commKitUser = std::make_shared<User>(std::move(commKitUserImpl));

    return commKitUser;
}


// --------------------------------------------------------------------------
//  Message handling.
// --------------------------------------------------------------------------
QFuture<Self::Result>
Self::sendMessage(MessageHandler message) {

    return QtConcurrent::run([this, message = std::move(message)]() -> Result {
        qCInfo(lcCommKitMessenger) << "Trying to send message";

        if (!isOnline()) {
            qCInfo(lcCommKitMessenger) << "Trying to send message when offline";
            return Self::Result::Error_Offline;
        }

        //
        //  Find recipient.
        //
        auto recipient = findUserByUsername(message->recipientId());
        if (!recipient) {
            //
            //  Got network troubles to find recipient, so cache message and try later.
            //
            qCWarning(lcCommKitMessenger) << "Can not send message - recipient is not found";
            return Self::Result::Error_UserNotFound;
        }

        //
        //  Encrypt message content.
        //
        auto contentJson = MessageContentJsonUtils::toBytes(message->content());
        auto ciphertextDataMinLen = vssq_messenger_encrypted_message_len(m_impl->messenger.get(), contentJson.size(), recipient->impl()->user.get());

        qCDebug(lcCommKitMessenger) << "Message Len   : " << contentJson.size();
        qCDebug(lcCommKitMessenger) << "ciphertext Len: " << ciphertextDataMinLen;

        QByteArray ciphertextData(ciphertextDataMinLen, 0x00);

        auto ciphertext = vsc_buffer_wrap_ptr(vsc_buffer_new());
        vsc_buffer_use(ciphertext.get(),  (byte *)ciphertextData.data(), ciphertextData.size());

        const vssq_status_t encryptionStatus = vssq_messenger_encrypt_data(
                    m_impl->messenger.get(), vsc_data_from(contentJson), recipient->impl()->user.get(), ciphertext.get());

        if (encryptionStatus != vssq_status_SUCCESS) {
            qCWarning(lcCommKitMessenger) << "Can not encrypt ciphertext: " << vsc_str_to_qstring(vssq_error_message_from_status(encryptionStatus));
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        ciphertextData.resize(vsc_buffer_len(ciphertext.get()));

        //
        //  Pack JSON body
        //
        QJsonObject ciphertextJson;
        ciphertextJson.insert("version", "v3");
        ciphertextJson.insert("timestamp", static_cast<qint64>(message->createdAt().toTime_t()));
        ciphertextJson.insert("ciphertext", QString::fromLatin1(ciphertextData.toBase64()));
        auto ciphertextJsonStr = QJsonDocument(ciphertextJson).toJson(QJsonDocument::Compact);

        qCDebug(lcCommKitMessenger) << "Will send XMPP message with body: " << ciphertextJsonStr;

        // TODO: review next lines when implement group chats.
        auto senderJid = userIdToJid(message->senderId());
        auto recipientJid = userIdToJid(UserId(message->chatId()));

        QXmppMessage xmppMessage(senderJid, recipientJid, ciphertextJsonStr);
        xmppMessage.setStamp(message->createdAt());
        xmppMessage.setType(QXmppMessage::Type::Chat);

        //
        //  Send.
        //
        bool isSent = m_impl->xmpp->sendPacket(xmppMessage);
        if (isSent) {
            qCDebug(lcCommKitMessenger) << "XMPP message was sent";
            return Self::Result::Success;
        } else {
            qCWarning(lcCommKitMessenger) << "Can not send message - XMPP send failed";
            return Self::Result::Error_SendMessageFailed;
        }
    });

}


QFuture<Self::Result>
Self::processReceivedXmppMessage(const QXmppMessage& xmppMessage) {

    return QtConcurrent::run([this, xmppMessage = xmppMessage]() -> Result {
        qCInfo(lcCommKitMessenger) << "Trying to sign in user";

        auto message = std::make_unique<IncomingMessage>();

        // TODO: review next lines when implement group chats.
        message->setId(MessageId(xmppMessage.id()));
        message->setSenderId(userIdFromJid(xmppMessage.from()));
        message->setChatId(ChatId(userIdFromJid(xmppMessage.to())));
        message->setCreatedAt(xmppMessage.stamp());

        //
        //  Unpack JSON body.
        //
        auto messageBodyJson = QJsonDocument::fromJson(xmppMessage.body().toUtf8());
        if (messageBodyJson.isNull()) {
            qCWarning(lcCommKitMessenger) << "Got invalid XMPP message - body (not JSON)";
            return Self::Result::Error_InvalidMessageFormat;
        }

        auto version = messageBodyJson["version"].toString();
        if (version != "v3") {
            qCWarning(lcCommKitMessenger) << "Got invalid XMPP message - unsupported version, expected v3, got " << version;
            return Self::Result::Error_InvalidMessageVersion;
        }

        auto ciphertextBase64 = messageBodyJson["ciphertext"].toString();
        if (ciphertextBase64.isEmpty()) {
            qCWarning(lcCommKitMessenger) << "Got invalid XMPP message - empty ciphertext";
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        auto ciphertextResult = QByteArray::fromBase64Encoding(ciphertextBase64.toLatin1());
        if (!ciphertextResult) {
            qCWarning(lcCommKitMessenger) << "Got invalid XMPP message - ciphertext is not base64 encoded";
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        auto ciphertext = *ciphertextResult;

        //
        //  Find sender.
        //
        auto sender = findUserById(message->senderId());
        if (!sender) {
            //
            //  Got network troubles to find sender, so cache message and try later.
            //
            qCWarning(lcCommKitMessenger) << "Can not decrypt ciphertext - sender is not found";
            message->setContent(MessageContentEncrypted(ciphertext));
            emit messageReceived(std::move(message));
            return Self::Result::Success;
        }

        //
        //  Decrypt message.
        //
        auto plaintextDataMinLen = vssq_messenger_decrypted_message_len(m_impl->messenger.get(), ciphertext.size());
        QByteArray plaintextData(plaintextDataMinLen, 0x00);

        auto plaintext = vsc_buffer_wrap_ptr(vsc_buffer_new());
        vsc_buffer_use(plaintext.get(),  (byte *)plaintextData.data(), plaintextData.size());

        const vssq_status_t decryptionStatus = vssq_messenger_decrypt_data(
                    m_impl->messenger.get(), vsc_data_from(ciphertext), sender->impl()->user.get(), plaintext.get());

        if (decryptionStatus != vssq_status_SUCCESS) {
            qCWarning(lcCommKitMessenger) << "Can not decrypt ciphertext: " << vsc_str_to_qstring(vssq_error_message_from_status(decryptionStatus));
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        plaintextData.resize(vsc_buffer_len(plaintext.get()));

        //
        //  Parse content.
        //
        QString errorString;
        auto content = MessageContentJsonUtils::fromBytes(plaintextData, errorString);

        if (std::get_if<std::monostate>(&content)) {
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setContent(std::move(content));

        //
        //  Tell the world we got a message.
        //
        emit messageReceived(std::move(message));

        return Self::Result::Success;
    });
}


QXmppMessage
Self::createXmppMessageToSend() {

}


// --------------------------------------------------------------------------
//  XMPP event handlers.
// --------------------------------------------------------------------------
void
Self::xmppOnConnected() {
    m_impl->lastActivityManager->setEnabled(true);

    registerForNotifications();

    emit connectionStateChanged(Self::ConnectionState::Connected);
}


void
Self::xmppOnDisconnected() {
    m_impl->lastActivityManager->setEnabled(false);

    emit connectionStateChanged(Self::ConnectionState::Disconnected);
}


void
Self::xmppOnStateChanged(QXmppClient::State state) {
    if (QXmppClient::ConnectingState == state) {
        emit connectionStateChanged(Self::ConnectionState::Connecting);
    }
}


void
Self::xmppOnError(QXmppClient::Error error) {
    qCWarning(lcCommKitMessenger) << "XMPP error: " << error;
    emit connectionStateChanged(Self::ConnectionState::Error);
}


void
Self::xmppOnPresenceReceived(const QXmppPresence &presence) {

}


void
Self::xmppOnIqReceived(const QXmppIq &iq) {

}

void
Self::xmppOnSslErrors(const QList<QSslError> &errors) {
    qCWarning(lcCommKitMessenger) << "XMPP SSL errors: " << errors;
    emit connectionStateChanged(Self::ConnectionState::Error);
}


void
Self::xmppOnMessageReceived(const QXmppMessage &xmppMessage) {
    //
    //  TODO: handle result.
    //
    processReceivedXmppMessage(xmppMessage);
}


void
Self::xmppOnMessageDelivered(const QString &jid, const QString &messageId) {
    qCDebug(lcCommKitMessenger) << "Message delivered to: " << jid;
//    emit updateMessage(MessageContentUpdateDelivered(messageId));
}


// --------------------------------------------------------------------------
//  Network event handlers.
// --------------------------------------------------------------------------
void
Self::onProcessNetworkState(bool isOnline) {
    if (isSignedIn() && isOnline && !this->isOnline()) {
        emit fireConnectXmppServer();
    }
}


// --------------------------------------------------------------------------
//  LastActivityManager: controls and events.
// --------------------------------------------------------------------------
void
Self::setCurrentRecipient(const QString& recipientId) {

    //  FIXME: now username is passed, but should be user identity.
    if (!recipientId.isEmpty()) {
        auto user = findUserByUsername(recipientId);
        if (user) {
            m_impl->lastActivityManager->setCurrentJid(userIdToJid(user->id()));
            return;
        }
    }

    m_impl->lastActivityManager->setCurrentJid(QString());
}


// --------------------------------------------------------------------------
//  File upload: controls and events.
// --------------------------------------------------------------------------
bool
Self::isUploadServiceFound() const {
    return m_impl->xmppUploadManager->serviceFound();
}


QString
Self::requestUploadSlot(const QString &filePath) {
    return m_impl->xmppUploadManager->requestUploadSlot(QFileInfo(filePath));
}


void
Self::xmppOnUploadServiceFound() {

    const auto found = m_impl->xmppUploadManager->serviceFound();
    qCDebug(lcCommKitMessenger) << "Upload service found: " << found;
    emit uploadServiceFound(found);
}

void Self::xmppOnUploadSlotReceived(const QXmppHttpUploadSlotIq &slot) {
    emit uploadSlotReceived(slot.id(), slot.putUrl(), slot.getUrl());
}


void Self::xmppOnUploadRequestFailed(const QXmppHttpUploadRequestIq &request) {

    const auto error = request.error();

    qCDebug(lcCommKitMessenger) << QString("code(%1), condition(%2), text(%3)")
                             .arg(error.code()).arg(error.condition()).arg(error.text());

    if (error.condition() == QXmppStanza::Error::Condition::NotAcceptable && error.code() == 406) {
        emit uploadSlotErrorOccurred(request.id(), tr("File is larger than limit"));
    }
    else {
        emit uploadSlotErrorOccurred(request.id(), tr("Fail to upload file"));
    }
}


// --------------------------------------------------------------------------
//  Handle self events for debug purposes.
// --------------------------------------------------------------------------
void
Self::onLogConnectionStateChanged(CoreMessenger::ConnectionState state) {
    switch(state) {
        case Self::ConnectionState::Offline:
            qCDebug(lcCommKitMessenger) << "New connection status: offline";
            break;

        case Self::ConnectionState::Disconnected:
            qCDebug(lcCommKitMessenger) << "New connection status: disconnected";
            break;

        case Self::ConnectionState::Connecting:
            qCDebug(lcCommKitMessenger) << "New connection status: connecting";
            break;

        case Self::ConnectionState::Connected:
            qCDebug(lcCommKitMessenger) << "New connection status: connected";
            break;

        case Self::ConnectionState::Error:
            qCDebug(lcCommKitMessenger) << "New connection status: error";
            break;
    }
}
