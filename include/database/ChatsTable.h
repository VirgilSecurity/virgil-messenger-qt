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

#ifndef VM_CHATSTABLE_H
#define VM_CHATSTABLE_H

#include "core/DatabaseTable.h"
#include "Chat.h"

namespace vm {
class ChatsTable : public DatabaseTable
{
    Q_OBJECT

public:
    explicit ChatsTable(Database *database);

signals:
    //
    //  Control signals.
    //--
    void fetch();
    void addChat(const ChatHandler &chat);
    void deleteChat(const ChatId &chatId);
    void requestChatUnreadMessageCount(const ChatId &chatId);
    void markMessagesAsRead(const ChatHandler &chat);
    //--

    //
    //  Info signals.
    //--
    void errorOccurred(const QString &errorText);
    void fetched(ModifiableChats chats);
    void chatUnreadMessageCountUpdated(const ChatId &chatId, qsizetype unreadMessageCount);
    //--

    //
    //  FIXME: Review and possible delete next 3 signals.
    //
    void updateLastMessage(const MessageHandler &message);
    void resetLastMessage(const ChatId &chatId);

private:
    bool create() override;

    void onFetch();
    void onAddChat(const ChatHandler &chat);
    void onDeleteChat(const ChatId &chatId);
    void onResetUnreadCount(const ChatHandler &chat);
    void onUpdateLastMessage(const MessageHandler &message);
    void onResetLastMessage(const ChatId &chatId);
    void onRequestChatUnreadMessageCount(const ChatId &chatId);
    void onMarkMessagesAsRead(const ChatHandler &chat);
};
} // namespace vm

#endif // VM_CHATSTABLE_H
