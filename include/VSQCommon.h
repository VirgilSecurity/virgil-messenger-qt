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

#ifndef VM_COMMON_H
#define VM_COMMON_H

#include <functional>
#include <memory>

#include <QDateTime>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSize>
#include <QtGlobal>
#include <QUrl>
#include <QVariant>

#include <thirdparty/optional/optional.hpp>

namespace args {
    using namespace std::placeholders;
}

using DataSize = qint64;
Q_DECLARE_METATYPE(DataSize);

using Seconds = quint64;
Q_DECLARE_METATYPE(Seconds);

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
        Picture
    };
    Q_ENUM_NS(AttachmentType)

    enum class AttachmentStatus
    {
        Created,
        Preloading,
        Loading,
        Postloading,
        Loaded,
        Interrupted, // must be re-loaded
        Invalid // Can't be processed
    };
    Q_ENUM_NS(AttachmentStatus)

    enum class MessageAuthorV0
    {
        // User is message author
        User,
        // Contact is message author
        Contact
    };
    Q_ENUM_NS(MessageAuthorV0)

    enum class MessageStatusV0
    {
        // Created by user
        MST_CREATED,
        // Sent by user
        MST_SENT,
        // Receiver by contact
        MST_RECEIVED,
        // Read by contact
        MST_READ,
        // Failed to send by user
        MST_FAILED
    };
    Q_ENUM_NS(MessageStatusV0)

    enum class MessageStatus
    {
        Created, // Created by user
        Sent, // Sent by user
        Delivered, // Delivered to contact / Sent by contact
        Read, // Read by contact
        Failed, // Failed to send by user
        Invalid // Can't be sent
    };
    Q_ENUM_NS(MessageStatus)
}

// TODO(fpohtmeh): remove this old class
struct AttachmentV0
{
    using Type = Enums::AttachmentType;
    using Status = Enums::AttachmentStatus;

    QString id;
    Type type = Type::File;
    QString filePath; // raw
    QString fileName;
    QString displayName;
    QUrl remoteUrl; // encrypted
    // Thumbnail
    QString thumbnailPath; // raw
    QUrl remoteThumbnailUrl; // encrypted
    QSize thumbnailSize;
    // Status
    DataSize bytesTotal = 0; // encrypted
    DataSize bytesLoaded = 0; // encrypted
    Status status = Status::Created;
};
Q_DECLARE_METATYPE(AttachmentV0)

using OptionalAttachmentV0 = Optional<AttachmentV0>;
Q_DECLARE_METATYPE(OptionalAttachmentV0)

struct MessageV0
{
    using Author = Enums::MessageAuthorV0;
    using Status = Enums::MessageStatusV0;

    QString messageId;
    QString message;
    QString sender;
    QString recipient;
    OptionalAttachmentV0 attachment;
};
Q_DECLARE_METATYPE(MessageV0);

namespace vm
{
struct Contact
{
    enum class Type
    {
        Person,
        Group
    };

    using Id = QString;

    Id id;
    Type type = Type::Person;
    QString name;
    QUrl avatarUrl;
};

using UserId = QString;
using MessageId = QString;
using ChatId = QString;
using Jid = QString;

struct PictureExtras
{
    QSize size;
    int orientation; // same as QImageIOHandler::Transformation
    QSize thumbnailSize;
    QString thumbnailPath;
    QString previewPath;
};

struct Attachment
{
    using Id = QString;
    using Type = Enums::AttachmentType;
    using Status = Enums::AttachmentStatus;

    Id id;
    MessageId messageId;
    Type type = Type::File;
    Status status = Status::Created;
    QString fileName;
    DataSize size = 0;
    DataSize bytesTotal = 0; // after encryption
    DataSize bytesLoaded = 0;
    QString localPath;
    QUrl url;
    QVariant extras;
};

struct Message
{
    using Id = MessageId;
    using Status = Enums::MessageStatus;

    Id id;
    QDateTime timestamp;
    ChatId chatId;
    Contact::Id authorId;
    Status status = Status::Created;
    QString body;
    Optional<Attachment> attachment;
};

using Messages = std::vector<Message>;

struct Chat
{
    using Id = QString;

    Id id;
    QDateTime timestamp;
    Contact::Id contactId;
    Optional<Message> lastMessage;
    uint unreadMessageCount = 0;
};

using Chats = std::vector<Chat>;
}

Q_DECLARE_METATYPE(vm::Contact::Type)
Q_DECLARE_METATYPE(vm::PictureExtras)
Q_DECLARE_METATYPE(vm::Attachment::Type)
Q_DECLARE_METATYPE(vm::Attachment::Status)
Q_DECLARE_METATYPE(vm::Message::Status)
Q_DECLARE_METATYPE(vm::Message)
Q_DECLARE_METATYPE(vm::Messages)
Q_DECLARE_METATYPE(vm::Chat)
Q_DECLARE_METATYPE(vm::Chats)

void registerCommonMetaTypes();

#endif // VM_COMMON_H
