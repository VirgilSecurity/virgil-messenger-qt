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


#ifndef VM_MESSAGE_H
#define VM_MESSAGE_H

#include "MessageId.h"
#include "ChatId.h"
#include "ChatType.h"
#include "UserId.h"
#include "MessageContent.h"
#include "MessageContentType.h"
#include "MessageGroupChatInfo.h"
#include "MessageStatus.h"
#include "MessageUpdateable.h"

#include <QString>
#include <QDateTime>

#include <memory>
#include <list>

namespace vm {

class IncomingMessage;
class OutgoingMessage;

//
//  Base class for all messages.
//
class Message : public MessageUpdateable  {

public:
    using Status = MessageStatus;

    virtual ~Message() noexcept = default;

    //
    // Return message unique identifier.
    //
    MessageId id() const;

    //
    // Set message unique identifier.
    //
    void setId(MessageId id);

    //
    // Return a chat unique identifier the message belongs to.
    //
    ChatId chatId() const;

    //
    // Set a chat unique identifier the message belongs to.
    //
    void setChatId(ChatId chatId);

    //
    // Return a chat type the message belongs to.
    //
    ChatType chatType() const;

    //
    // Set a chat type the message belongs to.
    //
    void setChatType(ChatType chatType);

    //
    // Return message sender unique identifier.
    //
    UserId senderId() const;

    //
    // Set message sender unique identifier.
    //
    void setSenderId(UserId userId);

    //
    // Return message recipient unique identifier.
    //
    UserId recipientId() const;

    //
    // Return date when message was created.
    //
    QDateTime createdAt() const;

    //
    // Set date when message was created.
    //
    void setCreatedAt(QDateTime createdAt);

    //
    //  Return message content.
    //
    const MessageContent& content() const;

    //
    //  Return message content.
    //
    MessageContent& content();

    //
    //  Return message content type.
    //
    MessageContentType contentType() const noexcept;

    //
    //  Set message content.
    //
    void setContent(MessageContent content);

    //
    //  Return info related to a group chat message.
    //
    std::shared_ptr<MessageGroupChatInfo> groupChatInfo() const;

    //
    //  Set info related to a group chat message.
    //
    void setGroupChatInfo(std::shared_ptr<MessageGroupChatInfo> groupChatInfo);

    //
    //  Return current message status.
    //
    Status status() const noexcept;

    //
    //  Set current message status.
    //
    void setStatus(Status status);

    //
    //  Return current message status as string.
    //
    QString statusString() const;

    //
    //  Set current message status from string.
    //
    void setStatusString(const QString statusString);

    //
    //  Return the message stage as string.
    //
    virtual QString stageString() const = 0;

    //
    //  State the message stage from string.
    //
    virtual void setStageString(QString stageString) = 0;

    //
    //  Return processing message attempts count.
    //
    qsizetype attemptCount() const noexcept;

    //
    //  Increment processing message attempts count and return true if MAX was not reached.
    //
    bool increaseAttemptCount();

    //
    //  Reset processing message attempts count to 0.
    //
    void resetAttemptCount();

    //
    //  Apply message update. Return true some properties were actually updated.
    //
    bool applyUpdate(const MessageUpdate& update) override;

    //
    //  Return whether message is peer-to-peer or direct message from a group chat.
    //
    bool isPersonal() const noexcept;

    //
    //  Returns true if message was sent to the group or was received by using GroupChat,
    //  including direct messages.
    //
    bool isGroupChatMessage() const noexcept;

    //
    //  Return true if message is outgoing.
    //
    virtual bool isOutgoing() const noexcept;

    //
    //  Return true if message is incoming.
    //
    virtual bool isIncoming() const noexcept;

private:
    MessageId m_id;
    ChatId m_chatId;
    ChatType m_chatType;
    UserId m_senderId;
    QDateTime m_createdAt;
    MessageContent m_content;
    std::shared_ptr<MessageGroupChatInfo> m_groupChatInfo;
    qsizetype m_attemptCount;
    Status m_status;
};

using MessageHandler = std::shared_ptr<const Message>;
using ModifiableMessageHandler = std::shared_ptr<Message>;
using Messages = std::vector<MessageHandler>;
using ModifiableMessages = std::vector<ModifiableMessageHandler>;

} // namespace vm

#endif // VM_MESSAGE_H
