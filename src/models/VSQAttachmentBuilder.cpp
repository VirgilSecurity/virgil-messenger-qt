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

#include "models/VSQAttachmentBuilder.h"

#include <QPixmap>

#include "VSQSettings.h"
#include "VSQUtils.h"

VSQAttachmentBuilder::VSQAttachmentBuilder(VSQSettings *settings)
    : m_settings(settings)
{}


OptionalAttachment VSQAttachmentBuilder::build(const QUrl &url, const Attachment::Type type)
{
    if (!url.isValid() || !url.isLocalFile())
        return NullOptional;
    QFileInfo info(url.toLocalFile());
    if (!info.exists()) {
        qInfo() << QString("Attachment file %1 doesn't exist").arg(info.absoluteFilePath());
        return NullOptional;
    }
    if (info.size() > m_settings->attachmentMaxSize()) {
        qInfo() << QString("File size exceeds maximum limit: %1").arg(m_settings->attachmentMaxSize());
        return NullOptional;
    }
    Attachment attachment;
    attachment.id = VSQUtils::createUuid();
    attachment.type = type;
    attachment.local_url = QUrl::fromLocalFile(info.absoluteFilePath());
    if (type == Attachment::Type::Picture)
        attachment.local_preview = createPreviewImage(info.absoluteFilePath());
    attachment.size = info.size();
    return attachment;
}

QUrl VSQAttachmentBuilder::createPreviewImage(const QString &fileName) const
{
    QPixmap pixmap(fileName);
    QSizeF size = pixmap.size();
    const double ratio = size.height() / size.width();
    const QSizeF maxSize = m_settings->previewMaxSize();
    if (size.width() > maxSize.width())
    {
        size.setWidth(maxSize.width());
        size.setHeight(maxSize.width() * ratio);
    }
    if (size.height() > maxSize.height())
    {
        size.setHeight(maxSize.height());
        size.setWidth(maxSize.height() / ratio);
    }
    if (size != pixmap.size())
        pixmap = pixmap.scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const QString previewFileName = m_settings->attachmentCacheDir().filePath(VSQUtils::createUuid() + QLatin1String(".png"));
    pixmap.save(previewFileName);
    qInfo() << "Created preview image:" << previewFileName;
    return QUrl::fromLocalFile(previewFileName);
}
