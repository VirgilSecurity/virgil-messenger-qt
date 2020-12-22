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


#include "MessageContentJsonUtils.h"


#include "MessageContentType.h"


using namespace vm;
using Self = MessageContentJsonUtils;


QJsonObject Self::to(const MessageContent& messageContent, const bool writeLocalPaths) {
    QJsonObject mainObject;
    mainObject.insert(QLatin1String("type"), MessageContentTypeToString(messageContent));
    if (auto text = std::get_if<MessageContentText>(&messageContent)) {
        mainObject.insert(QLatin1String("text"), text->text());
    }
    else if (auto encrypted = std::get_if<MessageContentEncrypted>(&messageContent)) {
        mainObject.insert(QLatin1String("ciphertext"), QString(encrypted->ciphertext().toBase64()));
    }
    else if (auto attachment = std::get_if<MessageContentAttachment>(&messageContent)) {
        QJsonObject attachmentObject;
        writeAttachment(*attachment, attachmentObject);
        QJsonObject extrasObject;
        if (auto picture = std::get_if<MessageContentPicture>(&messageContent)) {
            writeExtras(*picture, writeLocalPaths, extrasObject);
        }
        attachmentObject.insert(QLatin1String("extras"), extrasObject);
        mainObject.insert(QLatin1String("attachment"), attachmentObject);
    }
    else {
        throw std::logic_error("Invalid messageContent");
    }
    return mainObject;
}


QString Self::toString(const MessageContent& messageContent, const bool writeLocalPaths) {
    return toBytes(messageContent, writeLocalPaths);
}


QByteArray Self::toBytes(const MessageContent& messageContent, const bool writeLocalPaths) {
    return toBytes(to(messageContent, writeLocalPaths));
}


QByteArray MessageContentJsonUtils::toBytes(const QJsonObject& jsonObject) {
    return QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}


MessageContent Self::from(const QJsonObject& messageJsonObject, QString& errorString) {
    const auto type = MessageContentTypeFrom(messageJsonObject[QLatin1String("type")].toString());
    switch (type) {
        case MessageContentType::Text:
            return MessageContentText(messageJsonObject[QLatin1String("text")].toString());
        case MessageContentType::Encrypted: {
            const QByteArray cipherText = messageJsonObject[QLatin1String("ciphertext")].toString().toUtf8();
            return MessageContentEncrypted(QByteArray::fromBase64(cipherText));
        }
        case MessageContentType::File: {
            MessageContentFile file;
            const auto attachmentObject = messageJsonObject[QLatin1String("attachment")].toObject();
            readAttachment(attachmentObject, file);
            return file;
        }
        case MessageContentType::Picture: {
            MessageContentPicture picture;
            const auto attachmentObject = messageJsonObject[QLatin1String("attachment")].toObject();
            readAttachment(attachmentObject, picture);
            const auto extrasObject = attachmentObject[QLatin1String("extras")].toObject();
            readExtras(extrasObject, picture);
            return picture;
        }
        default:
            errorString = QObject::tr("Invalid messageContent json");
            return {};
    }
}


MessageContent Self::fromString(const QString& messageJsonString, QString& errorString) {
    return fromBytes(messageJsonString.toUtf8(), errorString);
}


MessageContent Self::fromBytes(const QByteArray& messageJsonBytes, QString &errorString) {
    const auto json = QJsonDocument::fromJson(messageJsonBytes).object();
    return from(json, errorString);
}


bool Self::readExtras(const QJsonObject& json, MessageContentPicture &picture) {
    MessageContentFile thumbnail;
    thumbnail.setLocalPath(json["thumbnailPath"].toString());
    thumbnail.setRemoteUrl(json["thumbnailUrl"].toString());
    thumbnail.setEncryptedSize(json["thumbnailEncryptedSize"].toInt());
    picture.setThumbnail(thumbnail);
    picture.setThumbnailSize(QSize(json["thumbnailWidth"].toInt(), json["thumbnailHeight"].toInt()));
    picture.setPreviewPath(json["previewPath"].toString());
	return true;
}


bool Self::readExtras(const QString& jsonString, MessageContentPicture& picture) {
    const auto json = QJsonDocument::fromJson(jsonString.toUtf8()).object();
    return readExtras(json, picture);
}


bool Self::writeExtras(const MessageContentPicture& picture, const bool writeLocalPaths, QJsonObject& json) {
    const auto thumbnail = picture.thumbnail();
    if (writeLocalPaths) {
        json["thumbnailPath"] = thumbnail.localPath();
    }
    json["thumbnailUrl"] = thumbnail.remoteUrl().toString();
    json["thumbnailEncryptedSize"] = thumbnail.encryptedSize();
    const auto thumbnailSize = picture.thumbnailSize();
    json["thumbnailWidth"] = thumbnailSize.width();
    json["thumbnailHeight"] = thumbnailSize.height();
    if (writeLocalPaths) {
        json["previewPath"] = picture.previewPath();
    }
	return true;
}

void Self::writeAttachment(const MessageContentAttachment& attachment, QJsonObject& jsonObject) {
    // FIXME(fpohtmeh): write/read id?
    jsonObject.insert(QLatin1String("fileName"), attachment.fileName());
    jsonObject.insert(QLatin1String("size"), attachment.size());
    jsonObject.insert(QLatin1String("remoteUrl"), attachment.remoteUrl().toString());
    jsonObject.insert(QLatin1String("encryptedSize"), attachment.encryptedSize());
    jsonObject.insert(QLatin1String("fingerprint"), attachment.fingerprint());
}

void Self::readAttachment(const QJsonObject& jsonObject, MessageContentAttachment& attachment) {
    attachment.setFileName(jsonObject[QLatin1String("fileName")].toString());
    attachment.setSize(jsonObject[QLatin1String("size")].toInt());
    attachment.setRemoteUrl(jsonObject[QLatin1String("remoteUrl")].toString());
    attachment.setEncryptedSize(jsonObject[QLatin1String("encryptedSize")].toInt());
    attachment.setFingerprint(jsonObject[QLatin1String("fingerprint")].toString());
}
