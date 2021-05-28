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

#include "MessageContentPictureFactory.h"

#include "FileUtils.h"
#include "MessageContentJsonUtils.h"
#include "Utils.h"

#include <QImageReader>

using namespace vm;
using Self = MessageContentPictureFactory;

std::optional<MessageContentPicture> Self::createFromLocalFile(const QUrl &localUrl, const QString &imageFormat,
                                                               const QSize &thumbnailMaxSize, QString &errorString)
{
    MessageContentPicture picture;
    if (!picture.readLocalFile(localUrl, errorString)) {
        return std::nullopt;
    }

    const auto fileName = FileUtils::attachmentFileName(localUrl, true);
    picture.setFileName(fileName.section('.', 0, 0) + imageFormat);

    QImage source;
    QImageReader reader(picture.localPath());
    if (!Utils::readImage(&reader, &source)) {
        errorString = QObject::tr("Unable to read image");
        return std::nullopt;
    }

    const auto thumbnailSize = Utils::calculateThumbnailSize(source.size(), thumbnailMaxSize, reader.transformation());
    picture.setThumbnailSize(thumbnailSize);

    return picture;
}
