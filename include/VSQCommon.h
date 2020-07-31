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

#ifndef VSQ_COMMON_H
#define VSQ_COMMON_H

#include <functional>
#include <memory>

#include <QDateTime>
#include <QLoggingCategory>
#include <QtGlobal>
#include <QUrl>

#include <QXmppMessage.h>

#include <optional/optional.hpp>

namespace args {
    using namespace std::placeholders;
}

using DataSize = unsigned int;
Q_DECLARE_METATYPE(DataSize);

template <class Type>
using Optional = tl::optional<Type>;
using OptionalType = tl::nullopt_t;

static constexpr OptionalType NullOptional(tl::nullopt);

#ifdef VS_DEVMODE
Q_DECLARE_LOGGING_CATEGORY(lcDev);
#endif

// Namespace for passing of enum values to QML
namespace Enums {
    Q_NAMESPACE

    enum class AttachmentType
    {
        File,
        Picture,
        Video,
        Audio
    };
    Q_ENUM_NS(AttachmentType)

    enum class MessageAuthor
    {
        // User is message author
        User,
        // Contact is message author
        Contact
    };
    Q_ENUM_NS(MessageAuthor)

    enum class MessageStatus
    {
        // Created by user
        Created,
        // Sent by user
        Sent,
        // Receiver by contact
        Received,
        // Read by contact
        Read,
        // Failed to send by user
        Failed
    };
    Q_ENUM_NS(MessageStatus)
}

struct Attachment
{
    using Type = Enums::AttachmentType;

    QString id;
    Type type = Type::File;
    QUrl remote_url;
    QUrl local_url;
    QUrl local_preview;
    DataSize size = 0;
    DataSize uploaded = 0;
    bool loadingFailed = false;

    QString fileName() const
    {
        if (!local_url.isEmpty())
            return local_url.fileName();
        if (!remote_url.isEmpty())
            return remote_url.fileName();
        return QLatin1String();
    }
};

using OptionalAttachment = Optional<Attachment>;
Q_DECLARE_METATYPE(OptionalAttachment)

struct Message
{
    using Author = Enums::MessageAuthor;
    using Status = Enums::MessageStatus;

    QString id;
    QDateTime timestamp;
    QString body;
    QString contact;
    Author author;
    OptionalAttachment attachment;
    Status status;
};
Q_DECLARE_METATYPE(Message);

struct Chat
{
    QString id;
    QString contact;
    QString lastMessageBody;
    QDateTime lastEventTimestamp;
    int unreadMessageCount;
};
Q_DECLARE_METATYPE(Chat);

// FIXME(fpohtmeh): remove this workaround
struct ExtMessage : Message
{
    ExtMessage() : Message() {}
    explicit ExtMessage(const Message &message) : Message((message)) {}

    QXmppMessage xmpp;
};
Q_DECLARE_METATYPE(ExtMessage);

void registerCommonTypes();

#endif // VSQ_COMMON_H
