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

#ifndef VM_CHAT_H
#define VM_CHAT_H

#include "ChatId.h"
#include "ChatType.h"
#include "Message.h"

#include <vector>
#include <memory>

namespace vm {

class Chat {

public:
    //
    //  Defines chat type.
    //
    using Type = ChatType;

    //
    //  Return chat unique identifier.
    //
    ChatId id() const;

    //
    //  Set chat unique identifier.
    //
    void setId(ChatId id);

    //
    //  Return human readable chat title.
    //
    QString title() const;

    //
    //  Set human readable chat title.
    //
    void setTitle(QString title);

    //
    //  Return chat type.
    //
    Type type() const;

    //
    //  Set chat type.
    //
    void setType(Type type);

    //
    //  Return timestamp when chat was created.
    //
    QDateTime createdAt() const;

    //
    //  Set timestamp when chat was created.
    //
    void setCreatedAt(QDateTime createdAt);

    //
    //  Return chat last message if exist, otherwise nullptr.
    //
    MessageHandler lastMessage() const;

    //
    //  Set chat last message if exist, otherwise nullptr.
    //
    void setLastMessage(MessageHandler lastMessage);

    //
    //  Return count of unread messages in chat.
    //
    qsizetype unreadMessageCount() const;

    //
    //  Set count of unread messages in chat.
    //
    void setUnreadMessageCount(qsizetype  unreadMessageCount);

private:
    ChatId m_id;
    QString m_title;
    Type m_type;
    QDateTime m_createdAt;
    std::shared_ptr<const Message> m_lastMessage;
    qsizetype m_unreadMessageCount;
};

using ChatHandler = std::shared_ptr<const Chat>;
using ModifiableChatHandler = std::shared_ptr<Chat>;
using ModifiableChats = std::vector<ModifiableChatHandler>;

} // namespace vm

#endif // VM_CHAT_H

