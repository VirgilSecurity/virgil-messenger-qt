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

#include <virgil/iot/messenger/messenger.h>

#include "VSQSettings.h"
#include "VSQUtils.h"

using namespace VirgilIoTKit;

Q_LOGGING_CATEGORY(lcAttachment, "attachment");

VSQAttachmentBuilder::VSQAttachmentBuilder(VSQSettings *settings)
    : m_settings(settings)
{}

bool VSQAttachmentBuilder::isValidUrl(const QUrl &url) const
{
    return url.isValid() && url.isLocalFile();
}

OptionalAttachment VSQAttachmentBuilder::build(const QUrl &url, const Attachment::Type type, const QString &recipient)
{
    if (!isValidUrl(url)) {
        return NullOptional;
    }

    QFileInfo localInfo(url.toLocalFile());
    if (!localInfo.exists()) {
        qCInfo(lcAttachment) << QString("Attachment file %1 doesn't exist").arg(localInfo.absoluteFilePath());
        return NullOptional;
    }
    if (localInfo.size() > m_settings->attachmentMaxSize()) {
        qCInfo(lcAttachment) << QString("File size exceeds maximum limit: %1").arg(m_settings->attachmentMaxSize());
        return NullOptional;
    }
    const QString localFilePath = localInfo.absoluteFilePath();

    const QString encryptedFilePath = createEncryptedFile(localFilePath, recipient);
    if (encryptedFilePath.isEmpty()) {
        return NullOptional;
    }
    QFileInfo encryptedInfo(encryptedFilePath);

    Attachment attachment;
    attachment.id = VSQUtils::createUuid();
    attachment.type = type;
    attachment.localUrl = QUrl::fromLocalFile(localFilePath);
    attachment.encLocalUrl = QUrl::fromLocalFile(encryptedFilePath);
    if (type == Attachment::Type::Picture) {
        attachment.thumbnailUrl = QUrl::fromLocalFile(createThumbnailFile(localFilePath));
    }
    attachment.bytesTotal = encryptedInfo.size();
    return attachment;
}

QString VSQAttachmentBuilder::createEncryptedFile(const QString &filePath, const QString &recipient) const
{
    // FIXME(fpohtmeh): implement
    /*
    QByteArray bytes;
    {
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly)) {
            qCWarning(lcAttachment) << "Cannot open attachment for reading:" << filePath;
            return QString();
        }
        bytes = file.readAll();
    }
    qCDebug(lcAttachment) << "Input size:" << bytes.size();

    // Encrypt
    const size_t encryptedMaxSize = 10 * bytes.size();
    std::vector<uint8_t> encryptedBytes(encryptedMaxSize);
    size_t encryptedSize = 0;
    if (VS_CODE_OK != vs_messenger_virgil_encrypt_msg(
                recipient.toStdString().c_str(),
                bytes.data(),
                encryptedBytes.data(),
                encryptedMaxSize,
                &encryptedSize)) {
        qCWarning(lcAttachment) << "Cannot encrypt attachment:" << filePath;
        return QString();
    }
    encryptedBytes.resize(encryptedSize);
    qCDebug(lcAttachment) << "Encrypted size:" << encryptedSize;

    // Decrypt
    const size_t decryptedMaxSize = 10 * encryptedSize;
    std::vector<uint8_t> decryptedBytes(decryptedMaxSize);
    size_t decryptedSize = 0;
    if (VS_CODE_OK != vs_messenger_virgil_decrypt_msg(
                recipient.toStdString().c_str(),
                reinterpret_cast<const char *>(encryptedBytes.data()),
                decryptedBytes.data(),
                decryptedSize - 1,
                &decryptedSize)) {
        qCWarning(lcAttachment) << "Cannot decrypt attachment:" << filePath;
        return QString();
    }
    decryptedBytes.resize(decryptedSize);
    qCDebug(lcAttachment) << "Decrypted size:" << decryptedSize;
    */
    return filePath;
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
    const QString thumbnailFileName = m_settings->attachmentCacheDir().filePath(VSQUtils::createUuid() + QLatin1String(".png"));
    pixmap.save(thumbnailFileName);
    qCInfo(lcAttachment) << "Created thumbnail:" << thumbnailFileName;
    return thumbnailFileName;
}
