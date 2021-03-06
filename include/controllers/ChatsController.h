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

#ifndef VM_CHATSCONTROLLER_H
#define VM_CHATSCONTROLLER_H

#include "Chat.h"
#include "ChatId.h"
#include "ChatObject.h"
#include "Group.h"
#include "GroupMember.h"
#include "GroupUpdate.h"
#include "MessageId.h"
#include "User.h"
#include "UserId.h"

#include <QObject>
#include <QPointer>

#include <memory>

namespace vm {
class Models;
class UserDatabase;
class Messenger;

class ChatsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ChatObject *current MEMBER m_chatObject CONSTANT)

public:
    ChatsController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent);

    void loadChats();
    void clearChats();

    void createChatWithUsername(const QString &username);
    void createChatWithUserId(const UserId &userId);
    void createGroupChat(const QString &groupName, const Contacts &contacts);

    void openChat(const ModifiableChatHandler &chat);
    Q_INVOKABLE void openChat(const QString &chatId); // can be used within QML only
    void createChat(const ModifiableChatHandler &chat);
    Q_INVOKABLE void closeChat();
    ChatHandler currentChat() const;

    Q_INVOKABLE void acceptGroupInvitation(const MessageHandler &invitationMessage);
    Q_INVOKABLE void rejectGroupInvitation(const MessageHandler &invitationMessage);

    Q_INVOKABLE void loadGroupMembers();
    Q_INVOKABLE void addMembers(const Contacts &contacts);
    Q_INVOKABLE void removeSelectedMembers();

signals:
    void errorOccurred(const QString &errorText); // TODO(fpohtmeh): remove this signal everywhere?
    void notificationCreated(const QString &notification, const bool error);

    void chatsLoaded();
    void chatOpened(const ChatHandler &chat);
    void chatCreated(const ChatHandler &chat);
    void chatClosed();

    void createChatWithUser(const UserHandler &user, QPrivateSignal);

    void groupInvitationRejected();
    void groupMembersAdded();

private:
    void setupTableConnections();
    void setCurrentChat(ModifiableChatHandler chat);

    void onChatsLoaded(ModifiableChats chats);
    void onCreateChatWithUser(const UserHandler &user);
    void onGroupsFetched(const Groups &groups);
    void onGroupMembersFetched(const GroupId &groupId, const GroupMembers &groupMembers);

    void onGroupChatCreated(const GroupHandler &group, const GroupMembers &groupMembers);
    void onGroupChatCreateFailed(const GroupId &chatId, const QString &errorText);
    void onUpdateGroup(const GroupUpdate &groupUpdate);
    void onNewGroupChatLoaded(const GroupHandler &group);
    void onDatabaseGroupAdded(const GroupHandler &group);

    void onMessageAdded(const MessageHandler &message);
    void onChatUnreadMessageCountUpdated(const ChatId &chatId, qsizetype unreadMessageCount);
    void onLastUnreadMessageBeforeItWasRead(const MessageHandler &message);

    QPointer<Messenger> m_messenger;
    QPointer<Models> m_models;
    QPointer<UserDatabase> m_userDatabase;
    QPointer<ChatObject> m_chatObject;
};
} // namespace vm

#endif // VM_CHATSSCONTROLLER_H
