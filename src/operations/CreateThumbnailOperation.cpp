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

#include "operations/CreateThumbnailOperation.h"

#include <QImageReader>

#include "Utils.h"

using namespace vm;

CreateThumbnailOperation::CreateThumbnailOperation(QObject *parent,
                                                   const QString &sourcePath, const QString &destPath, const QSize &maxSize)
    : Operation(QLatin1String("CreateThumbnailOperation"), parent)
    , m_sourcePath(sourcePath)
    , m_destPath(destPath)
    , m_maxSize(maxSize)
{}

void CreateThumbnailOperation::run()
{
    if (m_sourceImage.isNull()) {
        QImageReader reader(m_sourcePath);
        QImage source;
        if (!Utils::readImage(&reader, &source)) {
            invalidate(tr("Failed to read image for thumbnail"));
            return;
        }
        m_sourceImage = Utils::applyOrientation(source, reader.transformation());
    }
    const auto size = Utils::calculateThumbnailSize(m_sourceImage.size(), m_maxSize);
    if (size == m_sourceImage.size()) {
        QFile::copy(m_sourcePath, m_destPath);
    }
    else {
        const auto dest = m_sourceImage.scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!QImage(dest).save(m_destPath)) {
            qCDebug(lcOperation) << "Failed to save thumbnail file:" << m_destPath;
            invalidate(tr("Failed to save thumbnail file"));
            return;
        }
    }
    emit thumbnailReady(m_destPath, size);
    finish();
}

void CreateThumbnailOperation::setSourcePath(const QString &path)
{
    m_sourcePath = path;
}

void CreateThumbnailOperation::setSourceImage(const QImage &image)
{
    m_sourceImage = image;
}
