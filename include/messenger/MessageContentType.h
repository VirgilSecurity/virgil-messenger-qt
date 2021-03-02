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

#ifndef VM_MESSAGE_CONTENT_TYPE_H
#define VM_MESSAGE_CONTENT_TYPE_H

#include "MessageContent.h"

#include <QString>

namespace vm {

//
//  Private helper to get compile time index of a variant.
//
namespace {
template<typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t variant_index()
{
    if constexpr (index == std::variant_size_v<VariantType>) {
        return index;
    } else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
        return index;
    } else {
        return variant_index<VariantType, T, index + 1>();
    }
}
} // namespace

//
//  Defines content type of a message.
//  Indexes are equal to the MessageContent variant indexes.
//
enum class MessageContentType : size_t {
    None = variant_index<MessageContent, std::monostate>(),
    Encrypted = variant_index<MessageContent, MessageContentEncrypted>(),
    Text = variant_index<MessageContent, MessageContentText>(),
    Picture = variant_index<MessageContent, MessageContentPicture>(),
    File = variant_index<MessageContent, MessageContentFile>(),
    GroupInvitation = variant_index<MessageContent, MessageContentGroupInvitation>(),
};

//
//  Return message content type from a given string.
//  Throws if correspond type is not found.
//
MessageContentType MessageContentTypeFrom(const QString &typeString);

//
//  Return message content type from a given content.
//  Throws if correspond type is not found.
//
MessageContentType MessageContentTypeFrom(const MessageContent &messageContent);

//
//  Return string from a given message content type.
//
QString MessageContentTypeToString(MessageContentType type);

//
//  Return string from a given message content.
//
QString MessageContentTypeToString(const MessageContent &content);

} // namespace vm

#endif // VM_MESSAGE_CONTENT_TYPE_H
