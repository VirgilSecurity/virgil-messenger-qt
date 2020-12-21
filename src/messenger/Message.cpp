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


ChatId Self::chatId() const {
    return m_chatId;
}


void Self::setChatId(ChatId chatId) {
    m_chatId = std::move(chatId);
}


ChatType Self::chatType() const {
    return m_chatType;
}


void Self::setChatType(ChatType chatType) {
    m_chatType = chatType;
}


UserId Self::senderId() const {
    return m_senderId;
}


void Self::setSenderId(UserId userId) {
    m_senderId = std::move(userId);
}


UserId Self::recipientId() const {
    return UserId(m_chatId);
}


QDateTime Self::createdAt() const {
    return m_createdAt;
}


void Self::setCreatedAt(QDateTime createdAt) {
    m_createdAt = std::move(createdAt);
}


const MessageContent& Self::content() const {
    return m_content;
}


MessageContent& Self::content() {
    return m_content;
}


MessageContentType Self::contentType() const noexcept {
    return MessageContentTypeFrom(m_content);
}


void Self::setContent(MessageContent content) {
    m_content = std::move(content);
}


std::shared_ptr<MessageGroupChatInfo> Self::groupChatInfo() const {
    return m_groupChatInfo;
}


void Self::setGroupChatInfo(std::shared_ptr<MessageGroupChatInfo> groupChatInfo) {
    m_groupChatInfo = std::move(groupChatInfo);
}


Self::Status Self::status() const noexcept {
    return m_status;
}


void Self::setStatus(Self::Status status) {
    m_status = status;
}


QString Self::statusString() const {
    return MessageStatusToString(m_status);
}

void Self::setStatusString(const QString statusString) {
    m_status = MessageStatusFromString(statusString);
}

qsizetype Self::attemptCount() const noexcept {
    return m_attemptCount;
}


bool Self::increaseAttemptCount() {
    if (m_attemptCount < 3) {
        ++m_attemptCount;
        return true;
    }

    return false;
}


void Self::resetAttemptCount() {
    m_attemptCount = 0;
}


bool Self::applyUpdate(const MessageUpdate& update) {
    if (auto statusUpdate = std::get_if<MessageStatusUpdate>(&update)) {
        m_status = statusUpdate->status;
        return true;
    }
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
