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

#include "models/AttachmentsModel.h"

#include <QImageReader>
#include <QUrlQuery>

#include "Settings.h"
#include "Utils.h"
#include "android/VSQAndroid.h"

using namespace vm;

AttachmentsModel::AttachmentsModel(Settings *settings, QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(settings)
{}

AttachmentsModel::~AttachmentsModel()
{}

Optional<Attachment> AttachmentsModel::createAttachment(const QUrl &url, const Attachment::Type type)
{
    // Check file
    if (!Utils::isValidUrl(url)) {
        return NullOptional;
    }
    QFileInfo localInfo(Utils::urlToLocalFile(url));
    if (!localInfo.exists()) {
        qCWarning(lcModel) << tr("File doesn't exist"); // TODO(fpohtmeh): forward error to UI
        return NullOptional;
    }

    // Check file size
    if (localInfo.size() == 0) {
        qCWarning(lcModel) << tr("File is empty");
        return NullOptional;
    }
#ifdef VS_ANDROID
    const DataSize fileSize = VSQAndroid::getFileSize(url);
#else
    const DataSize fileSize = localInfo.size();
#endif
    if (fileSize > m_settings->attachmentMaxFileSize()) {
        qCWarning(lcModel) << tr("File exceeds size limit");
        return NullOptional;
    }

    // Create attachment
    Attachment attachment;
    attachment.id = Utils::createUuid();
    attachment.messageId = Utils::createUuid();
    attachment.type = type;
    attachment.size = fileSize;

    // Filename
#ifdef VS_ANDROID
    attachment.fileName = VSQAndroid::getDisplayName(url);
#elif defined(VS_IOS)
    if (type == Attachment::Type::Picture) {
        // Build file name from url, i.e. "file:assets-library://asset/asset.PNG?id=7CE20DC4-89A8-4079-88DC-AD37920581B5&ext=PNG"
        QUrl urlWithoutFileScheme{url.toLocalFile()};
        const QUrlQuery query(urlWithoutFileScheme.query());
        attachment.fileName = query.queryItemValue("id") + QChar('.') + query.queryItemValue("ext").toLower();
    }
#endif
    if (attachment.fileName.isEmpty()) {
        attachment.fileName = localInfo.fileName();
    }
    attachment.localPath = localInfo.absoluteFilePath();

    // Picture
    if (type == Attachment::Type::Picture) {
        const auto extras = createPictureExtras(attachment.localPath);
        if (!extras) {
            return NullOptional;
        }
        attachment.extras.setValue(*extras);
    }

    return attachment;
}

int AttachmentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

QVariant AttachmentsModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant();
}

Optional<PictureExtras> AttachmentsModel::createPictureExtras(const QString &localPath) const
{
    PictureExtras extras;
    QImage source;
    QImageReader reader(localPath);
    if (!Utils::readImage(&reader, &source)) {
        return NullOptional;
    }
    qCDebug(lcModel) << "Read image dimensions:" << source.size();
    extras.thumbnailSize = Utils::calculateThumbnailSize(source.size(), m_settings->thumbnailMaxSize(), reader.transformation());
    return extras;
}
