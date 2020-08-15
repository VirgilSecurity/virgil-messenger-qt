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

#include "VSQAttachmentBuilder.h"

#include <QPixmap>

#include <QXmppClient.h>

#include <virgil/iot/messenger/messenger.h>

#include "VSQCryptoTransferManager.h"
#include "VSQSettings.h"
#include "VSQUpload.h"
#include "VSQUtils.h"

using namespace VirgilIoTKit;

Q_LOGGING_CATEGORY(lcAttachment, "attachment");

VSQAttachmentBuilder::VSQAttachmentBuilder(VSQCryptoTransferManager *transferManager, QObject *parent)
    : QObject(parent)
    , m_transferManager(transferManager)
    , m_settings(transferManager->settings())
{}

bool VSQAttachmentBuilder::isValidUrl(const QUrl &url) const
{
    bool isValid = url.isValid();
#if !defined(Q_OS_ANDROID)
    isValid = isValid && url.isLocalFile();
#endif
    return isValid;
}

OptionalAttachment VSQAttachmentBuilder::build(const QUrl &url, const Attachment::Type type, const QString &messageId, const QString &recipient,
                                               QString &errorText)
{
    if (!isValidUrl(url)) {
        errorText = tr("Invalid attachment URL");
        return NullOptional;
    }
#if defined (Q_OS_ANDROID)
     QFileInfo localInfo(url.toString());
#else
    QFileInfo localInfo(url.toLocalFile());
#endif
    if (!localInfo.exists()) {
        errorText = tr("File doesn't exist");
        return NullOptional;
    }
    if (localInfo.size() > m_settings->attachmentMaxFileSize()) {
        errorText = tr("File size limit: %1").arg(VSQUtils::formattedDataSize(m_settings->attachmentMaxFileSize()));
        return NullOptional;
    }

    Attachment attachment;
    attachment.id = VSQUtils::createUuid();
    attachment.type = type;
    attachment.displayName = localInfo.fileName();
    attachment.filePath = localInfo.absoluteFilePath();
    const QString uploadErrorText = tr("Upload error");
    const bool isPicture = type == Attachment::Type::Picture;

    // Thumbnail processing
    if (isPicture) {
        attachment.thumbnailPath = createThumbnailFile(attachment.filePath);
        const auto id = messageId + QLatin1String("-thumb");
        auto upload = m_transferManager->startCryptoUpload(id, attachment.thumbnailPath, recipient);
        if (!upload) {
            errorText = uploadErrorText;
            return NullOptional;
        }
        attachment.remoteThumbnailUrl = *upload->remoteUrl();
    }

    // File processing
    auto upload = m_transferManager->startCryptoUpload(messageId, attachment.filePath, recipient);
    if (!upload) {
        errorText = uploadErrorText;
        return NullOptional;
    }
    attachment.remoteUrl = *upload->remoteUrl();
    attachment.bytesTotal = QFileInfo(upload->filePath()).size();

    return attachment;
}

QString VSQAttachmentBuilder::generateThumbnailFileName() const
{
    return m_settings->thumbnailsDir().filePath(VSQUtils::createUuid() + QLatin1String(".png"));
}

QString VSQAttachmentBuilder::createThumbnailFile(const QString &filePath) const
{
    QPixmap pixmap(filePath);
    QSizeF size = pixmap.size();
    const double ratio = size.height() / size.width();
    const QSizeF maxSize = m_settings->thumbnailMaxSize();
    if (size.width() > maxSize.width()) {
        size.setWidth(maxSize.width());
        size.setHeight(maxSize.width() * ratio);
    }
    if (size.height() > maxSize.height()) {
        size.setHeight(maxSize.height());
        size.setWidth(maxSize.height() / ratio);
    }
    if (size != pixmap.size()) {
        pixmap = pixmap.scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    const QString thumbnailFileName = generateThumbnailFileName();
    pixmap.save(thumbnailFileName);
    qCInfo(lcAttachment) << "Created thumbnail:" << thumbnailFileName;
    return thumbnailFileName;
}
