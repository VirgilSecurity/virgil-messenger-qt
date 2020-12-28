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


#include "Message.h"


using namespace vm;
using Self = Message;


MessageId Self::id() const {
    return m_id;
}


void Self::setId(MessageId id) {
    m_id = std::move(id);
}


UserId Self::senderId() const {
    return m_senderId;
}


void Self::setSenderId(UserId userId) {
    m_senderId = std::move(userId);
}


QString Self::senderUsername() const {
    return m_senderUsername;
}


void Self::setSenderUsername(QString username) {
    m_senderUsername = std::move(username);
}


UserId Self::recipientId() const {
    return m_recipientId;
}


void Self::setRecipientId(UserId userId) {
    m_recipientId = std::move(userId);
}


QString Self::recipientUsername() const {
    return m_recipientUsername;
}


void Self::setRecipientUsername(QString username) {
    m_recipientUsername = std::move(username);
}


ChatId Self::chatId() const {
    if (isGroupChatMessage()) {
        return ChatId(m_groupChatInfo->groupId());
    }
    else if (isIncoming()) {
        return ChatId(m_senderId);
    }
    else {
        return ChatId(m_recipientId);
    }
}


QString Self::chatTitle() const {
    if (isIncoming()) {
        return m_senderUsername;
    }
    else {
        return m_recipientUsername;
    }
}


ChatType Self::chatType() const {
    return m_chatType;
}


void Self::setChatType(ChatType chatType) {
    m_chatType = chatType;
}


QDateTime Self::createdAt() const {
    return m_createdAt;
}


void Self::setCreatedAt(QDateTime createdAt) {
    m_createdAt = std::move(createdAt);
}


const MessageContent& Self::content() const noexcept {
    return m_content;
}


MessageContent& Self::content() noexcept {
    return m_content;
}


MessageContentType Self::contentType() const noexcept {
    return MessageContentTypeFrom(m_content);
}


void Self::setContent(MessageContent content) {
    m_content = std::move(content);
}


bool Self::contentIsAttachment() const noexcept {
    const bool isFile = std::holds_alternative<MessageContentFile>(m_content);
    const bool isPicture = std::holds_alternative<MessageContentPicture>(m_content);
    return isFile || isPicture;
}


const MessageContentAttachment* Self::contentAsAttachment() const {
    if (auto file = std::get_if<MessageContentFile>(&m_content)) {
        return file;
    }
    else if (auto picture = std::get_if<MessageContentPicture>(&m_content)) {
        return picture;
    }
    else {
        throw std::logic_error("Message content is not an attachment.");
    }
}


MessageContentAttachment* Self::contentAsAttachment() {
    if (auto file = std::get_if<MessageContentFile>(&m_content)) {
        return file;
    }
    else if (auto picture = std::get_if<MessageContentPicture>(&m_content)) {
        return picture;
    }
    else {
        throw std::logic_error("Message content is not an attachment.");
    }
}


std::shared_ptr<MessageGroupChatInfo> Self::groupChatInfo() const {
    return m_groupChatInfo;
}


void Self::setGroupChatInfo(std::shared_ptr<MessageGroupChatInfo> groupChatInfo) {
    m_groupChatInfo = std::move(groupChatInfo);
}


QString Self::statusString() const {
    return MessageStatusToString(status());
}


bool Self::applyUpdate(const MessageUpdate& update) {
    Q_UNUSED(update)
    return false;
}


bool Self::isPersonal() const noexcept {
    return (nullptr == m_groupChatInfo) || m_groupChatInfo->isPrivate();
}


bool Self::isGroupChatMessage() const noexcept {
    return m_groupChatInfo != nullptr;
}


bool Self::isOutgoing() const noexcept {
    return false;
}


bool Self::isIncoming() const noexcept {
    return false;
}


bool Self::isOutgoingCopyFromOtherDevice() const noexcept {
    return m_isCarbonsCopy;
}


void Self::markAsOutgoingCopyFromOtherDevice() {
    m_isCarbonsCopy = true;
}
