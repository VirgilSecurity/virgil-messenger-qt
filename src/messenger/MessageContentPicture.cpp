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

#include "MessageContentPicture.h"

#include <QImageReader>

#include "FileUtils.h"
#include "MessageContentJsonUtils.h"
#include "Utils.h"

using namespace vm;
using Self = MessageContentPicture;

bool Self::applyUpdate(const MessageUpdate &update)
{
    if (auto thumbnailPathUpdate = std::get_if<MessagePictureThumbnailPathUpdate>(&update)) {
        const auto thumbnailUrl = FileUtils::localFileToUrl(thumbnailPathUpdate->thumbnailPath);
        QString errorString;
        const auto thumbnail = MessageContentFile::createFromLocalFile(thumbnailUrl, errorString);
        if (thumbnail) {
            setThumbnail(*thumbnail);
        }
    } else if (auto thumbnailSizeUpdate = std::get_if<MessagePictureThumbnailSizeUpdate>(&update)) {
        setThumbnailSize(thumbnailSizeUpdate->thumbnailSize);
    } else if (auto thumbnailEncryptionUpdate = std::get_if<MessagePictureThumbnailEncryptionUpdate>(&update)) {
        m_thumbnail.setEncryptedSize(thumbnailEncryptionUpdate->encryptedSize);
        m_thumbnail.setDecryptionKey(thumbnailEncryptionUpdate->decryptionKey);
        m_thumbnail.setSignature(thumbnailEncryptionUpdate->signature);
    } else if (auto thumbnailRemoteUrlUpdate = std::get_if<MessagePictureThumbnailRemoteUrlUpdate>(&update)) {
        m_thumbnail.setRemoteUrl(thumbnailRemoteUrlUpdate->remoteUrl);
    } else if (auto previewPathUpdate = std::get_if<MessagePicturePreviewPathUpdate>(&update)) {
        setPreviewPath(previewPathUpdate->previewPath);
    } else {
        return MessageContentAttachment::applyUpdate(update);
    }
    return false;
}

QString MessageContentPicture::extrasToJson(const bool writeLocalPaths) const
{
    QJsonObject json;
    MessageContentJsonUtils::writeExtras(*this, writeLocalPaths, json);
    return MessageContentJsonUtils::toBytes(json);
}

QString Self::previewPath() const
{
    return m_previewPath;
}

void MessageContentPicture::setPreviewPath(const QString &path)
{
    m_previewPath = path;
}

QString Self::previewOrThumbnailPath() const
{
    return !m_previewPath.isEmpty() ? m_previewPath : m_thumbnail.localPath();
}

QSize Self::thumbnailSize() const
{
    return QSize(m_thumbnailWidth, m_thumbnailHeight);
}

void MessageContentPicture::setThumbnailSize(const QSize &thumbnailSize)
{
    m_thumbnailWidth = thumbnailSize.width();
    m_thumbnailHeight = thumbnailSize.height();
}

MessageContentFile Self::thumbnail() const
{
    return m_thumbnail;
}

void MessageContentPicture::setThumbnail(MessageContentFile thumbnail)
{
    m_thumbnail = std::move(thumbnail);
}

std::optional<MessageContentPicture> Self::createFromLocalFile(const QUrl &localUrl, const QSize &thumbnailMaxSize,
                                                               QString &errorString)
{
    MessageContentPicture picture;
    if (!picture.readLocalFile(localUrl, errorString)) {
        return std::nullopt;
    }

    const auto fileName = FileUtils::attachmentFileName(localUrl, true);
    picture.setFileName(fileName.section('.', 0, 0) + QLatin1String(".png"));
    picture.readImage(thumbnailMaxSize, errorString);
    return picture;
}

bool MessageContentPicture::readImage(const QSize &thumbnailMaxSize, QString &errorString)
{
    QImage source;
    QImageReader reader(localPath());
    if (!Utils::readImage(&reader, &source)) {
        errorString = QObject::tr("Unable to read image");
        return false;
    }

    const auto thumbnailSize = Utils::calculateThumbnailSize(source.size(), thumbnailMaxSize, reader.transformation());
    setThumbnailSize(thumbnailSize);
    return true;
}
