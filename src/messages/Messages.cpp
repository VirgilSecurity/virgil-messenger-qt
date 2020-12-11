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

#include "Messages.h"

#include "Utils.h"

#include <QtQml>

#ifdef VS_DEVMODE
Q_LOGGING_CATEGORY(lcDev, "dev");
#endif
Q_LOGGING_CATEGORY(lcModel, "model");
Q_LOGGING_CATEGORY(lcController, "controller");

using namespace vm;

void registerMessagesMetaTypes()
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
    qRegisterMetaType<Attachment::Type>("Attachment::Type");
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


QString MessageUtils::extrasToJson(const QVariant &extras, const Attachment::Type type, bool skipLocal)
{
    if (type != Attachment::Type::Picture) {
        return QString();
    }
    const auto e = extras.value<PictureExtras>();
    QJsonObject obj;
    obj.insert("thumbnailWidth", e.thumbnailSize.width());
    obj.insert("thumbnailHeight", e.thumbnailSize.height());
    obj.insert("thumbnailUrl", e.thumbnailUrl.toString());
    obj.insert("encryptedThumbnailSize", e.encryptedThumbnailSize);
    if (!skipLocal) {
        obj.insert("thumbnailPath", e.thumbnailPath);
        obj.insert("previewPath", e.previewPath);
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QVariant MessageUtils::extrasFromJson(const QString &json, const Attachment::Type type, bool skipLocal)
{
    if (type != Attachment::Type::Picture) {
        return QVariant();
    }
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    PictureExtras extras;
    extras.thumbnailSize.setWidth(doc["thumbnailWidth"].toInt());
    extras.thumbnailSize.setHeight(doc["thumbnailHeight"].toInt());
    extras.thumbnailUrl = doc["thumbnailUrl"].toString();
    extras.encryptedThumbnailSize = doc["encryptedThumbnailSize"].toInt();
    if (!skipLocal) {
        extras.thumbnailPath = doc["thumbnailPath"].toString();
        extras.previewPath = doc["previewPath"].toString();
    }
    return QVariant::fromValue(extras);
}

GlobalMessage MessageUtils::messageFromCommKitMessage(const CommKitMessage &commKitMessage)
{
    // Get message from JSON
    auto doc = QJsonDocument::fromJson(commKitMessage.body);
    const auto type = doc["type"].toString();
    const auto payloadObject = doc["payload"];

    GlobalMessage message;
    if (type == QLatin1String("text")) {
        message.body = payloadObject["body"].toString();
        return message;
    }

    const auto at = doc["attachment"];
    Attachment attachment;
    if (type == QLatin1String("picture")) {
        attachment.type = Attachment::Type::Picture;
    }
    else {
        attachment.type = Attachment::Type::File;
    }
    attachment.id = Utils::createUuid();
    attachment.fileName = at["fileName"].toString();
    attachment.size = at["size"].toInt();
    attachment.url = at["url"].toString();
    attachment.encryptedSize = at["encryptedSize"].toInt();
    attachment.fingerprint = at["fingerprint"].toString();
    attachment.extras = extrasFromJson(at["extras"].toString(), attachment.type, true);
    message.attachment = attachment;
    if (!message.body.isEmpty()) {
        qCDebug(lcUtils) << "Parsed JSON message:" << message.body;
    }

    // FIXME(sseroshtan): Check this after Messages will be redesigned.
    message.id = commKitMessage.id;
    message.timestamp = commKitMessage.timestamp;
    message.chatId = commKitMessage.senderId;
    message.authorId = commKitMessage.senderId;
    message.senderId = commKitMessage.senderId;
    message.recipientId = commKitMessage.recipientId;
    if (message.attachment) {
        message.attachment->messageId = message.id;
    }

    return message;
}

CommKitMessage MessageUtils::messageToCommKitMessage(const GlobalMessage &message)
{
    QJsonObject mainObject;
    QJsonObject payloadObject;
    const auto &attachment = message.attachment;
    if (!attachment) {
        mainObject.insert("type", "text");
        payloadObject.insert("body", message.body);
    }
    else {
        QJsonObject at;
        if (attachment->type == Attachment::Type::Picture) {
            mainObject.insert("type", "picture");
        }
        else {
            mainObject.insert("type", "file");
        }
        at.insert("fileName", attachment->fileName);
        at.insert("size", attachment->size);
        at.insert("url", attachment->url.toString());
        at.insert("encryptedSize", attachment->encryptedSize);
        at.insert("fingerprint", attachment->fingerprint);
        at.insert("extras", extrasToJson(attachment->extras, attachment->type, true));
        mainObject.insert("attachment", at);
    }
    mainObject.insert("payload", payloadObject);

    QJsonDocument doc(mainObject);

    CommKitMessage commKitMessage;
    commKitMessage.id = message.id;
    commKitMessage.recipientId = message.recipientId;
    commKitMessage.senderId = message.senderId;
    commKitMessage.body = doc.toJson(QJsonDocument::Compact);
    commKitMessage.timestamp = message.timestamp;

    return commKitMessage;
}
