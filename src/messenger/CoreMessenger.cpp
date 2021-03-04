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

#include "CoreMessenger.h"

#include "CommKitBridge.h"
#include "CustomerEnv.h"
#include "IncomingMessage.h"
#include "MessageContentJsonUtils.h"
#include "OutgoingMessage.h"
#include "UserImpl.h"
#include "Utils.h"

#include "VSQNetworkAnalyzer.h"
#include "VSQDiscoveryManager.h"
#include "VSQContactManager.h"
#include "VSQLastActivityManager.h"
#include "XmppRoomParticipantsManager.h"
#include "XmppMucSubManager.h"

#if VS_PUSHNOTIFICATIONS
#    include "XmppPushNotifications.h"
using namespace notifications;
using namespace notifications::xmpp;
#endif // VS_PUSHNOTIFICATIONS

#include <virgil/crypto/foundation/vscf_ctr_drbg.h>

#include <virgil/sdk/core/vssc_json_object.h>
#include <virgil/sdk/comm-kit/vssq_messenger.h>
#include <virgil/sdk/comm-kit/vssq_error_message.h>
#include <virgil/sdk/comm-kit/vssq_messenger_file_cipher.h>
#include <virgil/sdk/comm-kit/vssq_messenger_cloud_fs.h>

#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppMessageReceiptManager.h>
#include <qxmpp/QXmppCarbonManager.h>
#include <qxmpp/QXmppUploadRequestManager.h>
#include <qxmpp/QXmppMucManager.h>
#include <qxmpp/QXmppPubSubItem.h>
#include <qxmpp/QXmppPubSubIq.h>

#include <QCryptographicHash>
#include <QMap>
#include <QXmlStreamWriter>
#include <QtConcurrent>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QPointer>

#include <memory>
#include <mutex>

using namespace vm;
using Self = vm::CoreMessenger;

Q_LOGGING_CATEGORY(lcCoreMessenger, "core-messenger");
Q_LOGGING_CATEGORY(lcCoreMessengerXMPP, "core-messenger-xmpp");

// --------------------------------------------------------------------------
// XMPP Helpers.
// --------------------------------------------------------------------------
template<typename T>
static QString toXmlString(const T &obj)
{
    QString xmlStr;
    QXmlStreamWriter xmlWriter(&xmlStr);
    xmlWriter.setAutoFormatting(true);
    obj.toXml(&xmlWriter);
    return xmlStr;
}

// --------------------------------------------------------------------------
// C Helpers.
// --------------------------------------------------------------------------
using vscf_ctr_drbg_ptr_t = vsc_unique_ptr<vscf_ctr_drbg_t>;

using vssc_json_object_unique_ptr_t = vsc_unique_ptr<vssc_json_object_t>;
using vssq_messenger_creds_unique_ptr_t = vsc_unique_ptr<vssq_messenger_creds_t>;
using vssq_messenger_ptr_t = vsc_unique_ptr<vssq_messenger_t>;
using vssq_messenger_file_cipher_ptr_t = vsc_unique_ptr<vssq_messenger_file_cipher_t>;
using vssq_messenger_user_list_ptr_t = vsc_unique_ptr<vssq_messenger_user_list_t>;
using vssq_messenger_group_ptr_t = vsc_unique_ptr<vssq_messenger_group_t>;
using vssq_messenger_group_shared_ptr_t = std::shared_ptr<vssq_messenger_group_t>;

static vscf_ctr_drbg_ptr_t vscf_ctr_drbg_wrap_ptr(vscf_ctr_drbg_t *ptr)
{
    return vscf_ctr_drbg_ptr_t { ptr, vscf_ctr_drbg_delete };
}

static vssc_json_object_unique_ptr_t vssc_json_object_wrap_ptr(vssc_json_object_t *ptr)
{
    return vssc_json_object_unique_ptr_t { ptr, vssc_json_object_delete };
}

static vssq_messenger_creds_unique_ptr_t vssq_messenger_creds_wrap_ptr(vssq_messenger_creds_t *ptr)
{
    return vssq_messenger_creds_unique_ptr_t { ptr, vssq_messenger_creds_delete };
}

static vssq_messenger_ptr_t vssq_messenger_wrap_ptr(vssq_messenger_t *ptr)
{
    return vssq_messenger_ptr_t { ptr, vssq_messenger_delete };
}

static vssq_messenger_file_cipher_ptr_t vssq_messenger_file_cipher_wrap(vssq_messenger_file_cipher_t *ptr)
{
    return vssq_messenger_file_cipher_ptr_t { ptr, vssq_messenger_file_cipher_delete };
}

static vssq_messenger_user_list_ptr_t vssq_messenger_user_list_wrap_ptr(vssq_messenger_user_list_t *ptr)
{
    return vssq_messenger_user_list_ptr_t { ptr, vssq_messenger_user_list_delete };
}

static vssq_messenger_group_ptr_t vssq_messenger_group_wrap_ptr(vssq_messenger_group_t *ptr)
{
    return vssq_messenger_group_ptr_t { ptr, vssq_messenger_group_delete };
}

static CoreMessengerStatus mapStatus(vssq_status_t status)
{
    switch (status) {
    case vssq_status_SUCCESS:
        return CoreMessengerStatus::Success;

    case vssq_status_MODIFY_GROUP_FAILED_PERMISSION_VIOLATION:
        return CoreMessengerStatus::Error_ModifyGroup_PermissionViolation;

    case vssq_status_ACCESS_GROUP_FAILED_PERMISSION_VIOLATION:
        return CoreMessengerStatus::Error_AccessGroup_PermissionViolation;

    case vssq_status_CREATE_GROUP_FAILED_CRYPTO_FAILED:
        return CoreMessengerStatus::Error_CreateGroup_CryptoFailed;

    case vssq_status_IMPORT_GROUP_EPOCH_FAILED_PARSE_FAILED:
        return CoreMessengerStatus::Error_ImportGroupEpoch_ParseFailed;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_SESSION_ID_DOESNT_MATCH:
        return CoreMessengerStatus::Error_ProcessGroupMessage_SessionIDDoesntMatch;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_EPOCH_NOT_FOUND:
        return CoreMessengerStatus::Error_ProcessGroupMessage_EpochNotFound;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_WRONG_KEY_TYPE:
        return CoreMessengerStatus::Error_ProcessGroupMessage_WrongKeyType;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_INVALID_SIGNATURE:
        return CoreMessengerStatus::Error_ProcessGroupMessage_InvalidSignature;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_ED25519_FAILED:
        return CoreMessengerStatus::Error_ProcessGroupMessage_Ed25519Failed;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_DUPLICATE_EPOCH:
        return CoreMessengerStatus::Error_ProcessGroupMessage_DuplicateEpoch;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_PLAIN_TEXT_TOO_LONG:
        return CoreMessengerStatus::Error_ProcessGroupMessage_PlainTextTooLong;

    case vssq_status_PROCESS_GROUP_MESSAGE_FAILED_CRYPTO_FAILED:
        return CoreMessengerStatus::Error_ProcessGroupMessage_CryptoFailed;

    default:
        return CoreMessengerStatus::Error_UnexpectedCommKitError;
    }
}

class Self::GroupImpl
{
public:
    explicit GroupImpl(vssq_messenger_group_ptr_t a_ctx) : ctx(std::move(a_ctx)) { }
    vssq_messenger_group_ptr_t ctx;
};

// --------------------------------------------------------------------------
// Configuration.
// --------------------------------------------------------------------------
class Self::Impl
{
public:
    vscf_impl_ptr_t random = vscf_impl_ptr_t(nullptr, vscf_impl_delete);

    vssq_messenger_ptr_t messenger = vssq_messenger_wrap_ptr(nullptr);

    std::map<GroupId, GroupImplHandler> messengerGroups;
    std::map<GroupId, UserId> groupOwners;
    std::mutex groupMutex;

    VSQNetworkAnalyzer *networkAnalyzer;

    std::unique_ptr<QXmppClient> xmpp;
    std::unique_ptr<VSQDiscoveryManager> discoveryManager;
    std::unique_ptr<VSQContactManager> contactManager;

    QPointer<QXmppCarbonManager> xmppCarbonManager;
    QPointer<QXmppUploadRequestManager> xmppUploadManager;
    QPointer<QXmppMucManager> xmppGroupChatManager;
    QPointer<VSQLastActivityManager> lastActivityManager;
    QPointer<XmppRoomParticipantsManager> xmppRoomParticipantsManager;
    QPointer<XmppMucSubManager> xmppMucSubManager;

    std::map<QString, std::shared_ptr<User>> identityToUser;
    std::map<QString, std::shared_ptr<User>> usernameToUser;

    ConnectionState connectionState = ConnectionState::Disconnected;

    bool suspended = false;
};

Self::CoreMessenger(Settings *settings, QObject *parent)
    : QObject(parent), m_impl(std::make_unique<Self::Impl>()), m_settings(settings)
{

    //
    // Register QML types.
    //
    qRegisterMetaType<QXmppClient::Error>();
    qRegisterMetaType<QXmppHttpUploadSlotIq>();
    qRegisterMetaType<QXmppHttpUploadRequestIq>();
    qRegisterMetaType<QXmppPresence>();
    qRegisterMetaType<QXmppMucItem::Affiliation>();

    qRegisterMetaType<vm::MessageHandler>("MessageHandler");
    qRegisterMetaType<vm::ModifiableMessageHandler>("ModifiableMessageHandler");
    qRegisterMetaType<vm::Messages>("Messages");
    qRegisterMetaType<vm::ModifiableMessages>("ModifiableMessages");
    qRegisterMetaType<vm::UserHandler>("UserHandler");
    qRegisterMetaType<vm::GroupHandler>("GroupHandler");
    qRegisterMetaType<vm::ChatHandler>("ChatHandler");
    qRegisterMetaType<vm::ModifiableChatHandler>("ModifiableChatHandler");
    qRegisterMetaType<vm::ModifiableChats>("ModifiableChats");
    qRegisterMetaType<vm::CloudFileHandler>("CloudFileHandler");
    qRegisterMetaType<vm::ModifiableCloudFileHandler>("ModifiableCloudFileHandler");
    qRegisterMetaType<vm::CloudFiles>("CloudFiles");
    qRegisterMetaType<vm::ModifiableCloudFiles>("ModifiableCloudFiles");
    qRegisterMetaType<vm::CloudFileMemberHandler>("CloudFileMemberHandler");
    qRegisterMetaType<vm::CloudFileMembers>("CloudFileMembers");
    qRegisterMetaType<vm::GroupUpdate>("GroupUpdate");
    qRegisterMetaType<vm::Users>("Users");
    qRegisterMetaType<vm::GroupMember>("GroupMember");
    qRegisterMetaType<vm::UserId>("UserId");
    qRegisterMetaType<vm::GroupId>("GroupId");
    qRegisterMetaType<vm::GroupMembers>("GroupMembers");
    qRegisterMetaType<vm::Contact>("Contact");
    qRegisterMetaType<vm::Contacts>("Contacts");

    qRegisterMetaType<vm::ChatId>("ChatId");
    qRegisterMetaType<vm::GroupId>("GroupId");
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
    connect(this, &Self::createGroupChat, this, &Self::onCreateGroupChat);
    connect(this, &Self::joinGroupChats, this, &Self::onJoinGroupChats);
    connect(this, &Self::acceptGroupInvitation, this, &Self::onAcceptGroupInvitation);
    connect(this, &Self::rejectGroupInvitation, this, &Self::onRejectGroupInvitation);

    connect(this, &Self::reconnectXmppServerIfNeeded, this, &Self::onReconnectXmppServerIfNeeded);
    connect(this, &Self::disconnectXmppServer, this, &Self::onDisconnectXmppServer);
    connect(this, &Self::cleanupXmppMucRooms, this, &Self::onCleanupXmppMucRooms);
    connect(this, &Self::cleanupCommKitMessenger, this, &Self::onCleanupCommKitMessenger);
    connect(this, &Self::registerPushNotifications, this, &Self::onRegisterPushNotifications);
    connect(this, &Self::deregisterPushNotifications, this, &Self::onDeregisterPushNotifications);
    connect(this, &Self::xmppCreateGroupChat, this, &Self::xmppOnCreateGroupChat);

    //
    //  Configure Network Analyzer.
    //
    m_impl->networkAnalyzer = new VSQNetworkAnalyzer(nullptr); // will be moved to the thread
    connect(m_impl->networkAnalyzer, &VSQNetworkAnalyzer::connectedChanged, this, &Self::onProcessNetworkState);
}

Self::~CoreMessenger() noexcept = default;

Self::Result Self::resetCommKitConfiguration()
{

    qCDebug(lcCoreMessenger) << "Reset Comm Kit configuration";

    auto messengerServiceUrl = CustomerEnv::messengerServiceUrl();
    auto xmppServiceUrl = CustomerEnv::xmppServiceUrl();
    auto contactDiscoveryServiceUrl = CustomerEnv::contactDiscoveryServiceUrl();

    qCDebug(lcCoreMessenger) << "Messenger Service URL: " << messengerServiceUrl;
    qCDebug(lcCoreMessenger) << "XMPP Service URL     : " << xmppServiceUrl;
    qCDebug(lcCoreMessenger) << "Contact Discovery URL: " << contactDiscoveryServiceUrl;

    auto messengerServiceUrlStd = messengerServiceUrl.toStdString();
    auto xmppServiceUrlStd = xmppServiceUrl.toStdString();
    auto contactDiscoveryServiceUrlStd = contactDiscoveryServiceUrl.toStdString();

    vssq_messenger_config_t *config = vssq_messenger_config_new_with(vsc_str_from(messengerServiceUrlStd),
                                                                     vsc_str_from(contactDiscoveryServiceUrlStd),
                                                                     vsc_str_from(xmppServiceUrlStd));

    auto caBundlePath = CustomerEnv::caBundlePath().toStdString();
    if (!caBundlePath.empty()) {
        vssq_messenger_config_set_ca_bundle(config, vsc_str_from(caBundlePath));
    }

    m_impl->messenger = vssq_messenger_wrap_ptr(vssq_messenger_new_with_config(config));
    vssq_messenger_config_destroy(&config);

    if (!m_impl->random) {
        auto randomImpl = vscf_ctr_drbg_wrap_ptr(vscf_ctr_drbg_new());
        auto randomStatus = vscf_ctr_drbg_setup_defaults(randomImpl.get());
        if (randomStatus != vscf_status_SUCCESS) {
            qCWarning(lcCoreMessenger) << "Got error status: failed to init crypto module - random";
            return Self::Result::Error_CryptoInit;
        }

        m_impl->random = vscf_impl_wrap_ptr(vscf_ctr_drbg_impl(randomImpl.release()));
    }

    vssq_messenger_use_random(m_impl->messenger.get(), m_impl->random.get());
    auto status = vssq_messenger_setup_defaults(m_impl->messenger.get());

    if (status != vssq_status_SUCCESS) {
        qCWarning(lcCoreMessenger) << "Got error status:" << vsc_str_to_qstring(vssq_error_message_from_status(status));
        return Self::Result::Error_CryptoInit;
    }

    //
    //  Cleanup caches.
    //
    std::scoped_lock _(m_impl->groupMutex);
    m_impl->messengerGroups.clear();

    return Self::Result::Success;
}

void Self::resetXmppConfiguration()
{

    qCDebug(lcCoreMessenger) << "Reset XMPP configuration";

    m_impl->xmpp = std::make_unique<QXmppClient>();

    // Add receipt messages extension
    m_impl->discoveryManager = std::make_unique<VSQDiscoveryManager>(m_impl->xmpp.get(), this);
    m_impl->contactManager = std::make_unique<VSQContactManager>(m_impl->xmpp.get(), this);

    //  Create & connect extensions.
    m_impl->xmppCarbonManager = new QXmppCarbonManager();
    m_impl->xmppUploadManager = new QXmppUploadRequestManager();
    m_impl->xmppGroupChatManager = new QXmppMucManager();
    m_impl->lastActivityManager = new VSQLastActivityManager(m_settings);
    m_impl->xmppRoomParticipantsManager = new XmppRoomParticipantsManager();
    m_impl->xmppMucSubManager = new XmppMucSubManager();

    // Parent is implicitly changed to the QXmppClient within addExtension()
    m_impl->xmpp->addExtension(new QXmppMessageReceiptManager());
    m_impl->xmpp->addExtension(m_impl->xmppCarbonManager);
    m_impl->xmpp->addExtension(m_impl->xmppUploadManager);
    m_impl->xmpp->addExtension(m_impl->xmppGroupChatManager);
    m_impl->xmpp->addExtension(m_impl->lastActivityManager);
    m_impl->xmpp->addExtension(m_impl->xmppRoomParticipantsManager);
    m_impl->xmpp->addExtension(m_impl->xmppMucSubManager);

    // Connect XMPP signals
    connect(m_impl->lastActivityManager, &VSQLastActivityManager::lastActivityTextChanged, this,
            &Self::lastActivityTextChanged);

    connect(m_impl->xmppUploadManager, &QXmppUploadRequestManager::serviceFoundChanged, this,
            &Self::xmppOnUploadServiceFound);

    connect(m_impl->xmppUploadManager, &QXmppUploadRequestManager::slotReceived, this, &Self::xmppOnUploadSlotReceived);

    connect(m_impl->xmppUploadManager, &QXmppUploadRequestManager::requestFailed, this,
            &Self::xmppOnUploadRequestFailed);

    connect(m_impl->xmppGroupChatManager, &QXmppMucManager::invitationReceived, this,
            &Self::xmppOnMucInvitationReceived);

    connect(m_impl->xmppGroupChatManager, &QXmppMucManager::roomAdded, this, &Self::xmppOnMucRoomAdded);

    connect(m_impl->xmppRoomParticipantsManager, &XmppRoomParticipantsManager::participantReceived, this,
            &Self::xmppOnRoomParticipantReceived);

    connect(m_impl->xmppMucSubManager, &XmppMucSubManager::subscribeReceived, this, &Self::xmppOnMucSubscribeReceived);

    connect(m_impl->xmppMucSubManager, &XmppMucSubManager::subscribedRoomReceived, this,
            &Self::xmppOnMucSubscribedRoomReceived);

    connect(m_impl->xmppMucSubManager, &XmppMucSubManager::roomSubscriberReceived, this,
            &Self::xmppOnMucRoomSubscriberReceived);

    connect(m_impl->xmppMucSubManager, &XmppMucSubManager::subscribedRoomsProcessed, this,
            &Self::xmppOnMucSubscribedRoomsProcessed);

    connect(m_impl->xmppMucSubManager, &XmppMucSubManager::roomSubscribersProcessed, this,
            &Self::xmppOnMucRoomSubscribersProcessed);

    connect(m_impl->xmppMucSubManager, &XmppMucSubManager::messageReceived, this, &Self::xmppOnMessageReceived);

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

    //
    // Add extra logging
    //
#if ENABLE_XMPP_EXTRA_LOGS
    auto logger = QXmppLogger::getLogger();
    logger->setLoggingType(QXmppLogger::SignalLogging);
    logger->setMessageTypes(QXmppLogger::AnyMessage);

    connect(logger, &QXmppLogger::message, [](QXmppLogger::MessageType, const QString &text) {
        qCDebug(lcCoreMessengerXMPP).noquote() << "" << text;
    });

    m_impl->xmpp->setLogger(logger);
#endif

    auto receipt = m_impl->xmpp->findExtension<QXmppMessageReceiptManager>();
    connect(receipt, &QXmppMessageReceiptManager::messageDelivered, this, &Self::xmppOnMessageDelivered);
}

// --------------------------------------------------------------------------
// Statuses.
// --------------------------------------------------------------------------
bool Self::isOnline() const noexcept
{
    return isSignedIn() && isNetworkOnline() && isXmppConnected();
}

bool Self::isSignedIn() const noexcept
{
    return m_impl->messenger && vssq_messenger_is_authenticated(m_impl->messenger.get());
}

CoreMessenger::ConnectionState Self::connectionState() const
{
    return m_impl->connectionState;
}

bool Self::isNetworkOnline() const noexcept
{
    return m_impl->networkAnalyzer->isConnected();
}

bool Self::isXmppConnected() const noexcept
{
    if (m_impl->xmpp) {
        return m_impl->xmpp->state() == QXmppClient::ConnectedState;
    }

    return false;
}

bool Self::isXmppConnecting() const noexcept
{
    if (m_impl->xmpp) {
        return m_impl->xmpp->state() == QXmppClient::ConnectingState;
    }

    return false;
}

bool Self::isXmppDisconnected() const noexcept
{
    if (m_impl->xmpp) {
        return m_impl->xmpp->state() == QXmppClient::DisconnectedState;
    }

    return true;
}

// --------------------------------------------------------------------------
// State controls.
// --------------------------------------------------------------------------
void Self::onActivate()
{
    m_impl->suspended = false;

    if (m_impl->xmpp) {
        m_impl->xmpp->setActive(true);
    }

    if (isOnline()) {
        QXmppPresence presenceOnline(QXmppPresence::Available);
        presenceOnline.setAvailableStatusType(QXmppPresence::Online);
        m_impl->xmpp->setClientPresence(presenceOnline);

    } else {
        reconnectXmppServerIfNeeded();
    }
}

void Self::onDeactivate()
{
    if (m_impl->xmpp) {
        m_impl->xmpp->setActive(false);
    }

    if (isOnline()) {
        QXmppPresence presenceAway(QXmppPresence::Available);
        presenceAway.setAvailableStatusType(QXmppPresence::Away);
        m_impl->xmpp->setClientPresence(presenceAway);
    }
}

void Self::onSuspend()
{
    if (m_impl->xmpp) {
        //
        //  Setting QXmppPresence::Unavailable also call the disconnectFromServer() function underneath.
        //
        QXmppPresence presenceAway(QXmppPresence::Unavailable);
        presenceAway.setAvailableStatusType(QXmppPresence::XA);
        m_impl->xmpp->setClientPresence(presenceAway);
    }

    m_impl->suspended = true;
}

// --------------------------------------------------------------------------
// User authorization.
// --------------------------------------------------------------------------
QFuture<Self::Result> Self::signIn(const QString &username)
{
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
        auto creds = vssq_messenger_creds_wrap_ptr(
                vssq_messenger_creds_from_json_str(vsc_str_from(credentialsString), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ImportCredentials;
        }

        qCInfo(lcCoreMessenger) << "Sign in user";
        error.status = vssq_messenger_authenticate(m_impl->messenger.get(), creds.get());

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_Signin;
        }

        emit reconnectXmppServerIfNeeded();

        return Self::Result::Success;
    });
}

QFuture<Self::Result> Self::signUp(const QString &username)
{
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
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_Signup;
        }

        qCInfo(lcCoreMessenger) << "User has been successfully signed up";

        qCInfo(lcCoreMessenger) << "Save user credentials";
        auto creds = vssq_messenger_creds(m_impl->messenger.get());
        auto credsJson = vssc_json_object_wrap_ptr(vssq_messenger_creds_to_json(creds, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ExportCredentials;
        }

        auto credentials = vsc_str_to_qstring(vssc_json_object_as_str(credsJson.get()));
        m_settings->setUserCredential(QString::fromStdString(username), credentials);

        emit reconnectXmppServerIfNeeded();

        return Self::Result::Success;
    });
}

QFuture<Self::Result> Self::backupKey(const QString &password)
{
    return QtConcurrent::run([this, password = password.toStdString()]() -> Result {
        qCInfo(lcCoreMessenger) << "Upload current user key to the cloud";

        const vssq_status_t status = vssq_messenger_backup_creds(m_impl->messenger.get(), vsc_str_from(password));
        if (vssq_status_SUCCESS != status) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_status(status));
            return Self::Result::Error_MakeKeyBackup;
        }

        return Self::Result::Success;
    });
}

QFuture<Self::Result> Self::signInWithBackupKey(const QString &username, const QString &password)
{
    return QtConcurrent::run([this, username = username.toStdString(), password = password.toStdString()]() -> Result {
        qCInfo(lcCoreMessenger) << "Load user key from the cloud";

        auto result = resetCommKitConfiguration();
        if (result != Self::Result::Success) {
            qCCritical(lcCoreMessenger) << "Can initialize C CommKit module";
            return result;
        }

        const vssq_status_t status = vssq_messenger_authenticate_with_backup_creds(
                m_impl->messenger.get(), vsc_str_from(username), vsc_str_from(password));

        if (vssq_status_SUCCESS != status) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_status(status));
            return Self::Result::Error_RestoreKeyBackup;
        }

        qCInfo(lcCoreMessenger) << "Save user credentials";
        vssq_error_t error;
        vssq_error_reset(&error);

        auto creds = vssq_messenger_creds(m_impl->messenger.get());
        auto credsJson = vssc_json_object_wrap_ptr(vssq_messenger_creds_to_json(creds, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            return Self::Result::Error_ExportCredentials;
        }

        auto credentials = vsc_str_to_qstring(vssc_json_object_as_str(credsJson.get()));
        m_settings->setUserCredential(QString::fromStdString(username), credentials);

        emit reconnectXmppServerIfNeeded();

        return Self::Result::Success;
    });
}

QFuture<Self::Result> Self::signOut()
{
    return QtConcurrent::run([this]() -> Result {
        qCInfo(lcCoreMessenger) << "Signing out";

        emit deregisterPushNotifications();
        emit cleanupCommKitMessenger();
        emit cleanupXmppMucRooms();
        emit disconnectXmppServer();

        return Self::Result::Success;
    });
}

QString Self::currentUserJid() const
{
    auto user = vssq_messenger_user(m_impl->messenger.get());
    vsc_str_t userIdentity = vssq_messenger_user_identity(user);
    return vsc_str_to_qstring(userIdentity) + "@" + CustomerEnv::xmppServiceDomain() + "/" + m_settings->deviceId();
}

UserId Self::userIdFromJid(const QString &jid) const
{
    return UserId(jid.split("@").first());
}

QString Self::userIdToJid(const UserId &userId) const
{
    return userId + "@" + CustomerEnv::xmppServiceDomain();
}

QString Self::groupIdToJid(const GroupId &groupId) const
{
    return groupId + "@conference." + CustomerEnv::xmppServiceDomain();
}

GroupId Self::groupIdFromJid(const QString &jid) const
{
    return GroupId(jid.split("@").first());
}

UserId Self::groupUserIdFromJid(const QString &jid) const
{
    return UserId(jid.split("/").last());
}

void Self::onRegisterPushNotifications()
{
#if VS_PUSHNOTIFICATIONS
    if (!isOnline()) {
        qCWarning(lcCoreMessenger) << "Can not subscribe for push notifications, no connection. Will try it later.";
        return;
    }

    qCInfo(lcCoreMessenger) << "Register for push notifications on XMPP server.";

    auto xmppPush = XmppPushNotifications::instance().buildEnableIq();

    xmppPush.setNode(currentUserJid());

    qCDebug(lcCoreMessengerXMPP).noquote() << "Subscribe XMPP request:" << toXmlString(xmppPush);

    const bool sentStatus = m_impl->xmpp->sendPacket(xmppPush);

    qCInfo(lcCoreMessenger) << "Register for push notifications on XMPP server status:" << sentStatus;

#endif // VS_PUSHNOTIFICATIONS
}

void Self::onDeregisterPushNotifications()
{
#if VS_PUSHNOTIFICATIONS
    if (!isOnline()) {
        qCWarning(lcCoreMessenger) << "Can not unsubscribe from the push notifications, no connection.";
    }

    auto xmppPush = XmppPushNotifications::instance().buildDisableIq();

    xmppPush.setNode(currentUserJid());

    qCDebug(lcCoreMessengerXMPP).noquote() << "Unsubscribe XMPP request:" << toXmlString(xmppPush);

    const bool sentStatus = m_impl->xmpp->sendPacket(xmppPush);

    qCDebug(lcCoreMessenger) << "Unsubscribe from push notifications status:" << (sentStatus ? "success" : "failed");
#endif // VS_PUSHNOTIFICATIONS
}

bool Self::subscribeToUser(const User &user)
{

    auto userJid = userIdToJid(user.id());

    return m_impl->contactManager->addContact(userJid, user.id(), QString());
}

QUrl Self::getCrashReportEndpointUrl() const
{
    auto urlStr = CustomerEnv::messengerServiceUrl() + "/send-logs";

    return QUrl(urlStr);
}

QString Self::getAuthHeaderVaue() const
{

    qCInfo(lcCoreMessenger) << "Generate auth header for the Messenger Backend Service";

    vssq_error_t error;
    vssq_error_reset(&error);

    const vssq_messenger_auth_t *auth = vssq_messenger_auth(m_impl->messenger.get());
    vssc_http_header_t *auth_header = vssq_messenger_auth_generate_messenger_auth_header(auth, &error);

    if (vssq_error_has_error(&error)) {
        qCWarning(lcCoreMessenger) << "Got error status:" << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return QString();
    }

    auto authHeaderValue = vsc_str_to_qstring(vssc_http_header_value(auth_header));

    vssc_http_header_destroy(&auth_header);

    qCDebug(lcCoreMessenger) << "Auth header for the Messenger Backend Service:" << authHeaderValue;

    return authHeaderValue;
}

void Self::changeConnectionState(ConnectionState state)
{
    if (m_impl->connectionState != state) {
        m_impl->connectionState = state;
        emit connectionStateChanged(state);
    }
}

void Self::connectXmppServer()
{
    vssq_error_t error;
    vssq_error_reset(&error);

    qCDebug(lcCoreMessenger) << "Obtain XMPP credentials...";

    const vssq_messenger_auth_t *auth = vssq_messenger_auth(m_impl->messenger.get());
    const vssq_ejabberd_jwt_t *jwt = vssq_messenger_auth_ejabberd_jwt(auth, &error);

    if (vssq_error_has_error(&error)) {
        qCWarning(lcCoreMessenger) << "Got error status:" << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        changeConnectionState(Self::ConnectionState::Disconnected);
        return;
    }

    QString xmppPass = vsc_str_to_qstring(vssq_ejabberd_jwt_as_string(jwt));

    qCDebug(lcCoreMessenger) << "Connect user with JID:" << currentUserJid();

    QXmppConfiguration config {};
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

void Self::onReconnectXmppServerIfNeeded()
{
    if (isSignedIn() && isNetworkOnline() && isXmppDisconnected() && !m_impl->suspended) {
        connectXmppServer();
    }
}

void Self::onDisconnectXmppServer()
{
    m_impl->xmpp->disconnectFromServer();
}

void Self::onCleanupXmppMucRooms()
{
    for (auto xmppRoom : m_impl->xmppGroupChatManager->rooms()) {
        xmppRoom->leave("Signed out");
        disconnect(xmppRoom);
        xmppRoom->deleteLater();
    }
}

void Self::onCleanupCommKitMessenger()
{
    m_impl->messenger = nullptr;
}

// --------------------------------------------------------------------------
// Find users.
// --------------------------------------------------------------------------
std::shared_ptr<User> Self::findUserByUsername(const QString &username) const
{
    qCDebug(lcCoreMessenger) << "Trying to find user with username:" << username;

    //
    //  Check cache first.
    //
    auto userIt = m_impl->usernameToUser.find(username);
    if (userIt != m_impl->usernameToUser.end()) {
        auto publicKeyId = vsc_str_to_qstring(
                vsc_str_from_data(vssq_messenger_user_public_key_id(userIt->second->impl()->user.get())));
        qCDebug(lcCoreMessenger) << "User found in the cache with public key id:" << publicKeyId;
        emit userWasFound(userIt->second);
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
        qCWarning(lcCoreMessenger) << "Got error status:" << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return nullptr;
    }

    auto publicKeyId = vsc_str_to_qstring(vsc_str_from_data(vssq_messenger_user_public_key_id(user)));
    qCDebug(lcCoreMessenger) << "User found in the cloud with public key id:" << publicKeyId;

    //
    //  Cache and return.
    //
    auto commKitUserImpl = std::make_unique<UserImpl>(user);
    auto commKitUser = std::make_shared<User>(std::move(commKitUserImpl));

    m_impl->usernameToUser[username] = commKitUser;
    m_impl->identityToUser[commKitUser->id()] = commKitUser;

    emit userWasFound(commKitUser);

    return commKitUser;
}

std::shared_ptr<User> Self::findUserById(const UserId &userId) const
{
    qCDebug(lcCoreMessenger) << "Trying to find user with id:" << userId;

    //
    //  Check cache first.
    //
    auto userIt = m_impl->identityToUser.find(userId);
    if (userIt != m_impl->identityToUser.end()) {
        auto publicKeyId = vsc_str_to_qstring(
                vsc_str_from_data(vssq_messenger_user_public_key_id(userIt->second->impl()->user.get())));
        qCDebug(lcCoreMessenger) << "User found in the cache with public key id:" << publicKeyId;
        emit userWasFound(userIt->second);
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
        qCWarning(lcCoreMessenger) << "Got error status:" << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return nullptr;
    }

    auto publicKeyId = vsc_str_to_qstring(vsc_str_from_data(vssq_messenger_user_public_key_id(user)));
    qCDebug(lcCoreMessenger) << "User found in the cloud with public key id:" << publicKeyId;

    //
    //  Cache and return.
    //
    auto commKitUserImpl = std::make_unique<UserImpl>(user);
    auto commKitUser = std::make_shared<User>(std::move(commKitUserImpl));

    m_impl->identityToUser[userId] = commKitUser;

    emit userWasFound(commKitUser);

    return commKitUser;
}

std::shared_ptr<User> Self::currentUser() const
{
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
QFuture<Self::Result> Self::sendMessage(MessageHandler message)
{

    return QtConcurrent::run([this, message = std::move(message)]() -> Result {
        qCInfo(lcCoreMessenger) << "Trying to send message with id:" << QString(message->id());

        if (!isOnline()) {
            qCInfo(lcCoreMessenger) << "Trying to send message when offline";
            return Self::Result::Error_Offline;
        }

        switch (message->chatType()) {
        case ChatType::Personal:
            return sendPersonalMessage(message);

        case ChatType::Group:
            return sendGroupMessage(message);
        }
    });
}

Self::Result Self::sendPersonalMessage(const MessageHandler &message)
{

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
    //  Encrypt message.
    //
    auto messageData = packMessage(message);
    auto ciphertextDataMinLen = vssq_messenger_encrypted_message_len(m_impl->messenger.get(), messageData.size(),
                                                                     recipient->impl()->user.get());

    qCDebug(lcCoreMessenger) << "Message Len   :" << messageData.size();
    qCDebug(lcCoreMessenger) << "ciphertext Len:" << ciphertextDataMinLen;

    auto [ciphertextData, ciphertext] = makeMappedBuffer(ciphertextDataMinLen);

    const vssq_status_t encryptionStatus = vssq_messenger_encrypt_data(
            m_impl->messenger.get(), vsc_data_from(messageData), recipient->impl()->user.get(), ciphertext.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        qCWarning(lcCoreMessenger) << "Can not encrypt ciphertext:"
                                   << vsc_str_to_qstring(vssq_error_message_from_status(encryptionStatus));
        return Self::Result::Error_InvalidMessageCiphertext;
    }

    adjustMappedBuffer(ciphertext, ciphertextData);

    //
    //  Pack JSON body.
    //
    auto messageBody = packXmppMessageBody(ciphertextData, PushType::Alert);

    qCDebug(lcCoreMessenger) << "Will send XMPP message with body:" << messageBody;

    auto senderJid = userIdToJid(message->senderId());
    auto recipientJid = userIdToJid(UserId(message->chatId()));

    qCDebug(lcCoreMessenger) << "Will send XMPP message from JID:" << senderJid;
    qCDebug(lcCoreMessenger) << "Will send XMPP message to JID:" << recipientJid;

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
        qCDebug(lcCoreMessenger) << "XMPP message was sent:" << message->id();
        return Self::Result::Success;
    } else {
        qCWarning(lcCoreMessenger) << "Can not send message - XMPP send failed:" << message->id();
        return Self::Result::Error_SendMessageFailed;
    }
}

Self::Result Self::sendGroupMessage(const MessageHandler &message)
{

    qCDebug(lcCoreMessenger) << "Will send group message:" << message->id() << ", from user:" << message->senderId()
                             << ", to group:" << message->groupChatInfo()->groupId();
    ;

    //
    //  Encrypt message for a group.
    //
    auto groupId = message->groupChatInfo()->groupId();

    auto messageData = packMessage(message);

    auto encryptedMessageDataResult = encryptGroupMessage(groupId, messageData);

    if (auto status = std::get_if<Self::Result>(&encryptedMessageDataResult)) {
        qCDebug(lcCoreMessenger) << "Failed to send message with id:" << message->id();
        return *status;
    }

    auto encryptedMessageData = std::move(*std::get_if<QByteArray>(&encryptedMessageDataResult));

    //
    //  Pack JSON body.
    //
    auto messageBody = packXmppMessageBody(encryptedMessageData, PushType::Alert);

    qCDebug(lcCoreMessenger) << "Will send XMPP message with body:" << messageBody;

    auto senderJid = userIdToJid(message->senderId());
    auto groupJid = groupIdToJid(groupId);

    QXmppMessage xmppMessage(senderJid, groupJid, messageBody);
    xmppMessage.setId(message->id());
    xmppMessage.setStamp(message->createdAt());
    xmppMessage.setType(QXmppMessage::Type::GroupChat);
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
}

QFuture<Self::Result> Self::processReceivedXmppMessage(const QXmppMessage &xmppMessage)
{

    return QtConcurrent::run([this, xmppMessage]() -> Result {
        qCInfo(lcCoreMessenger) << "Received XMPP message";
        qCDebug(lcCoreMessenger) << "Received XMPP message:" << xmppMessage.id() << "from:" << xmppMessage.from();

        switch (xmppMessage.type()) {
        case QXmppMessage::Type::Chat:
            return processChatReceivedXmppMessage(xmppMessage);

        case QXmppMessage::Type::GroupChat:
            return processGroupChatReceivedXmppMessage(xmppMessage);

        case QXmppMessage::Type::Error:
            return processErrorXmppMessage(xmppMessage);

        default:
            break;
        }

        qCWarning(lcCoreMessenger) << "Got unexpected message of type:" << xmppMessage.type();
        qCDebug(lcCoreMessengerXMPP).noquote() << "Got unexpected message:" << toXmlString(xmppMessage);

        return Self::Result::Success;
    });
}

Self::Result Self::processChatReceivedXmppMessage(const QXmppMessage &xmppMessage)
{

    auto message = std::make_unique<IncomingMessage>();

    message->setId(MessageId(xmppMessage.id()));
    message->setRecipientId(userIdFromJid(xmppMessage.to()));
    message->setSenderId(userIdFromJid(xmppMessage.from()));
    message->setCreatedAt(xmppMessage.stamp().toLocalTime());

    if (message->recipientId() != currentUser()->id()) {
        qCWarning(lcCoreMessenger) << "Got message sent to an another account:" << message->recipientId();

        return Self::Result::Error_InvalidMessageRecipient;
    }

    //
    //  Decode message body from Base64 and JSON.
    //
    auto ciphertextResult = unpackXmppMessageBody(xmppMessage);
    if (auto status = std::get_if<CoreMessengerStatus>(&ciphertextResult)) {
        return *status;
    }

    auto ciphertext = std::move(*std::get_if<QByteArray>(&ciphertextResult));

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

    auto [plaintextData, plaintext] = makeMappedBuffer(plaintextDataMinLen);

    const vssq_status_t decryptionStatus = vssq_messenger_decrypt_data(
            m_impl->messenger.get(), vsc_data_from(ciphertext), sender->impl()->user.get(), plaintext.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        qCWarning(lcCoreMessenger) << "Can not decrypt ciphertext:"
                                   << vsc_str_to_qstring(vssq_error_message_from_status(decryptionStatus));
        return Self::Result::Error_InvalidMessageCiphertext;
    }

    adjustMappedBuffer(plaintext, plaintextData);

    qCInfo(lcCoreMessenger) << "Received XMPP message was decrypted";

    //
    //  Unpack message.
    //
    if (auto result = unpackMessage(plaintextData, *message); result != Self::Result::Success) {
        return result;
    }

    qCInfo(lcCoreMessenger) << "Received XMPP message was parsed";

    //
    //  Tell the world we got a message.
    //
    emit messageReceived(std::move(message));

    return Self::Result::Success;
}

Self::Result Self::processGroupChatReceivedXmppMessage(const QXmppMessage &xmppMessage)
{

    auto groupId = groupIdFromJid(xmppMessage.from());

    auto message = std::make_unique<IncomingMessage>();

    message->setId(MessageId(xmppMessage.id()));
    message->setRecipientId(userIdFromJid(xmppMessage.to()));
    message->setSenderId(groupUserIdFromJid(xmppMessage.from()));
    message->setCreatedAt(xmppMessage.stamp());
    message->setGroupChatInfo(std::make_unique<MessageGroupChatInfo>(groupId));

    if (message->recipientId() != currentUser()->id()) {
        qCWarning(lcCoreMessenger) << "Got message sent to an another account:" << message->recipientId();

        return Self::Result::Error_InvalidMessageRecipient;
    }

    //
    //  Decode message body from Base64 and JSON.
    //
    auto ciphertextResult = unpackXmppMessageBody(xmppMessage);
    if (auto status = std::get_if<CoreMessengerStatus>(&ciphertextResult)) {
        return *status;
    }

    auto ciphertext = std::move(*std::get_if<QByteArray>(&ciphertextResult));

    qCDebug(lcCoreMessenger) << "Received group message:" << message->id() << ", from user:" << message->senderId()
                             << ", to group:" << groupId;

    //
    //  Decrypt message.
    //
    auto decryptedMessageResult = decryptGroupMessage(groupId, message->senderId(), ciphertext);

    if (auto status = std::get_if<Self::Result>(&decryptedMessageResult)) {
        qCDebug(lcCoreMessenger) << "Can not decrypt a message for group:" << groupId
                                 << ", from:" << message->senderId();

        if (*status == Self::Result::Error_GroupNotFound) {
            message->setContent(MessageContentEncrypted(std::move(ciphertext)));
            emit messageReceived(std::move(message));
            return Self::Result::Success;
        }

        return *status;
    }

    auto plaintextData = std::move(*std::get_if<QByteArray>(&decryptedMessageResult));

    qCInfo(lcCoreMessenger) << "Received XMPP message was decrypted";

    //
    //  Unpack message.
    //
    if (auto result = unpackMessage(plaintextData, *message); result != Self::Result::Success) {
        return result;
    }

    qCInfo(lcCoreMessenger) << "Received XMPP message was parsed";

    //
    //  Tell the world we got a message.
    //
    emit messageReceived(std::move(message));

    return Self::Result::Success;
}

Self::Result Self::processErrorXmppMessage(const QXmppMessage &xmppMessage)
{

    qCDebug(lcCoreMessengerXMPP).noquote() << "Got error message:" << toXmlString(xmppMessage);

    //
    //  TODO: Emit error.
    //
    return Self::Result::Success;
}

QFuture<Self::Result> Self::processReceivedXmppCarbonMessage(const QXmppMessage &xmppMessage)
{

    //
    //  FIXME: fix for group chat carbons.
    //
    return QtConcurrent::run([this, xmppMessage]() -> Result {
        qCInfo(lcCoreMessenger) << "Received XMPP message copy";
        qCDebug(lcCoreMessenger) << "Received XMPP message copy:" << xmppMessage.id();

        auto senderId = userIdFromJid(xmppMessage.from());
        auto recipientId = userIdFromJid(xmppMessage.to());

        if (currentUser()->id() != senderId) {
            qCWarning(lcCoreMessenger) << "Got message carbons copy from an another account:" << senderId;
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

            QString xmlStr;
            QXmlStreamWriter xmlWriter(&xmlStr);
            xmppMessage.toXml(&xmlWriter);
            qCDebug(lcCoreMessenger).noquote() << "Got invalid XMPP message:" << xmlStr;

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
        vsc_buffer_use(plaintext.get(), (byte *)plaintextData.data(), plaintextData.size());

        const vssq_status_t decryptionStatus = vssq_messenger_decrypt_data(
                m_impl->messenger.get(), vsc_data_from(ciphertext), sender->impl()->user.get(), plaintext.get());

        if (decryptionStatus != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessenger) << "Can not decrypt ciphertext:"
                                       << vsc_str_to_qstring(vssq_error_message_from_status(decryptionStatus));
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
            qCWarning(lcCoreMessenger) << "Got invalid message - unsupported version, expected v3, got" << version;
            return Self::Result::Error_InvalidMessageVersion;
        }

        const auto timestamp = messageJson["timestamp"].toVariant().value<uint>();
        if (timestamp != xmppMessage.stamp().toTime_t()) {
            qCWarning(lcCoreMessenger) << "Got invalid message - timestamp mismatch. Expected " << timestamp << ", xmpp"
                                       << xmppMessage.stamp().toTime_t();
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
            qCWarning(lcCoreMessenger) << "Got message without 'to' username";
            //
            //  TODO: Check if next line is redundant.
            //
            message->setRecipientUsername(currentUser()->username());
        } else {

            message->setRecipientUsername(std::move(recipientUsername));
        }

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

std::tuple<Self::Result, QByteArray, QByteArray> Self::encryptFile(const QString &sourceFilePath,
                                                                   const QString &destFilePath)
{
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> std::tuple<Result, QByteArray, QByteArray> {
        qCWarning(lcCoreMessenger) << "Can not encrypt file:"
                                   << vsc_str_to_qstring(vssq_error_message_from_status(status));

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

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(Self::Result::Error_FileEncryptionWriteFailed);
    }

    //
    //  Encrypt - Step 4 - Encrypt file.
    //
    qint64 processedFileSize = 0;
    while (!sourceFile.atEnd()) {
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
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()),
                           (qint64)vsc_buffer_len(workingBuffer.get()))
            == -1) {
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

    const auto signatureBufferLen =
            vssq_messenger_file_cipher_finish_encryption_signature_len(fileCipher.get(), ownerPrivateKey);
    auto [signature, signatureBuffer] = makeMappedBuffer(signatureBufferLen);

    encryptionStatus = vssq_messenger_file_cipher_finish_encryption(fileCipher.get(), ownerPrivateKey,
                                                                    workingBuffer.get(), signatureBuffer.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(Self::Result::Error_FileEncryptionWriteFailed);
    }

    //
    //  Return result.
    //
    return std::make_tuple(Self::Result::Success, std::move(decryptionKey), std::move(signature));
}

Self::Result Self::decryptFile(const QString &sourceFilePath, const QString &destFilePath,
                               const QByteArray &decryptionKey, const QByteArray &signature, const UserId senderId)
{
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> Self::Result {
        qCWarning(lcCoreMessenger) << "Can not decrypt file:"
                                   << vsc_str_to_qstring(vssq_error_message_from_status(status));

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
    decryptionStatus = vssq_messenger_file_cipher_start_decryption(fileCipher.get(), vsc_data_from(decryptionKey),
                                                                   vsc_data_from(signature));

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    //
    //  Decrypt - Step 3 - Decrypt file.
    //
    auto workingBuffer = vsc_buffer_wrap_ptr(vsc_buffer_new());

    qint64 processedFileSize = 0;
    while (!sourceFile.atEnd()) {
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
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()),
                           (qint64)vsc_buffer_len(workingBuffer.get()))
            == -1) {
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
    decryptionStatus =
            vssq_messenger_file_cipher_finish_decryption(fileCipher.get(), senderPublicKey, workingBuffer.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(Self::Result::Error_FileDecryptionWriteFailed);
    }

    return Self::Result::Success;
}

// --------------------------------------------------------------------------
//  XMPP event handlers.
// --------------------------------------------------------------------------
void Self::xmppOnConnected()
{
    m_impl->lastActivityManager->setEnabled(true);

    //
    //  TODO: Extract to a method
    //
    for (auto xmppRoom : m_impl->xmppGroupChatManager->rooms()) {
        if (xmppRoom->isJoined()) {
            qCDebug(lcCoreMessenger) << "XMPP room is already joined:" << xmppRoom->jid();
        } else {
            qCDebug(lcCoreMessenger) << "Re-join XMPP room:" << xmppRoom->jid();
            xmppRoom->join();
        }
    }

    changeConnectionState(Self::ConnectionState::Connected);

    //
    //  TODO: get available status from the cache as described within XEP-0318
    //
    QXmppPresence presenceOnline(QXmppPresence::Available);
    presenceOnline.setAvailableStatusType(QXmppPresence::Online);
    m_impl->xmpp->setClientPresence(presenceOnline);

    registerPushNotifications();
}

void Self::xmppOnDisconnected()
{
    m_impl->lastActivityManager->setEnabled(false);

    changeConnectionState(Self::ConnectionState::Disconnected);

    reconnectXmppServerIfNeeded();
}

void Self::xmppOnStateChanged(QXmppClient::State state)
{
    if (QXmppClient::ConnectingState == state) {
        changeConnectionState(Self::ConnectionState::Connecting);
    }
}

void Self::xmppOnError(QXmppClient::Error error)
{
    qCWarning(lcCoreMessenger) << "XMPP error:" << error;
    emit connectionStateChanged(Self::ConnectionState::Error);

    m_impl->xmpp->disconnectFromServer();

    // Wait 3 second and try to reconnect.
    QTimer::singleShot(3000, this, &Self::reconnectXmppServerIfNeeded);
}

void Self::xmppOnPresenceReceived(const QXmppPresence &presence)
{
    qCDebug(lcCoreMessengerXMPP).noquote() << "Got XMPP presence:" << toXmlString(presence);
}

void Self::xmppOnIqReceived(const QXmppIq &iq)
{
    qCDebug(lcCoreMessengerXMPP).noquote() << "Got XMPP IQ:" << toXmlString(iq);
}

void Self::xmppOnSslErrors(const QList<QSslError> &errors)
{
    qCWarning(lcCoreMessenger) << "XMPP SSL errors:" << errors;
    emit connectionStateChanged(Self::ConnectionState::Error);

    m_impl->xmpp->disconnectFromServer();

    // Wait 3 second and try to reconnect.
    QTimer::singleShot(3000, this, &Self::reconnectXmppServerIfNeeded);
}

void Self::xmppOnMessageReceived(const QXmppMessage &xmppMessage)
{
    if (xmppMessage.type() == QXmppMessage::Type::Normal) {
        //  Ignore normal messages.
        return;
    }

    //
    //  TODO: handle result.
    //
    processReceivedXmppMessage(xmppMessage);
}

void Self::xmppOnCarbonMessageReceived(const QXmppMessage &xmppMessage)
{
    //
    //  TODO: handle result.
    //
    processReceivedXmppCarbonMessage(xmppMessage);
}

void Self::xmppOnMessageDelivered(const QString &jid, const QString &messageId)
{
    qCDebug(lcCoreMessenger) << "Message delivered to:" << jid;
    OutgoingMessageStageUpdate update;
    update.messageId = MessageId(messageId);
    update.stage = OutgoingMessageStage::Delivered;
    emit updateMessage(update);
}

// --------------------------------------------------------------------------
//  Network event handlers.
// --------------------------------------------------------------------------
void Self::onProcessNetworkState(bool isOnline)
{
    if (isOnline) {
        emit reconnectXmppServerIfNeeded();
    } else {
        emit disconnectXmppServer();
    }
}

// --------------------------------------------------------------------------
//  LastActivityManager: controls and events.
// --------------------------------------------------------------------------
void Self::setCurrentRecipient(const UserId &recipientId)
{

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
bool Self::isUploadServiceFound() const
{
    return m_impl->xmppUploadManager->serviceFound();
}

QString Self::requestUploadSlot(const QString &filePath)
{
    return m_impl->xmppUploadManager->requestUploadSlot(QFileInfo(filePath));
}

void Self::xmppOnUploadServiceFound()
{

    const auto found = m_impl->xmppUploadManager->serviceFound();
    qCDebug(lcCoreMessenger) << "Upload service found:" << found;
    emit uploadServiceFound(found);
}

void Self::xmppOnUploadSlotReceived(const QXmppHttpUploadSlotIq &slot)
{
    emit uploadSlotReceived(slot.id(), slot.putUrl(), slot.getUrl());
}

void Self::xmppOnUploadRequestFailed(const QXmppHttpUploadRequestIq &request)
{

    const auto error = request.error();

    qCDebug(lcCoreMessenger)
            << QString("code(%1), condition(%2), text(%3)").arg(error.code()).arg(error.condition()).arg(error.text());

    if (error.condition() == QXmppStanza::Error::Condition::NotAcceptable && error.code() == 406) {
        emit uploadSlotErrorOccurred(request.id(), tr("File is larger than limit"));
    } else {
        emit uploadSlotErrorOccurred(request.id(), tr("Fail to upload file"));
    }
}

// --------------------------------------------------------------------------
//  Handle self events for debug purposes.
// --------------------------------------------------------------------------
void Self::onLogConnectionStateChanged(CoreMessenger::ConnectionState state)
{
    switch (state) {
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

// --------------------------------------------------------------------------
//  Message processing helpers.
// --------------------------------------------------------------------------
QByteArray Self::packMessage(const MessageHandler &message)
{
    //
    //  Pack message to JSON.
    //
    QJsonObject messageJson;
    messageJson.insert("version", "v3");
    messageJson.insert("timestamp", static_cast<qint64>(message->createdAt().toTime_t()));
    messageJson.insert("from", message->senderUsername());
    messageJson.insert("to", message->recipientUsername());
    messageJson.insert("content", MessageContentJsonUtils::to(message->content()));

    return MessageContentJsonUtils::toBytes(messageJson);
}

CoreMessengerStatus Self::unpackMessage(const QByteArray &messageData, Message &message)
{
    //
    //  Parse message.
    //
    const auto messageJsonDocument = QJsonDocument::fromJson(messageData);
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
        qCWarning(lcCoreMessenger) << "Got invalid message - unsupported version, expected v3, got" << version;
        return Self::Result::Error_InvalidMessageVersion;
    }

    const auto timestamp = messageJson["timestamp"].toVariant().value<uint>();
    if (auto xmppStamp = message.createdAt().toTime_t(); timestamp != xmppStamp) {
        qCWarning(lcCoreMessenger) << "Got invalid message - timestamp mismatch. Expected" << timestamp << ", xmpp"
                                   << xmppStamp;
        message.setCreatedAt(QDateTime::fromTime_t(timestamp));
    }

    //
    //  Get sender and recipient usernames.
    //
    auto senderUsername = messageJson["from"].toString();
    if (senderUsername.isEmpty()) {
        qCWarning(lcCoreMessenger) << "Got invalid message - missing 'from' username";
        return Self::Result::Error_InvalidMessageFormat;
    }

    message.setSenderUsername(std::move(senderUsername));

    auto recipientUsername = messageJson["to"].toString();
    if (recipientUsername.isEmpty()) {
        qCWarning(lcCoreMessenger) << "Got message without 'to' username";
        //
        //  TODO: Check if next line is redundant.
        //
        message.setRecipientUsername(currentUser()->username());
    } else {

        message.setRecipientUsername(std::move(recipientUsername));
    }

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

    message.setContent(std::move(content));

    //
    //  TODO: Parse addition fields related to group chats if exists.
    //

    return Self::Result::Success;
}

QByteArray Self::packXmppMessageBody(const QByteArray &messageCiphertext, PushType pushType)
{
    //
    //  Pack to JSON and return Base64 string.
    //
    QJsonObject messageBodyJson;

    switch (pushType) {
    case PushType::None:
        messageBodyJson["pushType"] = "none";
        break;

    case PushType::Alert:
        messageBodyJson["pushType"] = "alert";
        break;

    case PushType::Voip:
        messageBodyJson["pushType"] = "voip";
        break;
    }

    messageBodyJson["ciphertext"] = QString::fromLatin1(messageCiphertext.toBase64());

    return MessageContentJsonUtils::toBytes(messageBodyJson).toBase64();
}

std::variant<CoreMessengerStatus, QByteArray> Self::unpackXmppMessageBody(const QXmppMessage &xmppMessage)
{

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

    return *ciphertextDecoded;
}

// --------------------------------------------------------------------------
//  Cloud FS.
// --------------------------------------------------------------------------
CoreMessengerCloudFs Self::cloudFs() const
{
    auto cloudFsCopyPtr = vssq_messenger_cloud_fs_shallow_copy_const(vssq_messenger_cloud_fs(m_impl->messenger.get()));
    auto cloudFsCopy = vssq_messenger_cloud_fs_ptr_t(cloudFsCopyPtr, vssq_messenger_cloud_fs_delete);
    auto randomCopy = vscf_impl_wrap_ptr(vscf_impl_shallow_copy(m_impl->random.get()));
    return CoreMessengerCloudFs(std::move(cloudFsCopy), std::move(randomCopy), this);
}

// --------------------------------------------------------------------------
//  Group chats.
// --------------------------------------------------------------------------
void Self::onCreateGroupChat(const GroupHandler &group)
{

    QtConcurrent::run([this, group]() {
        //
        // Find participants.
        //
        qCInfo(lcCoreMessenger) << "Trying to create group chat";
        qCDebug(lcCoreMessenger) << "Crate group chat - start to find participants";
        Users userList;
        auto userListC = vssq_messenger_user_list_wrap_ptr(vssq_messenger_user_list_new());
        for (const auto &contact : group->contacts()) {
            //
            //  Find by username.
            //
            if (!contact->username().isEmpty()) {
                auto user = findUserByUsername(contact->username());
                if (user) {
                    userList.push_back(user);
                    vssq_messenger_user_list_add(userListC.get(), (vssq_messenger_user_t *)user->impl()->user.get());
                }
            }

            //
            //  TODO: Add contact by email and phone as well.
            //
        }

        qCDebug(lcCoreMessenger) << "Crate group chat - found" << userList.size() << " participants";
        for (const auto &user : userList) {
            qCDebug(lcCoreMessenger) << "Crate group chat - found participant:" << user->username();
        }

        if (!vssq_messenger_user_list_has_item(userListC.get())) {
            emit groupChatCreateFailed(group->id(), CoreMessengerStatus::Error_GroupNoParticipants);
            return;
        }

        //
        //  Tell the World that members are found.
        //
        auto owner = currentUser();
        emit updateGroup(AddGroupOwnersUpdate { group->id(), { owner } });
        emit updateGroup(AddGroupMembersUpdate { group->id(), userList });

        //
        //  Create Comm Kit group chat.
        //
        qCInfo(lcCoreMessenger) << "Trying to create group chat within CommKit";

        vssq_error_t error;
        vssq_error_reset(&error);

        auto groupIdStd = QString(group->id()).toStdString();
        auto groupC = vssq_messenger_group_wrap_ptr(vssq_messenger_create_group(
                m_impl->messenger.get(), vsc_str_from(groupIdStd), userListC.get(), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessenger) << "Got error status:"
                                       << vsc_str_to_qstring(vssq_error_message_from_error(&error));
            emit groupChatCreateFailed(group->id(), mapStatus(vssq_error_status(&error)));
            return;
        }

        {
            std::scoped_lock _(m_impl->groupMutex);
            m_impl->groupOwners[group->id()] = owner->id();
            m_impl->messengerGroups[group->id()] = std::make_shared<GroupImpl>(std::move(groupC));
        }

        //
        //  Tell the world that we have encryption keys for the group.
        //
        emit updateGroup(AddGroupUpdate { group->id() });

        //
        //  Create XMPP multi-user chat room, aka group chat.
        //
        xmppCreateGroupChat(group, userList);
    });
}

void Self::onJoinGroupChats(const GroupMembers &groupMembers)
{
    Q_ASSERT(m_impl->xmppGroupChatManager != nullptr);

    std::scoped_lock _(m_impl->groupMutex);

    for (const auto &groupMember : groupMembers) {
        Q_ASSERT(groupMember->memberId() == currentUser()->id());

        auto roomJid = groupIdToJid(groupMember->groupId());
        auto room = m_impl->xmppGroupChatManager->addRoom(roomJid);
        room->setNickName(groupMember->memberId());

        m_impl->groupOwners[groupMember->groupId()] = groupMember->groupOwnerId();

        connectXmppRoomSignals(room);

        if (isOnline()) {
            room->join();
        }
    }
}

void Self::onAcceptGroupInvitation(const GroupId &groupId, const UserId &groupOwnerId)
{

    auto roomJid = groupIdToJid(groupId);
    auto room = m_impl->xmppGroupChatManager->addRoom(roomJid);
    room->setNickName(currentUser()->id());

    m_impl->groupOwners[groupId] = groupOwnerId;

    connectXmppRoomSignals(room);

    emit updateGroup(GroupMemberAffiliationUpdate { groupId, currentUser()->id(), GroupAffiliation::Member });

    if (isOnline()) {
        room->join();
    }
}

void Self::onRejectGroupInvitation(const GroupId &groupId, const UserId &groupOwnerId)
{

    if (isOnline()) {
        QXmppElement xElement;
        xElement.setTagName("x");
        xElement.setAttribute("xmlns", "http://jabber.org/protocol/muc#user");

        QXmppElement declineElement;
        declineElement.setTagName("decline");
        declineElement.setAttribute("to", userIdToJid(groupOwnerId));

        xElement.appendChild(declineElement);

        QXmppMessage rejectInvitationMessage;
        rejectInvitationMessage.setTo(groupIdToJid(groupId));
        rejectInvitationMessage.setFrom(currentUserJid());
        rejectInvitationMessage.setReceiptRequested(false);

        rejectInvitationMessage.setExtensions(QXmppElementList() << xElement);

        m_impl->xmpp->sendPacket(rejectInvitationMessage);
    }
}

void Self::xmppOnCreateGroupChat(const GroupHandler &group, const Users &membersToBeInvited)
{
    qCInfo(lcCoreMessenger) << "Trying to create group chat within XMPP server";

    //
    //  Create XMPP multi-user chat room, aka group chat.
    //
    auto roomJid = groupIdToJid(group->id());

    QXmppMucRoom *room = m_impl->xmppGroupChatManager->addRoom(roomJid);
    room->setNickName(currentUser()->id());
    room->setSubject(group->name());

    connect(room, &QXmppMucRoom::joined, [this, group, currentUser = currentUser(), room, membersToBeInvited]() {
        qCDebug(lcCoreMessenger) << "Joined to the new XMPP room:" << room->jid();

        //
        //  Accept an Instant Room Configuration
        //
        QXmppMucOwnerIq acceptIq;
        acceptIq.setFrom(currentUserJid());
        acceptIq.setTo(room->jid());
        acceptIq.setType(QXmppIq::Type::Set);

        QXmppDataForm acceptForm;
        acceptForm.setType(QXmppDataForm::Type::Submit);

        acceptIq.setForm(acceptForm);

        if (!m_impl->xmpp->sendPacket(acceptIq)) {
            emit groupChatCreateFailed(group->id(), CoreMessengerStatus::Error_CreateGroup_XmppConfigFailed);
            return;
        }

        emit groupChatCreated(group->id());

        for (const auto &user : membersToBeInvited) {
            if (user->id() == currentUser->id()) {
                //  Do not add myself.
                continue;
            }

            //
            //  Send invitations.
            //
            auto invitationMessage = std::make_shared<OutgoingMessage>();
            invitationMessage->setId(MessageId::generate());
            invitationMessage->setSenderId(currentUser->id());
            invitationMessage->setSenderUsername(currentUser->username());
            invitationMessage->setRecipientId(user->id());
            invitationMessage->setRecipientUsername(user->username());
            invitationMessage->setContent(MessageContentGroupInvitation { group->name(), "Hello!" });
            invitationMessage->setGroupChatInfo(std::make_unique<MessageGroupChatInfo>(group->id()));
            invitationMessage->setCreatedNow();

            auto invitationMessageData = packMessage(invitationMessage);

            auto encryptedInvitationMessageResult = encryptGroupMessage(group->id(), invitationMessageData);

            if (auto status = std::get_if<Self::Result>(&encryptedInvitationMessageResult)) {
                qCDebug(lcCoreMessenger) << "User invitation was postponed:" << user->id();
                continue;
            }

            auto encryptedInvitationMessage = std::get_if<QByteArray>(&encryptedInvitationMessageResult)->toBase64();

            //
            //  Add to the room.
            //
            QXmppMucItem addMemberItem;
            addMemberItem.setAffiliation(QXmppMucItem::MemberAffiliation);
            addMemberItem.setJid(userIdToJid(user->id()));

            QXmppMucAdminIq requestAddMember;
            requestAddMember.setFrom(currentUserJid());
            requestAddMember.setTo(room->jid());
            requestAddMember.setItems({ addMemberItem });
            requestAddMember.setType(QXmppIq::Type::Set);

            if (m_impl->xmpp->sendPacket(requestAddMember)) {
                qCDebug(lcCoreMessenger) << "Requested to add user:" << user->id() << " the XMPP room:" << group->id();

            } else {
                qCDebug(lcCoreMessenger) << "User invitation was postponed:" << user->id();
                return;
            }

            if (room->sendInvitation(userIdToJid(user->id()), encryptedInvitationMessage)) {
                qCDebug(lcCoreMessenger) << "User was invited:" << user->id();
                invitationMessage->setStage(OutgoingMessageStage::Sent);
                emit messageReceived(invitationMessage);

            } else {
                qCDebug(lcCoreMessenger) << "User invitation was postponed:" << user->id();
            }
        }
    });

    connectXmppRoomSignals(room);

    room->join();
}

void Self::xmppOnMucInvitationReceived(const QString &roomJid, const QString &inviter, const QString &reason)
{

    qCDebug(lcCoreMessenger) << "Got XMPP room invitation:";
    qCDebug(lcCoreMessenger) << "    -> room:" << roomJid;
    qCDebug(lcCoreMessenger) << "    -> inviter:" << inviter;

    QtConcurrent::run([this, roomJid, inviter, reason]() {
        auto groupId = groupIdFromJid(roomJid);
        auto recipientId = currentUser()->id();
        auto senderId = userIdFromJid(inviter);

        //
        //  Decode from Base64.
        //
        auto maybeEncryptedInvitationMessage = QByteArray::fromBase64Encoding(reason.toLatin1());
        if (!maybeEncryptedInvitationMessage) {
            qDebug(lcCoreMessenger) << "Received invitation that is not Base64 encoded.";
            return;
        }

        auto encryptedInvitationMessage = std::move(*maybeEncryptedInvitationMessage);

        //
        //  Add inviter as group owner.
        //
        {
            std::scoped_lock<std::mutex> _(m_impl->groupMutex);
            m_impl->groupOwners[groupId] = senderId;
        }

        //
        //  Decrypt.
        //
        auto invitationMessage = std::make_shared<IncomingMessage>();
        invitationMessage->setId(MessageId::generate());
        invitationMessage->setSenderId(senderId);
        invitationMessage->setRecipientId(recipientId);
        invitationMessage->setGroupChatInfo(std::make_unique<MessageGroupChatInfo>(groupId));

        auto decryptedInvitationMessageResult = decryptGroupMessage(groupId, senderId, encryptedInvitationMessage);

        if (auto status = std::get_if<Self::Result>(&decryptedInvitationMessageResult)) {
            qCDebug(lcCoreMessenger) << "Invitation can not be decrypted, group:" << groupId
                                     << ", inviter:" << senderId;

            invitationMessage->setContent(MessageContentEncrypted(std::move(encryptedInvitationMessage)));

            emit messageReceived(invitationMessage);

            return;
        }

        qCDebug(lcCoreMessenger) << "Invitation was decrypted, group:" << groupId << ", inviter:" << senderId;

        //
        //  Unpack.
        //
        auto invitationMessageData = std::move(*std::get_if<1>(&decryptedInvitationMessageResult));

        if (auto status = unpackMessage(invitationMessageData, *invitationMessage); status != Self::Result::Success) {
            qCDebug(lcCoreMessenger) << "Failed to unpack invitation message";
            return;
        }

        emit messageReceived(invitationMessage);
    });
}

void Self::xmppOnMucRoomAdded(QXmppMucRoom *room)
{
    qCDebug(lcCoreMessenger) << "!!!!Room was added:" << room->jid();
}

void Self::xmppOnRoomParticipantReceived(const QString &roomJid, const QString &jid,
                                         QXmppMucItem::Affiliation affiliation)
{

    auto mapAffiliation = [](QXmppMucItem::Affiliation affiliation) {
        switch (affiliation) {
        case QXmppMucItem::UnspecifiedAffiliation:
            throw std::logic_error("Got UnspecifiedAffiliation when handle participantAffiliationChanged() signal.");

        case QXmppMucItem::OutcastAffiliation:
            return GroupAffiliation::Outcast;

        case QXmppMucItem::NoAffiliation:
            return GroupAffiliation::None;

        case QXmppMucItem::MemberAffiliation:
            return GroupAffiliation::Member;

        case QXmppMucItem::AdminAffiliation:
            return GroupAffiliation::Admin;

        case QXmppMucItem::OwnerAffiliation:
            return GroupAffiliation::Owner;
        }
    };

    emit updateGroup(
            GroupMemberAffiliationUpdate { groupIdFromJid(roomJid), userIdFromJid(jid), mapAffiliation(affiliation) });
}

void Self::xmppOnMucSubscribeReceived(const QString &roomJid, const QString &subscriberJid, const QString &nickname) { }

void Self::xmppOnMucSubscribedRoomReceived(const QString &id, const QString &roomJid, const QString &subscriberJid,
                                           const std::list<XmppMucSubEvent> &events)
{
}

void Self::xmppOnMucRoomSubscriberReceived(const QString &id, const QString &roomJid, const QString &subscriberJid,
                                           const std::list<XmppMucSubEvent> &events) {

};

void Self::xmppOnMucSubscribedRoomsProcessed(const QString &id) { }

void Self::xmppOnMucRoomSubscribersProcessed(const QString &id, const QString &roomJid) { }

void Self::connectXmppRoomSignals(QXmppMucRoom *room)
{

    qCDebug(lcCoreMessenger) << "Connecting room signals:" << room->jid();

    connect(room, &QXmppMucRoom::allowedActionsChanged,
            [room](QXmppMucRoom::Actions actions) { qCDebug(lcCoreMessenger) << "---> allowedActionsChanged:"; });

    connect(room, &QXmppMucRoom::configurationReceived, [room](const QXmppDataForm &configuration) {
        qCDebug(lcCoreMessengerXMPP).noquote() << "Room configuration received:" << toXmlString(configuration);
    });

    connect(room, &QXmppMucRoom::error, [room](const QXmppStanza::Error &error) {
        qCWarning(lcCoreMessenger) << "Got room:" << room->jid() << ", error:" << toXmlString(error);
    });

    connect(room, &QXmppMucRoom::joined, [this, room]() {
        auto roomId = groupIdFromJid(room->jid());

        qCDebug(lcCoreMessenger) << "Joined to the XMPP room:" << roomId;

        // TODO: Maybe replace it with MUC/Sub Subscribers mechanism?
        m_impl->xmppRoomParticipantsManager->requestAll(room->jid());

        m_impl->xmppMucSubManager->subscribe(
                {
                        XmppMucSubEvent::Messages,
                        XmppMucSubEvent::Affiliations,
                        XmppMucSubEvent::Subscribers,
                        XmppMucSubEvent::Presence,
                },
                currentUserJid(), room->jid(), currentUser()->id());
    });

    //
    //  TODO: Replace this events with MUC/Sub events.
    //
    connect(room, &QXmppMucRoom::kicked,
            [room](const QString &jid, const QString &reason) { qCDebug(lcCoreMessenger) << "---> kicked:"; });

    connect(room, &QXmppMucRoom::isJoinedChanged,
            [room]() { qCDebug(lcCoreMessenger) << "---> isJoinedChanged:" << room->isJoined(); });

    connect(room, &QXmppMucRoom::left, [room]() { qCDebug(lcCoreMessenger) << "---> left:"; });

    connect(room, &QXmppMucRoom::nameChanged,
            [room](const QString &name) { qCDebug(lcCoreMessenger) << "---> nameChanged:" << name; });

    connect(room, &QXmppMucRoom::nickNameChanged,
            [room](const QString &nickName) { qCDebug(lcCoreMessenger) << "---> nickNameChanged:" << nickName; });

    connect(room, &QXmppMucRoom::participantAdded,
            [room](const QString &jid) { qCDebug(lcCoreMessenger) << "---> participantAdded:" << jid; });

    connect(room, &QXmppMucRoom::participantChanged,
            [room](const QString &jid) { qCDebug(lcCoreMessenger) << "---> participantChanged:" << jid; });

    connect(room, &QXmppMucRoom::participantRemoved,
            [room](const QString &jid) { qCDebug(lcCoreMessenger) << "---> participantRemoved:" << jid; });

    connect(room, &QXmppMucRoom::participantsChanged,
            [room]() { qCDebug(lcCoreMessenger) << "---> participantsChanged"; });

    connect(room, &QXmppMucRoom::permissionsReceived, [room](const QList<QXmppMucItem> &permissions) {
        qCDebug(lcCoreMessenger) << "Room permissions received:" << room->jid();
        for (const auto &permission : permissions) {
            qCDebug(lcCoreMessengerXMPP).noquote() << "Room permission:" << toXmlString(permission);
        }
    });

    connect(room, &QXmppMucRoom::subjectChanged,
            [room](const QString &subject) { qCDebug(lcCoreMessenger) << "---> subjectChanged:" << subject; });
}

std::variant<CoreMessengerStatus, Self::GroupImplHandler> Self::findGroup(const GroupId &groupId) const
{
    qCDebug(lcCoreMessenger) << "Trying to find group with id:" << groupId;

    std::scoped_lock<std::mutex> _(m_impl->groupMutex);

    //
    //  Check cache first.
    //
    auto groupIt = m_impl->messengerGroups.find(groupId);
    if (groupIt != m_impl->messengerGroups.end()) {
        qCDebug(lcCoreMessenger) << "Group found in the cache";
        return groupIt->second;
    }

    //
    //  Search on-line.
    //
    if (!isOnline()) {
        return Self::Result::Error_Offline;
    }

    //
    //  Find group owner.
    //
    const auto groupOwnerIt = m_impl->groupOwners.find(groupId);
    if (groupOwnerIt == m_impl->groupOwners.end()) {
        qCCritical(lcCoreMessenger) << "Group owner is undefined when loading group:" << groupId;
        return Self::Result::Error_GroupOwnerNotFound;
    }

    const auto &groupOwnerId = groupOwnerIt->second;

    auto groupOwner = findUserById(groupOwnerId);
    if (!groupOwner) {
        qCDebug(lcCoreMessenger) << "Group owner is not found for group:" << groupId;
        return Self::Result::Error_GroupOwnerNotFound;
    }

    //
    //  Load a group.
    //
    vssq_error_t error;
    vssq_error_reset(&error);

    auto groupIdStdStr = QString(groupId).toStdString();

    auto group = vssq_messenger_group_wrap_ptr(vssq_messenger_load_group(
            m_impl->messenger.get(), vsc_str_from(groupIdStdStr), groupOwner->impl()->user.get(), &error));

    if (vssq_error_has_error(&error)) {
        qCDebug(lcCoreMessenger) << "Group not found:" << groupId;
        qCWarning(lcCoreMessenger) << "Got error status:" << vsc_str_to_qstring(vssq_error_message_from_error(&error));
        return Self::Result::Error_GroupNotFound;
    }

    qCDebug(lcCoreMessenger) << "Group found in the cloud:" << groupId;

    //
    //  Cache and return.
    //
    auto groupImpl = std::make_shared<GroupImpl>(std::move(group));

    m_impl->messengerGroups[groupId] = groupImpl;

    return groupImpl;
}

std::variant<CoreMessengerStatus, QByteArray> Self::encryptGroupMessage(const GroupId &groupId,
                                                                        const QByteArray &messageData)
{

    //
    //  Find and Load group.
    //
    auto groupResult = findGroup(groupId);
    if (auto status = std::get_if<0>(&groupResult)) {
        return *status;
    }

    auto group = std::move(*std::get_if<1>(&groupResult));

    //
    //  Encrypt message.
    //
    const auto outLen = vssq_messenger_group_encrypted_message_len(group->ctx.get(), messageData.size());
    auto [out, outBuf] = makeMappedBuffer(outLen);

    const auto status =
            vssq_messenger_group_encrypt_binary_message(group->ctx.get(), vsc_data_from(messageData), outBuf.get());

    if (status == vssq_status_SUCCESS) {
        adjustMappedBuffer(outBuf, out);
        return out;
    }

    return mapStatus(status);
}

std::variant<CoreMessengerStatus, QByteArray> Self::decryptGroupMessage(const GroupId &groupId, const UserId &senderId,
                                                                        const QByteArray &encryptedMessageData)
{

    //
    //  Find and Load group.
    //
    auto groupResult = findGroup(groupId);
    if (auto status = std::get_if<0>(&groupResult)) {
        return *status;
    }

    auto group = *std::get_if<1>(&groupResult);

    //
    //  Find sender.
    //
    auto sender = findUserById(senderId);
    if (!sender) {
        return Self::Result::Error_UserNotFound;
    }

    //
    //  Decrypt message.
    //
    const auto outLen = vssq_messenger_group_decrypted_message_len(group->ctx.get(), encryptedMessageData.size());
    auto [out, outBuf] = makeMappedBuffer(outLen);

    const auto status = vssq_messenger_group_decrypt_binary_message(
            group->ctx.get(), vsc_data_from(encryptedMessageData), sender->impl()->user.get(), outBuf.get());

    if (status == vssq_status_SUCCESS) {
        adjustMappedBuffer(outBuf, out);
        return out;
    } else {
        qCWarning(lcCoreMessenger) << "Failed to decrypt group message:"
                                   << vsc_str_to_qstring(vssq_error_message_from_status(status));
    }

    return mapStatus(status);
}
