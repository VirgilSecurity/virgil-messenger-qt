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
#include "OutgoingMessage.h"

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
#include <virgil/sdk/comm-kit/vssq_messenger_file_cipher.h>

#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppMessageReceiptManager.h>
#include <qxmpp/QXmppCarbonManager.h>
#include <qxmpp/QXmppUploadRequestManager.h>

#include <QCryptographicHash>
#include <QMap>
#include <QXmlStreamWriter>
#include <QtConcurrent>
#include <QJsonDocument>
#include <QLoggingCategory>

#include <memory>

using namespace vm;
using Self = vm::CoreMessenger;

Q_LOGGING_CATEGORY(lcCoreMessenger, "core-messenger");

// --------------------------------------------------------------------------
// C Helpers.
// --------------------------------------------------------------------------
template<typename CType>
using vsc_unique_ptr = std::unique_ptr<CType, void(*)(const CType*)>;

using vsc_buffer_unique_ptr_t = vsc_unique_ptr<vsc_buffer_t>;
using vssc_json_object_unique_ptr_t = vsc_unique_ptr<vssc_json_object_t>;
using vssq_messenger_creds_unique_ptr_t = vsc_unique_ptr<vssq_messenger_creds_t>;
using vssq_messenger_unique_ptr_t = vsc_unique_ptr<vssq_messenger_t>;
using vssq_messenger_file_cipher_ptr_t = vsc_unique_ptr<vssq_messenger_file_cipher_t>;


static vsc_buffer_unique_ptr_t vsc_buffer_wrap_ptr(vsc_buffer_t *ptr) {
    return vsc_buffer_unique_ptr_t{ptr, vsc_buffer_delete};
}

static vssc_json_object_unique_ptr_t vssc_json_object_wrap_ptr(vssc_json_object_t *ptr) {
    return vssc_json_object_unique_ptr_t{ptr, vssc_json_object_delete};
}

static vssq_messenger_creds_unique_ptr_t vssq_messenger_creds_wrap_ptr(vssq_messenger_creds_t *ptr) {
    return vssq_messenger_creds_unique_ptr_t{ptr, vssq_messenger_creds_delete};
}

static vssq_messenger_unique_ptr_t vssq_messenger_wrap_ptr(vssq_messenger_t *ptr) {
    return vssq_messenger_unique_ptr_t{ptr, vssq_messenger_delete};
}

static vssq_messenger_file_cipher_ptr_t vssq_messenger_file_cipher_wrap(vssq_messenger_file_cipher_t *ptr) {
    return vssq_messenger_file_cipher_ptr_t{ptr, vssq_messenger_file_cipher_delete};
}

static std::tuple<QByteArray, vsc_buffer_unique_ptr_t> makeMappedBuffer(size_t size) {

    QByteArray byteArray(size, 0x00);
    auto buffer  = vsc_buffer_wrap_ptr(vsc_buffer_new());
    vsc_buffer_use(buffer.get(),  (byte *)byteArray.data(), byteArray.size());

    return std::make_tuple(std::move(byteArray), std::move(buffer));
}

static void adjustMappedBuffer(const vsc_buffer_unique_ptr_t& buffer, QByteArray& bytes) {

    bytes.resize(vsc_buffer_len(buffer.get()));
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

    ConnectionState connectionState = ConnectionState::Disconnected;

    bool suspended = false;
};


Self::CoreMessenger(Settings *settings, QObject *parent)
        : QObject(parent), m_impl(std::make_unique<Self::Impl>()), m_settings(settings) {

    //
    // Register QML types.
    //
    qRegisterMetaType<QXmppClient::Error>();
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();

    qRegisterMetaType<vm::MessageHandler>("MessageHandler");
    qRegisterMetaType<vm::ModifiableMessageHandler>("ModifiableMessageHandler");
    qRegisterMetaType<vm::Messages>("Messages");
    qRegisterMetaType<vm::ModifiableMessages>("ModifiableMessages");
    qRegisterMetaType<vm::UserHandler>("UserHandler");
    qRegisterMetaType<vm::ChatHandler>("ChatHandler");
    qRegisterMetaType<vm::ModifiableChatHandler>("ModifiableChatHandler");
    qRegisterMetaType<vm::ModifiableChats>("ModifiableChats");
    qRegisterMetaType<vm::CloudFileHandler>("CloudFileHandler");
    qRegisterMetaType<vm::ModifiableCloudFileHandler>("ModifiableCloudFileHandler");
    qRegisterMetaType<vm::CloudFiles>("CloudFiles");
    qRegisterMetaType<vm::ModifiableCloudFiles>("ModifiableCloudFiles");

    qRegisterMetaType<vm::ChatId>("ChatId");
    qRegisterMetaType<vm::MessageId>("MessageId");
    qRegisterMetaType<vm::AttachmentId>("AttachmentId");
    qRegisterMetaType<vm::CloudFileId>("CloudFileId");

    //
    //  Register self signals-slots
    //
    connect(this, &Self::activate, this, &Self::onActivate);
    connect(this, &Self::deactivate, this, &Self::onDeactivate);
    connect(this, &Self::suspend, this, &Self::onSuspend);
    connect(this, &Self::connectionStateChanged, this, &Self::onLogConnectionStateChanged);

    connect(this, &Self::reconnectXmppServerIfNeeded, this, &Self::onReconnectXmppServerIfNeeded);
    connect(this, &Self::disconnectXmppServer, this, &Self::onDisconnectXmppServer);
    connect(this, &Self::cleanupCommKitMessenger, this, &Self::onCleanupCommKitMessenger);
    connect(this, &Self::registerPushNotifications, this, &Self::onRegisterPushNotifications);
    connect(this, &Self::deregisterPushNotifications, this, &Self::onDeregisterPushNotifications);

    //
    //  Configure Network Analyzer.
    //
    m_impl->networkAnalyzer = new VSQNetworkAnalyzer(nullptr); // will be moved to the thread
    connect(m_impl->networkAnalyzer, &VSQNetworkAnalyzer::connectedChanged, this, &Self::onProcessNetworkState);
}


Self::~CoreMessenger() noexcept = default;


Self::Result
Self::resetCommKitConfiguration() {

    qCDebug(lcCoreMessenger) << "Reset Comm Kit configuration";

    auto messengerServiceUrl = CustomerEnv::messengerServiceUrl();
    auto xmppServiceUrl = CustomerEnv::xmppServiceUrl();
    auto contactDiscoveryServiceUrl = CustomerEnv::contactDiscoveryServiceUrl();

    qCDebug(lcCoreMessenger) << "Messenger Service URL: "<< messengerServiceUrl;
    qCDebug(lcCoreMessenger) << "XMPP Service URL     : "<< xmppServiceUrl;
    qCDebug(lcCoreMessenger) << "Contact Discovery URL: "<< contactDiscoveryServiceUrl;

    auto messengerServiceUrlStd = messengerServiceUrl.toStdString();
    auto xmppServiceUrlStd = xmppServiceUrl.toStdString();
    auto contactDiscoveryServiceUrlStd = contactDiscoveryServiceUrl.toStdString();

    vssq_messenger_config_t *config = vssq_messenger_config_new_with(
            vsc_str_from(messengerServiceUrlStd), vsc_str_from(contactDiscoveryServiceUrlStd), vsc_str_from(xmppServiceUrlStd));

    auto caBundlePath = CustomerEnv::caBundlePath().toStdString();
    if (!caBundlePath.empty()) {
        vssq_messenger_config_set_ca_bundle(config, vsc_str_from(caBundlePath));
    }

    m_impl->messenger = vssq_messenger_wrap_ptr(vssq_messenger_new_with_config(config));

    auto status = vssq_messenger_setup_defaults(m_impl->messenger.get());
    vssq_messenger_config_destroy(&config);

    if (status != vssq_status_SUCCESS) {
        qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_status(status));
        return Self::Result::Error_CryptoInit;
    }

    return Self::Result::Success;
}


void
Self::resetXmppConfiguration() {

    qCDebug(lcCoreMessenger) << "Reset XMPP configuration";

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

    //
    //  Handle carbons (message copies).
    //  See, XEP-0280.
    //
    connect(m_impl->xmppCarbonManager, &QXmppCarbonManager::messageReceived, this, &Self::xmppOnMessageReceived);
    connect(m_impl->xmppCarbonManager, &QXmppCarbonManager::messageSent, this, &Self::xmppOnCarbonMessageReceived);

    // Add extra logging
#if USE_XMPP_LOGS
    auto logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);
    logger->setMessageTypes(QXmppLogger::AnyMessage);

    connect(logger, &QXmppLogger::message, [](QXmppLogger::MessageType, const QString &text){
        qCDebug(lcCoreMessenger) << text;
    });

    m_impl->xmpp->setLogger(logger);
#endif

    auto receipt = m_impl->xmpp->findExtension<QXmppMessageReceiptManager>();
    connect(receipt, &QXmppMessageReceiptManager::messageDelivered, this, &Self::xmppOnMessageDelivered);
}


// --------------------------------------------------------------------------
// Statuses.
// --------------------------------------------------------------------------
bool
Self::isOnline() const noexcept {
    return isSignedIn() && isNetworkOnline() && isXmppConnected();
}


bool
Self::isSignedIn() const noexcept {
    return m_impl->messenger && vssq_messenger_is_authenticated(m_impl->messenger.get());
}


CoreMessenger::ConnectionState Self::connectionState() const {
    return m_impl->connectionState;
}


bool
Self::isNetworkOnline() const noexcept {
    return m_impl->networkAnalyzer->isConnected();
}


bool
Self::isXmppConnected() const noexcept {
    if (m_impl->xmpp) {
        return  m_impl->xmpp->state() == QXmppClient::ConnectedState;
    }

    return false;
}


bool
Self::isXmppConnecting() const noexcept {
    if (m_impl->xmpp) {
        return m_impl->xmpp->state() == QXmppClient::ConnectingState;
    }

    return false;
}

bool
Self::isXmppDisconnected() const noexcept {
    if (m_impl->xmpp) {
        return m_impl->xmpp->state() == QXmppClient::DisconnectedState;
    }

    return true;
}


// --------------------------------------------------------------------------
// State controls.
// --------------------------------------------------------------------------
void
Self::onActivate() {
    if (m_impl->xmpp) {
        m_impl->xmpp->setActive(true);

        QXmppPresence presenceOnline(QXmppPresence::Available);
        presenceOnline.setAvailableStatusType(QXmppPresence::Online);
        m_impl->xmpp->setClientPresence(presenceOnline);
    }

    m_impl->suspended = false;

    reconnectXmppServerIfNeeded();
}


void
Self::onDeactivate() {
    if (m_impl->xmpp) {
        m_impl->xmpp->setActive(false);

        QXmppPresence presenceAway(QXmppPresence::Available);
        presenceAway.setAvailableStatusType(QXmppPresence::Away);
        m_impl->xmpp->setClientPresence(presenceAway);
    }
}


void
Self::onSuspend() {
    if (m_impl->xmpp) {
        m_impl->xmpp->disconnectFromServer();
    }
    m_impl->suspended = true;
}


// --------------------------------------------------------------------------
// User authorization.
// --------------------------------------------------------------------------
QFuture<Self::Result>
Self::signIn(const QString& username) {
    return QtConcurrent::run([this, username = username]() -> Result {
        qCInfo(lcCoreMessenger) << "Trying to sign in user";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCoreMessenger) << "Can initialize C CommKit module";
            return result;
        }

        qCInfo(lcCoreMessenger) << "Load user credentials";
        auto credentialsString = m_settings->userCredential(username).toStdString();
        if (credentialsString.empty()) {
            qCWarning(lcCoreMessenger) << "User credentials are not found locally";
            return Self::Result::Error_NoCred;
        }

        qCInfo(lcCoreMessenger) << "Parse user credentials";
        vssq_error_t error;
        vssq_error_reset(&error);
        auto creds = vssq_messenger_creds_wrap_ptr(vssq_messenger_creds_from_json_str(vsc_str_from(credentialsString), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ImportCredentials;
        }

        qCInfo(lcCoreMessenger) << "Sign in user";
        error.status = vssq_messenger_authenticate(m_impl->messenger.get(), creds.get());

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_Signin;
        }

        emit reconnectXmppServerIfNeeded();

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::signUp(const QString& username) {
    return QtConcurrent::run([this, username = username.toStdString()]() -> Result {
        qCInfo(lcCoreMessenger) << "Trying to sign up";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCoreMessenger) << "Can initialize C CommKit module";
            return result;
        }

        vssq_error_t error;
        vssq_error_reset(&error);
        error.status = vssq_messenger_register(m_impl->messenger.get(), vsc_str_from(username));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_Signup;
        }

        qCInfo(lcCoreMessenger) << "User has been successfully signed up";

        qCInfo(lcCoreMessenger) << "Save user credentials";
        auto creds = vssq_messenger_creds(m_impl->messenger.get());
        auto credsJson = vssc_json_object_wrap_ptr(vssq_messenger_creds_to_json(creds, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ExportCredentials;
        }

        auto credentials = vsc_str_to_qstring(vssc_json_object_as_str(credsJson.get()));
        m_settings->setUserCredential(QString::fromStdString(username), credentials);

        emit reconnectXmppServerIfNeeded();

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::backupKey(const QString &password) {
    return QtConcurrent::run([this, password = password.toStdString()]() -> Result {
        qCInfo(lcCoreMessenger) <<  "Upload current user key to the cloud";

        const vssq_status_t status = vssq_messenger_backup_creds(m_impl->messenger.get(), vsc_str_from(password));
        if (vssq_status_SUCCESS != status) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_status(status));
            return Self::Result::Error_MakeKeyBackup;
        }

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::signInWithBackupKey(const QString& username, const QString& password) {
    return QtConcurrent::run([this, username = username.toStdString(),  password = password.toStdString()]() -> Result {
        qCInfo(lcCoreMessenger) <<  "Load user key from the cloud";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCoreMessenger) << "Can initialize C CommKit module";
            return result;
        }

        const vssq_status_t status = vssq_messenger_authenticate_with_backup_creds(m_impl->messenger.get(), vsc_str_from(username), vsc_str_from(password));

        if (vssq_status_SUCCESS != status) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_status(status));
            return Self::Result::Error_RestoreKeyBackup;
        }

        qCInfo(lcCoreMessenger) << "Save user credentials";
        vssq_error_t error;
        vssq_error_reset(&error);

        auto creds = vssq_messenger_creds(m_impl->messenger.get());
        auto credsJson = vssc_json_object_wrap_ptr(vssq_messenger_creds_to_json(creds, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ExportCredentials;
        }

        auto credentials = vsc_str_to_qstring(vssc_json_object_as_str(credsJson.get()));
        m_settings->setUserCredential(QString::fromStdString(username), credentials);

        emit reconnectXmppServerIfNeeded();

        return Self::Result::Success;
    });
}


QFuture<Self::Result>
Self::signOut() {
    return QtConcurrent::run([this]() -> Result {
        qCInfo(lcCoreMessenger) << "Signing out";

        emit deregisterPushNotifications();
        emit cleanupCommKitMessenger();
        emit disconnectXmppServer();

        return Self::Result::Success;
    });
}


QString
Self::currentUserJid() const {
    auto user = vssq_messenger_user(m_impl->messenger.get());
    vsc_str_t userIdentity = vssq_messenger_user_identity(user);
    return vsc_str_to_qstring(userIdentity) + "@" + CustomerEnv::xmppServiceDomain() + "/" + m_settings->deviceId();
}


UserId
Self::userIdFromJid(const QString& jid) const {
    return UserId(jid.split("@").first());
}


QString
Self::userIdToJid(const UserId& userId) const {
    return userId + "@" + CustomerEnv::xmppServiceDomain();
}


void
Self::onRegisterPushNotifications() {
#if VS_PUSHNOTIFICATIONS
    if (!isOnline()) {
        qCWarning(lcCoreMessenger) << "Can not subscribe for push notifications, no connection. Will try it later.";
        return;
    }

    qCInfo(lcCoreMessenger) << "Register for push notifications on XMPP server.";

    auto xmppPush = XmppPushNotifications::instance().buildEnableIq();

    xmppPush.setNode(currentUserJid());

#ifdef QT_DEBUG
    QString xml;
    QXmlStreamWriter xmlWriter(&xml);
    xmlWriter.setAutoFormatting(true);
    xmppPush.toXml(&xmlWriter);
    qCDebug(lcCoreMessenger).noquote() << "Subscribe XMPP request:" << xml;
#endif

    const bool sentStatus = m_impl->xmpp->sendPacket(xmppPush);

    qCInfo(lcCoreMessenger) << "Register for push notifications on XMPP server status: " << sentStatus;

#endif // VS_PUSHNOTIFICATIONS
}


void
Self::onDeregisterPushNotifications() {
#if VS_PUSHNOTIFICATIONS
    if (!isOnline()) {
        qCWarning(lcCoreMessenger) << "Can not unsubscribe from the push notifications, no connection.";
    }

    auto xmppPush = XmppPushNotifications::instance().buildDisableIq();

    xmppPush.setNode(currentUserJid());

#ifdef QT_DEBUG
    QString xml;
    QXmlStreamWriter xmlWriter(&xml);
    xmlWriter.setAutoFormatting(true);
    xmppPush.toXml(&xmlWriter);
    qCDebug(lcCoreMessenger).noquote() << "Unsubscribe XMPP request:" << xml;
#endif

    const bool sentStatus = m_impl->xmpp->sendPacket(xmppPush);

    qCDebug(lcCoreMessenger) << "Unsubscribe from push notifications status: " << (sentStatus ? "success" : "failed");
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

    qCInfo(lcCoreMessenger) << "Generate auth header for the Messenger Backend Service";

    vssq_error_t error;
    vssq_error_reset(&error);

    const vssq_messenger_auth_t *auth = vssq_messenger_auth(m_impl->messenger.get());
    vssc_http_header_t *auth_header = vssq_messenger_auth_generate_messenger_auth_header(auth, &error);

    if (vssq_error_has_error(&error)) {
        qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return QString();
    }

    auto authHeaderValue = vsc_str_to_qstring(vssc_http_header_value(auth_header));

    vssc_http_header_destroy(&auth_header);

    qCDebug(lcCoreMessenger) << "Auth header for the Messenger Backend Service: " << authHeaderValue;

    return authHeaderValue;
}


void
Self::changeConnectionState(ConnectionState state) {
    if (m_impl->connectionState != state) {
        m_impl->connectionState = state;
        emit connectionStateChanged(state);
    }
}

void
Self::connectXmppServer() {
    vssq_error_t error;
    vssq_error_reset(&error);

    qCDebug(lcCoreMessenger) << "Obtain XMPP credentials...";

    const vssq_messenger_auth_t *auth = vssq_messenger_auth(m_impl->messenger.get());
    const vssq_ejabberd_jwt_t *jwt = vssq_messenger_auth_ejabberd_jwt(auth, &error);

    if (vssq_error_has_error(&error)) {
        qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        changeConnectionState(Self::ConnectionState::Disconnected);
        return;
    }

    QString xmppPass = vsc_str_to_qstring(vssq_ejabberd_jwt_as_string(jwt));

    qCDebug(lcCoreMessenger) << "Connect user with JID: " << currentUserJid();

    QXmppConfiguration config{};
    config.setJid(currentUserJid());
    config.setHost(CustomerEnv::xmppServiceUrl());
    config.setPassword(xmppPass);
    config.setAutoReconnectionEnabled(false);
    config.setAutoAcceptSubscriptions(true);
    if (nullptr == m_impl->xmpp) {
        //
        //  Configure QXMPP.
        //
        resetXmppConfiguration();
    }

    qCDebug(lcCoreMessenger) << "Connecting to XMPP server...";
    m_impl->xmpp->connectToServer(config);
}

void
Self::onReconnectXmppServerIfNeeded() {
    if (isSignedIn() && isNetworkOnline() && isXmppDisconnected() && !m_impl->suspended) {
        connectXmppServer();
    }
}

void
Self::onDisconnectXmppServer() {
    m_impl->xmpp->disconnectFromServer();
}

void
Self::onCleanupCommKitMessenger() {
    m_impl->messenger = nullptr;
}

// --------------------------------------------------------------------------
// Find users.
// --------------------------------------------------------------------------
std::shared_ptr<User>
Self::findUserByUsername(const QString &username) const {
    qCDebug(lcCoreMessenger) << "Trying to find user with username: " << username;

    //
    //  Check cache first.
    //
    auto userIt = m_impl->usernameToUser.find(username);
    if (userIt != m_impl->usernameToUser.end()) {
        qCDebug(lcCoreMessenger) << "User found in the cache";
        return userIt->second;
    }

    //
    //  Search on-line.
    //
    if (!isOnline()) {
        qCWarning(lcCoreMessenger) << "Attempt to find user when offline.";
        return nullptr;
    }

    vssq_error_t error;
    vssq_error_reset(&error);

    auto usernameStdStr = username.toStdString();

    auto user = vssq_messenger_find_user_with_username(m_impl->messenger.get(), vsc_str_from(usernameStdStr), &error);

    if (vssq_error_has_error(&error)) {
        qCDebug(lcCoreMessenger) << "User not found";
        qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return nullptr;
    }

    qCDebug(lcCoreMessenger) << "User found in the cloud";

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
    qCDebug(lcCoreMessenger) << "Trying to find user with id: " << userId;

    //
    //  Check cache first.
    //
    auto userIt = m_impl->identityToUser.find(userId);
    if (userIt != m_impl->identityToUser.end()) {
        qCDebug(lcCoreMessenger) << "User found in the cache";
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
        qCDebug(lcCoreMessenger) << "User not found";
        qCWarning(lcCoreMessenger) << "Got error status: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return nullptr;
    }

    qCDebug(lcCoreMessenger) << "User found in the cloud";

    //
    //  Cache and return.
    //
    auto commKitUserImpl = std::make_unique<UserImpl>(user);
    auto commKitUser = std::make_shared<User>(std::move(commKitUserImpl));

    m_impl->identityToUser[userId] = commKitUser;

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
        qCInfo(lcCoreMessenger) << "Trying to send message with id: " << QString(message->id());

        if (!isOnline()) {
            qCInfo(lcCoreMessenger) << "Trying to send message when offline";
            return Self::Result::Error_Offline;
        }

        //
        //  Find recipient.
        //
        auto recipient = findUserById(message->recipientId());
        if (!recipient) {
            //
            //  Got network troubles to find recipient, so cache message and try later.
            //
            qCWarning(lcCoreMessenger) << "Can not send message - recipient is not found";
            return Self::Result::Error_UserNotFound;
        }

        //
        //  Pack message to JSON.
        //
        QJsonObject messageJson;
        messageJson.insert("version", "v3");
        messageJson.insert("timestamp", static_cast<qint64>(message->createdAt().toTime_t()));
        messageJson.insert("from", message->senderUsername());
        messageJson.insert("to", message->recipientUsername());
        messageJson.insert("content", MessageContentJsonUtils::to(message->content()));

        //
        //  Encrypt message.
        //
        auto messageData = MessageContentJsonUtils::toBytes(messageJson);
        auto ciphertextDataMinLen = vssq_messenger_encrypted_message_len(m_impl->messenger.get(), messageData.size(), recipient->impl()->user.get());

        qCDebug(lcCoreMessenger) << "Message Len   : " << messageData.size();
        qCDebug(lcCoreMessenger) << "ciphertext Len: " << ciphertextDataMinLen;

        QByteArray ciphertextData(ciphertextDataMinLen, 0x00);

        auto ciphertext = vsc_buffer_wrap_ptr(vsc_buffer_new());
        vsc_buffer_use(ciphertext.get(),  (byte *)ciphertextData.data(), ciphertextData.size());

        const vssq_status_t encryptionStatus = vssq_messenger_encrypt_data(
                    m_impl->messenger.get(), vsc_data_from(messageData), recipient->impl()->user.get(), ciphertext.get());

        if (encryptionStatus != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessenger) << "Can not encrypt ciphertext: " << vsc_str_to_qstring(vssq_error_message_from_status(encryptionStatus));
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        ciphertextData.resize(vsc_buffer_len(ciphertext.get()));

        //
        //  Pack JSON body.
        //
        QJsonObject messageBodyJson;
        messageBodyJson["pushType"] = "alert";
        messageBodyJson["ciphertext"] = QString::fromLatin1(ciphertextData.toBase64());

        QByteArray messageBody = MessageContentJsonUtils::toBytes(messageBodyJson).toBase64();

        qCDebug(lcCoreMessenger) << "Will send XMPP message with body: " << messageBody;

        // TODO: review next lines when implement group chats.
        auto senderJid = userIdToJid(message->senderId());
        auto recipientJid = userIdToJid(UserId(message->chatId()));

        qCDebug(lcCoreMessenger) << "Will send XMPP message from JID: " << senderJid;
        qCDebug(lcCoreMessenger) << "Will send XMPP message to JID: " << recipientJid;

        QXmppMessage xmppMessage(senderJid, recipientJid, messageBody);
        xmppMessage.setId(message->id());
        xmppMessage.setStamp(message->createdAt());
        xmppMessage.setType(QXmppMessage::Type::Chat);
        xmppMessage.setReceiptRequested(true);

        //
        //  Send.
        //
        bool isSent = m_impl->xmpp->sendPacket(xmppMessage);
        if (isSent) {
            qCDebug(lcCoreMessenger) << "XMPP message was sent";
            return Self::Result::Success;
        } else {
            qCWarning(lcCoreMessenger) << "Can not send message - XMPP send failed";
            return Self::Result::Error_SendMessageFailed;
        }
    });

}


QFuture<Self::Result>
Self::processReceivedXmppMessage(const QXmppMessage& xmppMessage) {

    return QtConcurrent::run([this, xmppMessage]() -> Result {
        qCInfo(lcCoreMessenger) << "Received XMPP message";
        qCDebug(lcCoreMessenger) << "Received XMPP message: " << xmppMessage.id() << "from: " << xmppMessage.from();

        auto message = std::make_unique<IncomingMessage>();

        // TODO: review next lines when implement group chats.
        message->setId(MessageId(xmppMessage.id()));
        message->setRecipientId(userIdFromJid(xmppMessage.to()));
        message->setSenderId(userIdFromJid(xmppMessage.from()));
        message->setCreatedAt(xmppMessage.stamp());

        //
        //  Decode message body from Base64 and JSON.
        //
        auto messageBody = xmppMessage.body().toLatin1();
        if (messageBody.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - body is empty";
            return Self::Result::Error_InvalidMessageFormat;
        }

        auto messageBodyJsonString = QByteArray::fromBase64Encoding(messageBody);
        if (!messageBodyJsonString) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - body is not Base64";
            return Self::Result::Error_InvalidMessageFormat;
        }

        const auto messageBodyJsonDocument = QJsonDocument::fromJson(*messageBodyJsonString);
        if (messageBodyJsonDocument.isNull()) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - body is not JSON within Base64";
            return Self::Result::Error_InvalidMessageFormat;
        }

        const auto messageBodyJson = messageBodyJsonDocument.object();

        //
        //  Get ciphertext.
        //
        const auto ciphertextBase64 = messageBodyJson["ciphertext"].toString();
        if (ciphertextBase64.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - empty ciphertext";
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        auto ciphertextDecoded = QByteArray::fromBase64Encoding(ciphertextBase64.toLatin1());
        if (!ciphertextDecoded) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - ciphertext is not base64 encoded";
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        auto ciphertext = *ciphertextDecoded;

        //
        //  Find sender.
        //
        auto sender = findUserById(message->senderId());
        if (!sender) {
            //
            //  Got network troubles to find sender, so cache message and try later.
            //
            qCWarning(lcCoreMessenger) << "Can not decrypt ciphertext - sender is not found";
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
            qCWarning(lcCoreMessenger) << "Can not decrypt ciphertext: " << vsc_str_to_qstring(vssq_error_message_from_status(decryptionStatus));
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        plaintextData.resize(vsc_buffer_len(plaintext.get()));

        //
        //  Parse message.
        //
        const auto messageJsonDocument = QJsonDocument::fromJson(plaintextData);
        if (messageJsonDocument.isNull()) {
            qCWarning(lcCoreMessenger) << "Got invalid message format - not a JSON";
            return Self::Result::Error_InvalidMessageFormat;
        }

        const auto messageJson = messageJsonDocument.object();

        //
        //  Check version and timestamp.
        //
        const auto version = messageJson["version"].toString();
        if (version != "v3") {
            qCWarning(lcCoreMessenger) << "Got invalid message - unsupported version, expected v3, got " << version;
            return Self::Result::Error_InvalidMessageVersion;
        }

        const auto timestamp = messageJson["timestamp"].toVariant().value<uint>();
        if (timestamp != xmppMessage.stamp().toTime_t()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - timestamp mismatch. Expected "
                                          << timestamp << ", xmpp " << xmppMessage.stamp().toTime_t();
            message->setCreatedAt(QDateTime::fromTime_t(timestamp));
        }

        //
        //  Get sender and recipient usernames.
        //
        auto senderUsername = messageJson["from"].toString();
        if (senderUsername.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - missing 'from' username";
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setSenderUsername(std::move(senderUsername));

        auto recipientUsername = messageJson["to"].toString();
        if (recipientUsername.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - missing 'to' username";
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setRecipientUsername(std::move(recipientUsername));

        //
        //  Parse content.
        //
        const auto contentJson = messageJson["content"].toObject();
        if (contentJson.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - missing content";
            return Self::Result::Error_InvalidMessageFormat;
        }

        QString errorString;
        auto content = MessageContentJsonUtils::from(contentJson, errorString);

        if (std::get_if<std::monostate>(&content)) {
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setContent(std::move(content));

        qCInfo(lcCoreMessenger) << "Received XMPP message was decrypted";

        //
        //  Tell the world we got a message.
        //
        emit messageReceived(std::move(message));

        return Self::Result::Success;
    });
}

QFuture<Self::Result>
Self::processReceivedXmppCarbonMessage(const QXmppMessage& xmppMessage) {

    return QtConcurrent::run([this, xmppMessage]() -> Result {
        qCInfo(lcCoreMessenger) << "Received XMPP message copy";
        qCDebug(lcCoreMessenger) << "Received XMPP message copy: " << xmppMessage.id();

        auto senderId = userIdFromJid(xmppMessage.from());
        auto recipientId = userIdFromJid(xmppMessage.to());

        if (currentUser()->id() != senderId) {
            qCWarning(lcCoreMessenger) << "Got message carbons copy from an another account: " << senderId;
            return Self::Result::Error_InvalidCarbonMessage;
        }

        auto message = std::make_unique<OutgoingMessage>();

        // TODO: review next lines when implement group chats.
        message->setId(MessageId(xmppMessage.id()));
        message->setRecipientId(recipientId);
        message->setSenderId(senderId);
        message->setCreatedAt(xmppMessage.stamp());

        //
        //  Decode message body from Base64 and JSON.
        //
        auto messageBody = xmppMessage.body().toLatin1();
        if (messageBody.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - body is empty";
            return Self::Result::Error_InvalidMessageFormat;
        }

        auto messageBodyJsonString = QByteArray::fromBase64Encoding(messageBody);
        if (!messageBodyJsonString) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - body is not Base64";
            return Self::Result::Error_InvalidMessageFormat;
        }

        const auto messageBodyJsonDocument = QJsonDocument::fromJson(*messageBodyJsonString);
        if (messageBodyJsonDocument.isNull()) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - body is not JSON within Base64";
            return Self::Result::Error_InvalidMessageFormat;
        }

        const auto messageBodyJson = messageBodyJsonDocument.object();

        //
        //  Get ciphertext.
        //
        const auto ciphertextBase64 = messageBodyJson["ciphertext"].toString();
        if (ciphertextBase64.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - empty ciphertext";
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        auto ciphertextDecoded = QByteArray::fromBase64Encoding(ciphertextBase64.toLatin1());
        if (!ciphertextDecoded) {
            qCWarning(lcCoreMessenger) << "Got invalid XMPP message - ciphertext is not base64 encoded";
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        auto ciphertext = *ciphertextDecoded;

        //
        //  Sender is a current user.
        //
        auto sender = currentUser();

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
            qCWarning(lcCoreMessenger) << "Can not decrypt ciphertext: " << vsc_str_to_qstring(vssq_error_message_from_status(decryptionStatus));
            return Self::Result::Error_InvalidMessageCiphertext;
        }

        plaintextData.resize(vsc_buffer_len(plaintext.get()));

        //
        //  Parse message.
        //
        const auto messageJsonDocument = QJsonDocument::fromJson(plaintextData);
        if (messageJsonDocument.isNull()) {
            qCWarning(lcCoreMessenger) << "Got invalid message format - not a JSON";
            return Self::Result::Error_InvalidMessageFormat;
        }

        const auto messageJson = messageJsonDocument.object();

        //
        //  Check version and timestamp.
        //
        const auto version = messageJson["version"].toString();
        if (version != "v3") {
            qCWarning(lcCoreMessenger) << "Got invalid message - unsupported version, expected v3, got " << version;
            return Self::Result::Error_InvalidMessageVersion;
        }

        const auto timestamp = messageJson["timestamp"].toVariant().value<uint>();
        if (timestamp != xmppMessage.stamp().toTime_t()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - timestamp mismatch. Expected "
                                          << timestamp << ", xmpp " << xmppMessage.stamp().toTime_t();
            message->setCreatedAt(QDateTime::fromTime_t(timestamp));
        }

        //
        //  Get sender and recipient usernames.
        //
        auto senderUsername = messageJson["from"].toString();
        if (senderUsername.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - missing 'from' username";
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setSenderUsername(std::move(senderUsername));

        auto recipientUsername = messageJson["to"].toString();
        if (recipientUsername.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - missing 'to' username";
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setRecipientUsername(std::move(recipientUsername));

        //
        //  Parse content.
        //
        const auto contentJson = messageJson["content"].toObject();
        if (contentJson.isEmpty()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - missing content";
            return Self::Result::Error_InvalidMessageFormat;
        }

        QString errorString;
        auto content = MessageContentJsonUtils::from(contentJson, errorString);

        if (std::get_if<std::monostate>(&content)) {
            return Self::Result::Error_InvalidMessageFormat;
        }

        message->setContent(std::move(content));
        message->setStage(OutgoingMessageStage::Sent);
        message->markAsOutgoingCopyFromOtherDevice();

        qCInfo(lcCoreMessenger) << "Received XMPP message was decrypted";

        //
        //  Tell the world we got a message.
        //
        emit messageReceived(std::move(message));

        return Self::Result::Success;
    });
}

std::tuple<Self::Result, QByteArray, QByteArray>
Self::encryptFile(const QString &sourceFilePath, const QString &destFilePath) {
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> std::tuple<Result, QByteArray, QByteArray> {
        qCWarning(lcCoreMessenger) << "Can not encrypt file: " <<
                vsc_str_to_qstring(vssq_error_message_from_status(status));

        return std::make_tuple(Self::Result::Error_FileEncryptionCryptoFailed, QByteArray(), QByteArray());
    };

    auto fileError = [](Self::Result error) -> std::tuple<Result, QByteArray, QByteArray> {
        qCWarning(lcCoreMessenger) << "Can not encrypt file - read/write failed.";

        return std::make_tuple(error, QByteArray(), QByteArray());
    };


    //
    //  Open File Streams.
    //
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.exists()) {
        qCWarning(lcCoreMessenger) << "Can not encrypt file - source file not exists.";
        return fileError(Self::Result::Error_FileDecryptionReadFailed);
    }

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return fileError(Self::Result::Error_FileEncryptionReadFailed);
    }

    QFile destFile(destFilePath);
    if (!destFile.open(QIODevice::WriteOnly)) {
        return fileError(Self::Result::Error_FileEncryptionReadFailed);
    }

    const auto fileSize = sourceFile.size();

    //
    //  Encrypt - Step 1 - Initialize cipher.
    //
    auto fileCipher = vssq_messenger_file_cipher_wrap(vssq_messenger_file_cipher_new());
    vssq_status_t encryptionStatus = vssq_messenger_file_cipher_setup_defaults(fileCipher.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    //
    //  Encrypt - Step 2 - Get and store decryption key.
    //
    const auto decryptionKeyBufLen = vssq_messenger_file_cipher_init_encryption_out_key_len(fileCipher.get());
    auto [decryptionKey, decryptionKeyBuf] = makeMappedBuffer(decryptionKeyBufLen);

    encryptionStatus = vssq_messenger_file_cipher_init_encryption(fileCipher.get(), decryptionKeyBuf.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    adjustMappedBuffer(decryptionKeyBuf, decryptionKey);

    //
    //  Encrypt - Step 3 - Write head.
    //
    auto workingBuffer = vsc_buffer_wrap_ptr(vsc_buffer_new());

    const auto headLen = vssq_messenger_file_cipher_start_encryption_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), headLen);

    encryptionStatus = vssq_messenger_file_cipher_start_encryption(fileCipher.get(), workingBuffer.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
        return fileError(Self::Result::Error_FileEncryptionWriteFailed);
    }

    //
    //  Encrypt - Step 4 - Encrypt file.
    //
    qint64 processedFileSize = 0;
    while(!sourceFile.atEnd()) {
        //
        //  Read chunk.
        //
        char readBytes[2048];
        const auto readBytesSize = sourceFile.read(readBytes, sizeof(readBytes));

        if (readBytesSize == -1) {
            return fileError(Self::Result::Error_FileEncryptionReadFailed);
        } else {
            processedFileSize += readBytesSize;
        }

        vsc_data_t readBytesData = vsc_data((byte *)readBytes, (size_t)readBytesSize);

        //
        //  Encrypt chunk.
        //
        const auto workingBufferLen =
                vssq_messenger_file_cipher_process_encryption_out_len(fileCipher.get(), readBytesData.len);

        vsc_buffer_reset_with_capacity(workingBuffer.get(), workingBufferLen);

        encryptionStatus =
            vssq_messenger_file_cipher_process_encryption(fileCipher.get(), readBytesData, workingBuffer.get());

        if (encryptionStatus != vssq_status_SUCCESS) {
            return cryptoError(encryptionStatus);
        }

        //
        //  Write chunk.
        //
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), (qint64)vsc_buffer_len(workingBuffer.get())) == -1) {
            return fileError(Self::Result::Error_FileEncryptionWriteFailed);
        }
    }

    if (processedFileSize != fileSize) {
        return fileError(Self::Result::Error_FileEncryptionReadFailed);
    }

    //
    //  Encrypt - Step 5 - Write tail and produce signature.
    //
    const auto tailLen = vssq_messenger_file_cipher_finish_encryption_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), tailLen);

    const auto ownerPrivateKey = vssq_messenger_creds_private_key(vssq_messenger_creds(m_impl->messenger.get()));

    const auto signatureBufferLen = vssq_messenger_file_cipher_finish_encryption_signature_len(fileCipher.get(), ownerPrivateKey);
    auto [signature, signatureBuffer] = makeMappedBuffer(signatureBufferLen);

    encryptionStatus = vssq_messenger_file_cipher_finish_encryption(fileCipher.get(), ownerPrivateKey, workingBuffer.get(), signatureBuffer.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
        return fileError(Self::Result::Error_FileEncryptionWriteFailed);
    }

    //
    //  Return result.
    //
    return std::make_tuple(Self::Result::Success, std::move(decryptionKey), std::move(signature));
}


Self::Result
Self::decryptFile(const QString &sourceFilePath, const QString &destFilePath, const QByteArray& decryptionKey,
        const QByteArray& signature, const UserId senderId) {
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> Self::Result {
        qCWarning(lcCoreMessenger) << "Can not decrypt file: " <<
                vsc_str_to_qstring(vssq_error_message_from_status(status));

        return Self::Result::Error_FileDecryptionCryptoFailed;
    };

    auto fileError = [](Self::Result error) -> Self::Result {
        qCWarning(lcCoreMessenger) << "Can not decrypt file - read/write failed.";

        return error;
    };

    //
    //  Find file sender.
    //
    auto sender = findUserById(senderId);
    if (!sender) {
        qCWarning(lcCoreMessenger) << "Can not decrypt file - file sender info is not found.";
        return Self::Result::Error_UserNotFound;
    }

    //
    //  Open File Streams.
    //
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.exists()) {
        qCWarning(lcCoreMessenger) << "Can not decrypt file - source file not exists.";
        return fileError(Self::Result::Error_FileDecryptionReadFailed);
    }

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return fileError(Self::Result::Error_FileDecryptionReadFailed);
    }

    QFile destFile(destFilePath);
    if (!destFile.open(QIODevice::WriteOnly)) {
        return fileError(Self::Result::Error_FileDecryptionReadFailed);
    }

    const auto fileSize = sourceFile.size();

    //
    //  Decrypt - Step 1 - Initialize crypto.
    //
    auto fileCipher = vssq_messenger_file_cipher_wrap(vssq_messenger_file_cipher_new());
    vssq_status_t decryptionStatus = vssq_messenger_file_cipher_setup_defaults(fileCipher.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    //
    //  Decrypt - Step 2 - Setup decryption key.
    //
    decryptionStatus = vssq_messenger_file_cipher_start_decryption(fileCipher.get(), vsc_data_from(decryptionKey), vsc_data_from(signature));

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    //
    //  Decrypt - Step 3 - Decrypt file.
    //
    auto workingBuffer = vsc_buffer_wrap_ptr(vsc_buffer_new());

    qint64 processedFileSize = 0;
    while(!sourceFile.atEnd()) {
        //
        //  Read chunk.
        //
        char readBytes[2048];
        const auto readBytesSize = sourceFile.read(readBytes, sizeof(readBytes));

        if (readBytesSize == -1) {
            return fileError(Self::Result::Error_FileDecryptionReadFailed);
        } else {
            processedFileSize += readBytesSize;
        }

        vsc_data_t readBytesData = vsc_data((byte *)readBytes, (size_t)readBytesSize);

        //
        //  Decrypt chunk.
        //
        const auto workingBufferLen =
                vssq_messenger_file_cipher_process_decryption_out_len(fileCipher.get(), readBytesData.len);

        vsc_buffer_reset_with_capacity(workingBuffer.get(), workingBufferLen);

        decryptionStatus =
            vssq_messenger_file_cipher_process_decryption(fileCipher.get(), readBytesData, workingBuffer.get());

        if (decryptionStatus != vssq_status_SUCCESS) {
            return cryptoError(decryptionStatus);
        }

        //
        //  Write chunk.
        //
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), (qint64)vsc_buffer_len(workingBuffer.get())) == -1) {
            return fileError(Self::Result::Error_FileDecryptionWriteFailed);
        }
    }

    if (processedFileSize != fileSize) {
        return fileError(Self::Result::Error_FileDecryptionReadFailed);
    }

    //
    //  Decrypt - Step 4 - Write tail and verify signature.
    //
    const auto tailLen = vssq_messenger_file_cipher_finish_decryption_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), tailLen);

    const auto senderPublicKey = vssq_messenger_user_public_key(sender->impl()->user.get());
    decryptionStatus = vssq_messenger_file_cipher_finish_decryption(fileCipher.get(), senderPublicKey, workingBuffer.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
        return fileError(Self::Result::Error_FileDecryptionWriteFailed);
    }

    return Self::Result::Success;
}



// --------------------------------------------------------------------------
//  XMPP event handlers.
// --------------------------------------------------------------------------
void
Self::xmppOnConnected() {
    m_impl->lastActivityManager->setEnabled(true);

    if (m_impl->xmppCarbonManager->carbonsEnabled()) {
        m_impl->xmppCarbonManager->setCarbonsEnabled(true);
    }

    changeConnectionState(Self::ConnectionState::Connected);

    registerPushNotifications();
}


void
Self::xmppOnDisconnected() {
    m_impl->lastActivityManager->setEnabled(false);

    changeConnectionState(Self::ConnectionState::Disconnected);

    reconnectXmppServerIfNeeded();
}


void
Self::xmppOnStateChanged(QXmppClient::State state) {
    if (QXmppClient::ConnectingState == state) {
        changeConnectionState(Self::ConnectionState::Connecting);
    }
}


void
Self::xmppOnError(QXmppClient::Error error) {
    qCWarning(lcCoreMessenger) << "XMPP error: " << error;
    emit connectionStateChanged(Self::ConnectionState::Error);

    m_impl->xmpp->disconnectFromServer();

    // Wait 3 second and try to reconnect.
    QTimer::singleShot(3000, this, &Self::reconnectXmppServerIfNeeded);
}


void
Self::xmppOnPresenceReceived(const QXmppPresence &presence) {
    Q_UNUSED(presence)
}


void
Self::xmppOnIqReceived(const QXmppIq &iq) {
    Q_UNUSED(iq)
}

void
Self::xmppOnSslErrors(const QList<QSslError> &errors) {
    qCWarning(lcCoreMessenger) << "XMPP SSL errors: " << errors;
    emit connectionStateChanged(Self::ConnectionState::Error);

    m_impl->xmpp->disconnectFromServer();

    // Wait 3 second and try to reconnect.
    QTimer::singleShot(3000, this, &Self::reconnectXmppServerIfNeeded);
}


void
Self::xmppOnMessageReceived(const QXmppMessage &xmppMessage) {
    //
    //  TODO: handle result.
    //
    processReceivedXmppMessage(xmppMessage);
}


void
Self::xmppOnCarbonMessageReceived(const QXmppMessage &xmppMessage) {
    //
    //  TODO: handle result.
    //
    processReceivedXmppCarbonMessage(xmppMessage);
}


void
Self::xmppOnMessageDelivered(const QString &jid, const QString &messageId) {
    qCDebug(lcCoreMessenger) << "Message delivered to: " << jid;
    OutgoingMessageStageUpdate update;
    update.messageId = MessageId(messageId);
    update.stage = OutgoingMessageStage::Delivered;
    emit updateMessage(update);
}


// --------------------------------------------------------------------------
//  Network event handlers.
// --------------------------------------------------------------------------
void
Self::onProcessNetworkState(bool isOnline) {
    if (isOnline) {
        emit reconnectXmppServerIfNeeded();
    } else {
        emit disconnectXmppServer();
    }
}


// --------------------------------------------------------------------------
//  LastActivityManager: controls and events.
// --------------------------------------------------------------------------
void
Self::setCurrentRecipient(const UserId& recipientId) {

    if (recipientId.isValid()) {
        auto user = findUserById(recipientId);
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
    qCDebug(lcCoreMessenger) << "Upload service found: " << found;
    emit uploadServiceFound(found);
}

void Self::xmppOnUploadSlotReceived(const QXmppHttpUploadSlotIq &slot) {
    emit uploadSlotReceived(slot.id(), slot.putUrl(), slot.getUrl());
}


void Self::xmppOnUploadRequestFailed(const QXmppHttpUploadRequestIq &request) {

    const auto error = request.error();

    qCDebug(lcCoreMessenger) << QString("code(%1), condition(%2), text(%3)")
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
        case Self::ConnectionState::Disconnected:
            qCDebug(lcCoreMessenger) << "New connection status: disconnected";
            break;

        case Self::ConnectionState::Connecting:
            qCDebug(lcCoreMessenger) << "New connection status: connecting";
            break;

        case Self::ConnectionState::Connected:
            qCDebug(lcCoreMessenger) << "New connection status: connected";
            break;

        case Self::ConnectionState::Error:
            qCDebug(lcCoreMessenger) << "New connection status: error";
            break;
    }
}
