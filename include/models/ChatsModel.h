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

#ifndef VM_CHATSMODEL_H
#define VM_CHATSMODEL_H

#include "ListModel.h"
#include "Chat.h"
#include "GroupUpdate.h"

#include <optional>

namespace vm
{
class ChatsModel : public ListModel
{
    Q_OBJECT

public:
    explicit ChatsModel(QObject *parent);
    ~ChatsModel() override;

    void setChats(ModifiableChats chats);
    void clearChats();

    void addChat(ModifiableChatHandler chat);
    void deleteChat(const ChatId &chatId);
    ChatHandler findChat(const ChatId &chatId) const;
    ModifiableChatHandler findChat(const ChatId &chatId);

    void resetUnreadCount(const ChatId &chatId);
    void updateLastMessage(const MessageHandler &message, qsizetype unreadMessageCount);
    void resetLastMessage(const ChatId &chatId);

    //
    // Update group chat UI.
    //
    void updateGroup(const GroupUpdate& groupUpdate);

signals:
    void chatAdded(const ChatHandler &chat);
    void chatUpdated(const ChatHandler &chat);

private:
    enum Roles
    {
        IdRole = Qt::UserRole,
        ContactIdRole,
        LastEventTimeRole,
        LastEventTimestampRole,
        LastMessageBodyRole,
        UnreadMessagesCountRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    std::optional<int> findRowById(const ChatId &chatId) const;

private:
    ModifiableChats m_chats;
};
} // namespace vm

#endif // VM_CHATSMODEL_H
