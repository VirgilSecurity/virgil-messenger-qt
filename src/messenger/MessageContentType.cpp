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

#include "MessageContentType.h"

using namespace vm;

MessageContentType vm::MessageContentTypeFrom(const QString &typeString)
{
    if (typeString == QLatin1String("none")) {
        return MessageContentType::None;
    } else if (typeString == QLatin1String("encrypted")) {
        return MessageContentType::Encrypted;
    } else if (typeString == QLatin1String("text")) {
        return MessageContentType::Text;
    } else if (typeString == QLatin1String("picture")) {
        return MessageContentType::Picture;
    } else if (typeString == QLatin1String("file")) {
        return MessageContentType::File;
    } else if (typeString == QLatin1String("group_invitation")) {
        return MessageContentType::GroupInvitation;
    } else {
        throw std::logic_error("Invalid MessageContentType string");
    }
}

MessageContentType vm::MessageContentTypeFrom(const MessageContent &messageContent)
{
    return static_cast<MessageContentType>(messageContent.index());
}

QString vm::MessageContentTypeToString(MessageContentType type)
{
    switch (type) {
    case MessageContentType::None:
        return QLatin1String("none");
    case MessageContentType::Encrypted:
        return QLatin1String("encrypted");
    case MessageContentType::Text:
        return QLatin1String("text");
    case MessageContentType::Picture:
        return QLatin1String("picture");
    case MessageContentType::File:
        return QLatin1String("file");
    case MessageContentType::GroupInvitation:
        return QLatin1String("group_invitation");
    default:
        throw std::logic_error("Invalid MessageContentType");
    }
}

QString vm::MessageContentTypeToString(const MessageContent &content)
{
    return MessageContentTypeToString(MessageContentTypeFrom(content));
}
