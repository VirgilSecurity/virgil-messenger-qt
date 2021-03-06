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

#ifndef VM_MESSAGESTABLE_H
#define VM_MESSAGESTABLE_H

#include "core/DatabaseTable.h"
#include "GroupId.h"
#include "Message.h"
#include "Chat.h"

#include <optional>

namespace vm {
class MessagesTable : public DatabaseTable
{
    Q_OBJECT

public:
    explicit MessagesTable(Database *database);

signals:
    //
    //  Control signals.
    //
    void fetchChatMessages(const ChatId &chatId);
    void fetchNotSentMessages();
    void addMessage(const MessageHandler &message);
    void deleteChatMessages(const ChatId &chatId);

    void updateMessage(const MessageUpdate &messageUpdate);
    void markIncomingMessagesAsReadBeforeMessage(const MessageId &messageId);
    void markOutgoingMessagesAsReadBeforeMessage(const MessageId &messageId);

    //
    //  Notification signals.
    //
    void errorOccurred(const QString &errorText);
    void chatMessagesFetched(ModifiableMessages messages);
    void notSentMessagesFetched(ModifiableMessages messages);
    void messageAdded(const MessageHandler &message);
    void chatUnreadMessageCountChanged(const ChatId &chatId);

private:
    bool create() override;

    void onFetchChatMessages(const ChatId &chatId);
    void onFetchNotSentMessages();
    void onAddMessage(const MessageHandler &message);
    void onDeleteChatMessages(const ChatId &chatId);

    void onUpdateMessage(const MessageUpdate &messageUpdate);
    void onMarkIncomingMessagesAsReadBeforeMessage(const MessageId &messageId);
    void onMarkOutgoingMessagesAsReadBeforeMessage(const MessageId &messageId);
};
} // namespace vm

#endif // VM_MESSAGESTABLE_H
