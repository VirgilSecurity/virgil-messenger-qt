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


using namespace vm;
using Self = MessageContentJsonUtils;


QJsonObject Self::to(const MessageContent& messageContent) {
	// FIXME(fpohtmeh): implement
	return QJsonObject();
}


QString Self::toString(const MessageContent& messageContent) {
	// FIXME(fpohtmeh): implement
	return QString();
}


QByteArray Self::toBytes(const MessageContent& messageContent) {
	// FIXME(fpohtmeh): implement
	return QByteArray();
}


MessageContent Self::from(const QJsonObject& messageJsonObject, QString &errorString) {
	// FIXME(fpohtmeh): implement
	return MessageContent();
}


MessageContent Self::fromString(const QString& messageJsonString, QString &errorString) {
	// FIXME(fpohtmeh): implement
	return MessageContent();
}


MessageContent Self::fromBytes(const QByteArray& messageJsonBytes, QString &errorString) {
	// FIXME(fpohtmeh): implement
	return MessageContent();
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


bool Self::readExtras(const QString& jsonString, MessageContentPicture &picture) {
    const auto json = QJsonDocument::fromJson(jsonString.toUtf8()).object();
    return readExtras(json, picture);
}


bool Self::writeExtras(const MessageContentPicture &picture, QJsonObject& json) {
    const auto thumbnail = picture.thumbnail();
    json["thumbnailPath"] = thumbnail.localPath();
    json["thumbnailUrl"] = thumbnail.remoteUrl().toString();
    json["thumbnailEncryptedSize"] = thumbnail.encryptedSize();
    const auto thumbnailSize = picture.thumbnailSize();
    json["thumbnailWidth"] = thumbnailSize.width();
    json["thumbnailHeight"] = thumbnailSize.height();
    json["previewPath"] = picture.previewPath();
	return true;
}
