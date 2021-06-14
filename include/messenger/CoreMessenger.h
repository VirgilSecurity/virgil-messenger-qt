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

#ifndef VM_CORE_MESSENGER_H
#define VM_CORE_MESSENGER_H

#include "Chat.h"
#include "CloudFile.h"
#include "CloudFileMember.h"
#include "CoreMessengerCloudFs.h"
#include "CoreMessengerStatus.h"
#include "Group.h"
#include "Message.h"
#include "Settings.h"
#include "User.h"
#include "Group.h"
#include "GroupMember.h"
#include "GroupUpdate.h"
#include "Contact.h"
#include "ContactUpdate.h"
#include "xmpp/XmppMucSubIq.h"

#include <qxmpp/QXmppClient.h>
#include <qxmpp/QXmppHttpUploadIq.h>
#include <qxmpp/QXmppMucManager.h>
#include <qxmpp/QXmppResultSet.h>

#include <QObject>
#include <QFuture>
#include <QUrl>
#include <QPointer>

#include <memory>
#include <tuple>
#include <list>
#include <variant>

extern "C" {
//
//  Forward declaration C types.
//
typedef struct vssq_messenger_cloud_fs_t vssq_messenger_cloud_fs_t;
}

namespace vm {
class CoreMessenger : public QObject
{
    Q_OBJECT
private:
    enum class PushType { None, Alert, Voip };

public:
    using Result = CoreMessengerStatus;

    enum class ConnectionState { Disconnected, Connecting, Connected, Error };

    //
    //  Create a new group chat and became the owner.
    //  Note, it runs concurrently.
    //
    void createGroupChat(const QString &groupName, const Contacts &contacts);

    //
    //  Load existing group chats to be able send messages and if online then run groups synchronization.
    //  Note, it runs concurrently.
    //
    void loadGroupChats(const Groups &groups);

signals:
    //
    //  Should be called when application became activated.
    //
    void activate();

    //
    //  Should be called when application goes to the background.
    //
    void deactivate();

    //
    //  Should be called when application went to the background and was suspended.
    //
    void suspend();

    //
    //  Should be called when application shows and hide system dialog that suspends application.
    //
    void setShouldDisconnectWhenSuspended(bool disconnectWhenSuspended);

    //
    //  Info signals.
    //
    void connectionStateChanged(ConnectionState state);

    void lastActivityTextChanged(const QString &text);

    void messageReceived(ModifiableMessageHandler message);

    void updateMessage(const MessageUpdate &messageUpdate);

    void userWasFound(const UserHandler &user) const;

    void updateContact(const ContactUpdate &update) const;

    //
    //  Group Chats has signal / slot architecture.
    //--
    void acceptGroupInvitation(const GroupId &groupId, const UserId &groupOwnerId);
    void rejectGroupInvitation(const GroupId &groupId, const UserId &groupOwnerId);
    void renameGroupChat(const GroupId &groupId, const QString &groupName);

    void groupChatCreated(const GroupHandler &group, const GroupMembers &groupMembers);
    void groupChatCreateFailed(const GroupId &chatId, CoreMessengerStatus errorStatus);
    void updateGroup(const GroupUpdate &groupUpdate) const;

    void newGroupChatLoaded(const GroupHandler &group) const;
    // --

    //
    //  Message history control.
    // --
    void sendMessageStatusDisplayed(const MessageHandler &message);
    // --

    //
    //  Private signals, to resolve thread. issues.
    //
    void resetXmppConfiguration();
    void reconnectXmppServerIfNeeded();
    void disconnectXmppServer();
    void cleanupXmppMucRooms();
    void cleanupCommKitMessenger();
    void registerPushNotifications();
    void deregisterPushNotifications();
    void xmppCreateGroupChat(const GroupHandler &group, const GroupMembers &members);
    void xmppMessageDelivered(const QString &jid, const QString &messageId);
    void xmppFetchRoomsFromServer();
    void xmppJoinRoom(const GroupId &groupId);

    void syncPrivateChatsHistory();
    void syncGroupChatHistory(const GroupId &groupId);

public:
    //
    //  Create.
    //
    explicit CoreMessenger(Settings *settings, QObject *parent = nullptr);
    ~CoreMessenger() noexcept;

    //
    //  Info / Controls.
    //

    //
    //  Return true if Internet connection is fine.
    //
    bool isNetworkOnline() const noexcept;

    //
    //  Return true if messenger has Internet connection with all services.
    //
    bool isOnline() const noexcept;

    //
    // Return true if a user is signed in.
    //
    bool isSignedIn() const noexcept;

    //
    // Return true if a user is authenticated on the Messenger Services.
    //
    bool isAuthenticated() const noexcept;

    ConnectionState connectionState() const;

    //
    //  Sign-in / Sign-up / Backup.
    //
    QFuture<Result> signIn(const QString &username);
    QFuture<Result> signUp(const QString &username);
    QFuture<Result> signInWithBackupKey(const QString &username, const QString &password);
    QFuture<Result> backupKey(const QString &password);
    QFuture<Result> signOut();

    //
    // Users.
    //
    std::shared_ptr<User> findUserByUsername(const QString &username) const;
    std::shared_ptr<User> findUserById(const UserId &userId) const;
    std::shared_ptr<User> currentUser() const;

    //
    //  Messages.
    //
    QFuture<Result> sendMessage(MessageHandler message);

    //
    //  Try to decrypt given message for any local user.
    //  It is used to decrypt notification messages.
    //
    static std::variant<Result, MessageHandler> decryptStandaloneMessage(const Settings &settings,
                                                                         const QString &recipientJid,
                                                                         const QString &senderJid,
                                                                         const QString &ciphertext);

    //
    //  Encrypt given file and returns a key for decryption and signature.
    //
    std::tuple<Result, QByteArray, QByteArray> encryptFile(const QString &sourceFilePath, const QString &destFilePath);

    //
    //  Decrypt given file.
    //
    Result decryptFile(const QString &sourceFilePath, const QString &destFilePath, const QByteArray &decryptionKey,
                       const QByteArray &signature, const UserId senderId);

    //
    //  Contacts (XMPP).
    //
    bool subscribeToUser(const User &user);
    void setCurrentRecipient(const UserId &recipientId);

    //
    //  Cloud FS.
    //
    CoreMessengerCloudFs cloudFs() const;

    //
    //  Internal helpers.
    //
    QUrl getCrashReportEndpointUrl() const;
    QString getAuthHeaderVaue() const;

    //--
    //  File upload.
    //
public:
    bool isUploadServiceFound() const;
    QString requestUploadSlot(const QString &filePath);

signals:
    void uploadServiceFound(bool found);
    void uploadSlotReceived(const QString &slotId, const QUrl &putUrl, const QUrl &getUrl);
    void uploadSlotErrorOccurred(const QString &slotId, const QString &errorText);
    //
    //--

public:
    //
    //  Helpers.
    //
    static UserId userIdFromJid(const QString &jid);
    QString userIdToJid(const UserId &userId) const;
    QString currentUserJid() const;

    static QString groupChatsDomain();
    static QString groupIdToJid(const GroupId &userId);
    static GroupId groupIdFromJid(const QString &jid);
    static UserId groupUserIdFromJid(const QString &jid);

private:
    //
    //  Helper types.
    //
    class GroupImpl;
    using GroupImplHandler = std::shared_ptr<GroupImpl>;

private:
    //
    //  Configuration.
    //
    Result resetCommKitConfiguration();
    void connectXmppRoomSignals(QXmppMucRoom *room);

    //
    // Store helpers.
    //
    Result saveCurrentUserInfo();

    //
    //  Connection
    //
    void authenticate();
    void connectXmppServer();
    void changeConnectionState(ConnectionState state);
    Result finishSignIn();

    //
    //  Message processing helpers.
    //
    QByteArray packMessage(const MessageHandler &message);
    static CoreMessengerStatus unpackMessage(const QByteArray &messageData, Message &message);

    QByteArray packXmppMessageBody(const QByteArray &messageCiphertext, PushType pushType);
    static std::variant<CoreMessengerStatus, QByteArray> unpackXmppMessageBody(const QString &xmppMessageBody);

    //
    //  Message sending / processing helpers.
    //
    std::variant<CoreMessengerStatus, QByteArray> encryptPersonalMessage(const UserId &recipientId,
                                                                         const QByteArray &messageData);

    std::variant<CoreMessengerStatus, QByteArray> decryptPersonalMessage(const UserId &senderId,
                                                                         const QByteArray &encryptedMessageData);

    Result sendPersonalMessage(const MessageHandler &message);
    Result sendGroupMessage(const MessageHandler &message);

    QFuture<Result> processReceivedXmppMessage(const QXmppMessage &xmppMessage);
    Result processChatReceivedXmppMessage(const QXmppMessage &xmppMessage);
    Result processGroupChatReceivedXmppMessage(const QXmppMessage &xmppMessage);
    Result processErrorXmppMessage(const QXmppMessage &xmppMessage);

    Result processChatReceivedXmppCarbonMessage(const QXmppMessage &xmppMessage);
    Result processGroupChatReceivedXmppCarbonMessage(const QXmppMessage &xmppMessage);

    QFuture<Result> processReceivedXmppCarbonMessage(const QXmppMessage &xmppMessage);

    //
    //  Group helpers.
    //--
    CoreMessengerStatus loadGroup(const GroupId &groupId, const UserId &superOwnerId, bool emitNewGroup);

    void tryLoadGroupInBackground(const GroupId &groupId, const UserId &superOwnerId, bool emitNewGroup);

    bool isGroupCached(const GroupId &groupId) const;

    void syncLocalAndRemoteGroups();

    void updateGroupCache(const GroupImplHandler &group) const;

    const GroupImplHandler findGroupInCache(const GroupId &groupId) const;

    std::variant<CoreMessengerStatus, QByteArray> encryptGroupMessage(const GroupId &groupId,
                                                                      const QByteArray &messageData);

    std::variant<CoreMessengerStatus, QByteArray> decryptGroupMessage(const GroupId &groupId, const UserId &senderId,
                                                                      const QByteArray &encryptedMessageData);
    //--

    //
    //  Helpers.
    //
    bool isXmppConnected() const noexcept;
    bool isXmppConnecting() const noexcept;
    bool isXmppDisconnected() const noexcept;

    //
    //  XMPP helpers.
    //
    bool xmppRequestRoomConfiguration(const QString &roomJid);
    bool xmppSendRoomConfiguration(const QString &roomJid, const QString &roomName);

private slots:
    void onActivate();
    void onDeactivate();
    void onSuspend();
    void onSetShouldDisconnectWhenSuspended(bool disconnectWhenSuspended);
    void xmppOnConnected();
    void xmppOnDisconnected();
    void xmppOnStateChanged(QXmppClient::State state);
    void xmppOnError(QXmppClient::Error);
    void xmppOnPresenceReceived(const QXmppPresence &presence);
    void xmppOnIqReceived(const QXmppIq &iq);
    void xmppOnSslErrors(const QList<QSslError> &errors);
    void xmppOnMessageReceived(const QXmppMessage &xmppMessage);
    void xmppOnCarbonMessageReceived(const QXmppMessage &xmppMessage);
    void xmppOnMessageDelivered(const QString &jid, const QString &messageId);
    void xmppOnUploadServiceFound();
    void xmppOnUploadSlotReceived(const QXmppHttpUploadSlotIq &slot);
    void xmppOnUploadRequestFailed(const QXmppHttpUploadRequestIq &request);
    void xmppOnMucInvitationReceived(const QString &roomJid, const QString &inviter, const QString &reason);
    void xmppOnMucRoomAdded(QXmppMucRoom *room);
    void xmppOnCreateGroupChat(const GroupHandler &group, const GroupMembers &groupMembers);
    void xmppOnRoomParticipantReceived(const QString &roomJid, const QString &jid,
                                       QXmppMucItem::Affiliation affiliation);
    void xmppOnFetchRoomsFromServer();
    void xmppOnJoinRoom(const GroupId &groupId);

    void xmppOnArchivedMessageReceived(const QString &queryId, const QXmppMessage &message);
    void xmppOnArchivedResultsRecieved(const QString &queryId, const QXmppResultSetReply &resultSetReply,
                                       bool complete);
    //
    //  MUC/Sub slots.
    //--
    void xmppOnMucSubscribeReceived(const QString &roomJid, const QString &subscriberJid, const QString &nickname);

    void xmppOnMucSubscribedRoomsCountReceived(const QString &id, qsizetype totalRoomsCount);

    void xmppOnMucSubscribedRoomReceived(const QString &id, const QString &roomJid, const QString &subscriberJid,
                                         const std::list<XmppMucSubEvent> &events);

    void xmppOnMucSubscribedRoomsProcessed(const QString &id);

    void xmppOnMucRoomSubscriberReceived(const QString &id, const QString &roomJid, const QString &subscriberJid,
                                         const std::list<XmppMucSubEvent> &events);

    void xmppOnMucRoomSubscribersProcessed(const QString &id, const QString &roomJid);
    //--

    void onResetXmppConfiguration();
    void onReconnectXmppServerIfNeeded();
    void onDisconnectXmppServer();
    void onCleanupXmppMucRooms();
    void onCleanupCommKitMessenger();
    void onRegisterPushNotifications();
    void onDeregisterPushNotifications();

    void onAcceptGroupInvitation(const GroupId &groupId, const UserId &groupOwnerId);
    void onRejectGroupInvitation(const GroupId &groupId, const UserId &groupOwnerId);
    void onRenameGroupChat(const GroupId &groupId, const QString &groupName);
    void onGroupChatCreated(const GroupHandler &group, const GroupMembers &groupMembers);
    void onGroupChatCreateFailed(const GroupId &chatId, CoreMessengerStatus errorStatus);

    void onProcessNetworkState(bool online);
    void onLogConnectionStateChanged(CoreMessenger::ConnectionState state);

    void onSendMessageStatusDisplayed(const MessageHandler &message);

    void onSyncPrivateChatsHistory();
    void onSyncGroupChatHistory(const GroupId &groupId);

private:
    friend class CoreMessengerCloudFs;
    const vssq_messenger_cloud_fs_t *cloudFsC() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace vm

Q_DECLARE_METATYPE(vm::MessageHandler);
Q_DECLARE_METATYPE(vm::ModifiableMessageHandler);
Q_DECLARE_METATYPE(vm::Messages);
Q_DECLARE_METATYPE(vm::ModifiableMessages);
Q_DECLARE_METATYPE(vm::UserHandler);
Q_DECLARE_METATYPE(vm::ChatHandler);
Q_DECLARE_METATYPE(vm::ModifiableChatHandler);
Q_DECLARE_METATYPE(vm::ModifiableChats);
Q_DECLARE_METATYPE(vm::CloudFileHandler);
Q_DECLARE_METATYPE(vm::ModifiableCloudFileHandler);
Q_DECLARE_METATYPE(vm::CloudFiles);
Q_DECLARE_METATYPE(vm::ModifiableCloudFiles);
Q_DECLARE_METATYPE(vm::CloudFileMemberHandler);
Q_DECLARE_METATYPE(vm::CloudFileMembers);
Q_DECLARE_METATYPE(vm::ChatId);
Q_DECLARE_METATYPE(vm::MessageId);
Q_DECLARE_METATYPE(vm::AttachmentId);
Q_DECLARE_METATYPE(vm::CloudFileId);
Q_DECLARE_METATYPE(vm::Users);
Q_DECLARE_METATYPE(vm::UserId);
Q_DECLARE_METATYPE(vm::GroupId);
Q_DECLARE_METATYPE(vm::GroupMember);
Q_DECLARE_METATYPE(vm::GroupMembers);
Q_DECLARE_METATYPE(vm::Contact);
Q_DECLARE_METATYPE(vm::Contacts);
Q_DECLARE_METATYPE(vm::MessageUpdate);
Q_DECLARE_METATYPE(vm::ContactUpdate);

Q_DECLARE_METATYPE(QXmppClient::State);
Q_DECLARE_METATYPE(QXmppClient::Error);
Q_DECLARE_METATYPE(QXmppPresence);
Q_DECLARE_METATYPE(QXmppIq);
Q_DECLARE_METATYPE(QXmppHttpUploadSlotIq);
Q_DECLARE_METATYPE(QXmppHttpUploadRequestIq);
Q_DECLARE_METATYPE(QXmppMucItem::Affiliation);

#endif // VM_CORE_MESSENGER_H
