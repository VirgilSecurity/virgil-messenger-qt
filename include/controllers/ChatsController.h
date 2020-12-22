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

#include <QObject>

#include "VSQCommon.h"

namespace vm
{
class Models;
class UserDatabase;

class ChatsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Chat currentChat READ currentChat NOTIFY currentChatChanged)
    Q_PROPERTY(Contact::Id currentContactId READ currentContactId NOTIFY currentContactIdChanged)
    Q_PROPERTY(Chat::Id currentChatId READ currentChatId NOTIFY currentChatIdChanged)

public:
    ChatsController(Models *models, UserDatabase *userDatabase, QObject *parent);

    Chat currentChat() const;
    Contact::Id currentContactId() const;
    Chat::Id currentChatId() const;

    void loadChats(const UserId &userId);

    void createChat(const Contact::Id &contactId);
    void createGroupChat(const GroupId &groupId);
    void openChat(const Chat &chat);
    Q_INVOKABLE void openChatById(const Chat::Id &chatId);
    void closeChat();

    Q_INVOKABLE void addParticipant(const UserId &userId);
    Q_INVOKABLE void removeParticipant(const UserId &userId);
    Q_INVOKABLE void leaveGroup();

signals:
    void errorOccurred(const QString &errorText); // TODO(fpohtmeh): remove this signal everywhere?

    void chatsSet(const UserId &userId);

    void currentChatChanged(const Chat &chat);
    void currentContactIdChanged(const Contact::Id &contactId);
    void currentChatIdChanged(const Chat::Id &chatId);

    void chatOpened(const Chat &chat);
    void chatClosed();

    void newContactFound(const Contact::Id &contactId, QPrivateSignal);
    void groupChatCreated(const GroupId &groupId, QPrivateSignal);

private:
    void setupTableConnections();
    void setCurrentChat(const Chat &chat);

    void onNewContactFound(const Contact::Id &contactId);
    void onGroupChatCreated(const GroupId &groupId);
    void onChatsSet();

    Models *m_models;
    UserDatabase *m_userDatabase;
    UserId m_userId;
    Chat m_currentChat;
};
}

#endif // VM_CHATSSCONTROLLER_H
