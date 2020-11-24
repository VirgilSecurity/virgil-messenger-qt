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

#include "operations/ConvertToPngOperation.h"

#include <QImageReader>

#include "Settings.h"
#include "Utils.h"

using namespace vm;

ConvertToPngOperation::ConvertToPngOperation(const Settings *settings, const QString &sourcePath, QObject *parent)
    : Operation(QLatin1String("ConvertToPng"), parent)
    , m_settings(settings)
    , m_sourcePath(sourcePath)
{}

void ConvertToPngOperation::run()
{
    QImageReader reader(m_sourcePath);
    QImage source;
    if (!Utils::readImage(&reader, &source)) {
        invalidate(tr("Failed to read image file"));
        return;
    }
    const auto image = Utils::applyOrientation(source, reader.transformation());
    emit imageRead(image);

    const bool isPngFile = QFileInfo(m_sourcePath).completeSuffix().toLower() == QLatin1String(".png");
    if (isPngFile) {
        emit converted(m_sourcePath);
        finish();
    }
    else {
        const auto filePath = m_settings->attachmentCacheDir().filePath(Utils::createUuid() + QLatin1String(".png"));
        if (!image.save(filePath)) {
            qCWarning(lcOperation) << "Unable to save png file";
            invalidate(tr("Failed to convert to png"));
        }
        else if (!Utils::fileExists(filePath)) {
            qCWarning(lcOperation) << "Png file exceeds file limit";
            invalidate(tr("Png file exceeds file limit"));
        }
        else {
            qCDebug(lcOperation) << "File was converted to png";
            emit converted(filePath);
            emit fileCreated(filePath);
            finish();
        }
    }
}
