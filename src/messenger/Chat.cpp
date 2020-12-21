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


#include "Chat.h"


using namespace vm;
using Self = Chat;


ChatId Self::id() const {
    return m_id;
}


void Self::setId(ChatId id) {
    m_id = std::move(id);
}


QString Self::title() const {
    return m_title;
}


void Self::setTitle(QString title) {
    m_title = std::move(title);
}


Self::Type Self::type() const {
    return m_type;
}


void Self::setType(Self::Type type) {
    m_type = type;
}


QDateTime Self::createdAt() const {
    return m_createdAt;
}


void Self::setCreatedAt(QDateTime createdAt) {
    m_createdAt = std::move(createdAt);
}


MessageHandler Self::lastMessage() const {
    return m_lastMessage;
}


void Self::setLastMessage(MessageHandler lastMessage) {
    m_lastMessage = std::move(lastMessage);
}


qsizetype Self::unreadMessageCount() const {
    return m_unreadMessageCount;
}


void Self::setUnreadMessageCount(qsizetype  unreadMessageCount) {
    m_unreadMessageCount = unreadMessageCount;
}
