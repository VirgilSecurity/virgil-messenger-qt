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
#include "Utils.h"


using namespace vm;
using Self = MessageContentJsonUtils;


QJsonObject Self::to(const MessageContent& messageContent) {
    QJsonObject mainObject;

    if (auto text = std::get_if<MessageContentText>(&messageContent)) {
        mainObject.insert(QLatin1String("text"), text->text());
    }
    else if (auto encrypted = std::get_if<MessageContentEncrypted>(&messageContent)) {
        mainObject.insert(QLatin1String("ciphertext"), toBase64(encrypted->ciphertext()));
    }
    else if (auto file = std::get_if<MessageContentFile>(&messageContent)) {
        QJsonObject attachmentObject;
        writeAttachment(*file, attachmentObject);
        mainObject.insert(QLatin1String("file"), attachmentObject);
    }
    else if (auto picture = std::get_if<MessageContentPicture>(&messageContent)) {
        QJsonObject attachmentObject;
        writeAttachment(*picture, attachmentObject);
        writeExtras(*picture, false, attachmentObject);
        mainObject.insert(QLatin1String("picture"), attachmentObject);
    }
    else {
        throw std::logic_error("Invalid messageContent");
    }

    return mainObject;
}


QString Self::toString(const MessageContent& messageContent) {
    return Self::toBytes(messageContent);
}


QByteArray Self::toBytes(const MessageContent& messageContent) {
    return Self::toBytes(Self::to(messageContent));
}


QByteArray MessageContentJsonUtils::toBytes(const QJsonObject& jsonObject) {
    return QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
}


MessageContent Self::from(const QJsonObject& json, QString& errorString) {

    if (auto value = json[QLatin1String("text")]; !value.isUndefined()) {
        return MessageContentText(value.toString());
    }
    else if (auto value = json[QLatin1String("ciphertext")]; !value.isUndefined()) {
        return MessageContentEncrypted(fromBase64(value));
    }
    else if (auto value = json[QLatin1String("file")]; !value.isUndefined()) {
        MessageContentFile file;
        readAttachment(value.toObject(), file);
        return file;
    }
    else if (auto value = json[QLatin1String("picture")]; !value.isUndefined()) {
        MessageContentPicture picture;
        readAttachment(value.toObject(), picture);
        readExtras(value.toObject(), picture);
        return picture;
    }
    else {
        errorString = QObject::tr("Invalid messageContent json");
        return {};
    }
}


MessageContent Self::fromString(const QString& messageJsonString, QString& errorString) {
    return Self::fromBytes(messageJsonString.toUtf8(), errorString);
}


MessageContent Self::fromBytes(const QByteArray& messageJsonBytes, QString &errorString) {
    const auto json = QJsonDocument::fromJson(messageJsonBytes).object();
    return Self::from(json, errorString);
}


void Self::writeAttachment(const MessageContentAttachment& attachment, QJsonObject& json) {
    json.insert(QLatin1String("attachmentId"), QString(attachment.id()));
    json.insert(QLatin1String("fileName"), attachment.fileName());
    json.insert(QLatin1String("size"), attachment.size());
    json.insert(QLatin1String("remoteUrl"), attachment.remoteUrl().toString());
    json.insert(QLatin1String("encryptedSize"), attachment.encryptedSize());
    json.insert(QLatin1String("fingerprint"), attachment.fingerprint());
    json.insert(QLatin1String("decryptionKey"), toBase64(attachment.decryptionKey()));
    json.insert(QLatin1String("signature"), toBase64(attachment.signature()));
}


void Self::writeExtras(const MessageContentPicture& picture, const bool writeLocalPaths, QJsonObject& json) {
    const auto thumbnail = picture.thumbnail();
    json["thumbnailUrl"] = thumbnail.remoteUrl().toString();
    json["thumbnailEncryptedSize"] = thumbnail.encryptedSize();
    json["thumbnailDecryptionKey"] = toBase64(thumbnail.decryptionKey());
    json["thumbnailSignature"] = toBase64(thumbnail.signature());

    const auto thumbnailSize = picture.thumbnailSize();
    json["thumbnailWidth"] = thumbnailSize.width();
    json["thumbnailHeight"] = thumbnailSize.height();

    if (writeLocalPaths) {
        json["thumbnailPath"] = thumbnail.localPath();
        json["previewPath"] = picture.previewPath();
    }
}


bool Self::readExtras(const QString& str, MessageContentPicture &picture) {
    const auto json = QJsonDocument::fromJson(str.toUtf8()).object();
    return Self::readExtras(json, picture);
}


bool Self::readExtras(const QJsonObject& json, MessageContentPicture &picture) {
    // TODO: Check mandatory fields and return false if absent.

    MessageContentFile thumbnail;
    thumbnail.setLocalPath(json["thumbnailPath"].toString());
    thumbnail.setRemoteUrl(json["thumbnailUrl"].toString());
    thumbnail.setEncryptedSize(json["thumbnailEncryptedSize"].toInt());
    thumbnail.setDecryptionKey(fromBase64(json["thumbnailDecryptionKey"]));
    thumbnail.setSignature(fromBase64(json["thumbnailSignature"]));
    picture.setThumbnail(thumbnail);
    picture.setThumbnailSize(QSize(json["thumbnailWidth"].toInt(), json["thumbnailHeight"].toInt()));
    picture.setPreviewPath(json["previewPath"].toString());

    return true;
}


bool Self::readAttachment(const QJsonObject& jsonObject, MessageContentAttachment& attachment) {
    // TODO: Check mandatory fields and return false if absent.
    attachment.setId(AttachmentId(jsonObject[QLatin1String("attachmentId")].toString()));
    attachment.setFileName(jsonObject[QLatin1String("fileName")].toString());
    attachment.setSize(jsonObject[QLatin1String("size")].toInt());
    attachment.setRemoteUrl(jsonObject[QLatin1String("remoteUrl")].toString());
    attachment.setEncryptedSize(jsonObject[QLatin1String("encryptedSize")].toInt());
    attachment.setFingerprint(jsonObject[QLatin1String("fingerprint")].toString());
    attachment.setDecryptionKey(fromBase64(jsonObject[QLatin1String("decryptionKey")]));
    attachment.setSignature(fromBase64(jsonObject[QLatin1String("signature")]));

    return true;
}

QString MessageContentJsonUtils::toBase64(const QByteArray &bytes)
{
    return QString::fromUtf8(bytes.toBase64());
}

QByteArray MessageContentJsonUtils::fromBase64(const QVariant &str)
{
    return QByteArray::fromBase64(str.toString().toUtf8());
}
