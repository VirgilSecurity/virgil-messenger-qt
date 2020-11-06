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
#include <QImageIOHandler>
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
Q_DECLARE_LOGGING_CATEGORY(lcController);
Q_DECLARE_LOGGING_CATEGORY(lcModel);

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
        Loading,
        Loaded,
        Interrupted, // must be re-loaded
        Invalid // Can't be processed
    };
    Q_ENUM_NS(AttachmentStatus)

    // NOTE(fpohtmeh): some statuses have suffix M to avoid QML collisions
    // with AttachmentStatus
    enum class MessageStatus
    {
        Created, // Created by user
        Sent, // Sent by user
        Delivered, // Delivered to contact / Sent by contact
        Read, // Read by contact
        Failed, // Failed to send by user
        InvalidM // Can't be sent
    };
    Q_ENUM_NS(MessageStatus)
}

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
    QSize thumbnailSize;
    QUrl thumbnailUrl;
    QString thumbnailPath;
    QString previewPath;
    DataSize encryptedThumbnailSize = 0;

    bool operator==(const PictureExtras &e) const
    {
        return thumbnailSize == e.thumbnailSize && previewPath == e.previewPath && thumbnailPath == e.thumbnailPath && encryptedThumbnailSize == e.encryptedThumbnailSize;
    }
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
    DataSize encryptedSize = 0;
    QString localPath;
    QUrl url;
    QVariant extras;

    DataSize processedSize = 0;
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
    using UnreadCount = quint32;

    Id id;
    QDateTime timestamp;
    Contact::Id contactId;
    Optional<Message> lastMessage;
    UnreadCount unreadMessageCount = 0;
};

using Chats = std::vector<Chat>;

struct GlobalMessage : Message
{
    using Message::Message;

    GlobalMessage(const Message &message, const UserId &userId, const Contact::Id &contactId,
                  const Contact::Id &senderId, const Contact::Id &recipientId);

    UserId userId;
    Contact::Id contactId;
    Contact::Id senderId;
    Contact::Id recipientId;
};

using GlobalMessages = std::vector<GlobalMessage>;
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
Q_DECLARE_METATYPE(vm::GlobalMessage)
Q_DECLARE_METATYPE(vm::GlobalMessages)

void registerCommonMetaTypes();

#endif // VM_COMMON_H
