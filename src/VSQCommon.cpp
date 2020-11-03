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

#include "VSQCommon.h"

#include <QtQml>

#ifdef VS_DEVMODE
Q_LOGGING_CATEGORY(lcDev, "dev");
#endif
Q_LOGGING_CATEGORY(lcModel, "model");
Q_LOGGING_CATEGORY(lcController, "controller");

using namespace vm;

void registerCommonMetaTypes()
{
    qRegisterMetaType<Seconds>("Seconds");
    qRegisterMetaType<DataSize>("DataSize");
    qRegisterMetaType<Enums::AttachmentType>("Enums::AttachmentType");
    qRegisterMetaType<Enums::AttachmentStatus>("Enums::AttachmentStatus");
    qRegisterMetaType<Enums::MessageStatus>("Enums::MessageStatus");

    qRegisterMetaType<UserId>("UserId");
    qRegisterMetaType<Jid>("Jid");
    qRegisterMetaType<Contact::Id>("Contact::Id");
    qRegisterMetaType<Attachment::Id>("Attachment::Id");
    qRegisterMetaType<Attachment::Status>("Attachment::Status");
    qRegisterMetaType<Message>("Message");
    qRegisterMetaType<Message::Id>("Message::Id");
    qRegisterMetaType<Message::Status>("Message::Status");
    qRegisterMetaType<Messages>("Messages");
    qRegisterMetaType<Chat>("Chat");
    qRegisterMetaType<Chat::Id>("Chat::Id");
    qRegisterMetaType<Chat::UnreadCount>("Chat::UnreadCount");
    qRegisterMetaType<Chats>("Chats");
    qRegisterMetaType<GlobalMessage>("GlobalMessage");
    qRegisterMetaType<GlobalMessages>("GlobalMessages");

    qmlRegisterUncreatableMetaObject(Enums::staticMetaObject, "com.virgilsecurity.messenger", 1, 0, "Enums", "Not creatable as it is an enum type");
}

GlobalMessage::GlobalMessage(const Message &message, const UserId &userId, const Contact::Id &contactId,
                             const Contact::Id &senderId, const Contact::Id &recipientId)
    : Message(message)
    , userId(userId)
    , contactId(contactId)
    , senderId(senderId)
    , recipientId(recipientId)
{}
