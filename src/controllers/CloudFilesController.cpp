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

#include "controllers/CloudFilesController.h"

#include "Settings.h"
#include "Utils.h"
#include "FileUtils.h"
#include "models/CloudFilesModel.h"
#include "models/Models.h"
#include "Controller.h"

using namespace vm;

CloudFilesController::CloudFilesController(const Settings *settings, Models *models, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_models(models)
    , m_rootDir()
    , m_currentDir(m_rootDir)
{}

void CloudFilesController::openFile(const QVariant &proxyRow)
{
    const auto fileInfo = model()->getFileInfo(proxyRow.toInt());
    FileUtils::openUrl(FileUtils::localFileToUrl(fileInfo.absoluteFilePath()));
}

void CloudFilesController::setDirectory(const QVariant &proxyRow)
{
    const auto fileInfo = model()->getFileInfo(proxyRow.toInt());
    setDirectory(QDir(fileInfo.absoluteFilePath()));
}

void CloudFilesController::cdUp()
{
    m_currentDir.cdUp();
    setDirectory(m_currentDir);
}

void CloudFilesController::addFile(const QVariant &attachmentUrl)
{
    const auto url = attachmentUrl.toUrl();
    const auto filePath = FileUtils::urlToLocalFile(url);
    const auto fileName = FileUtils::attachmentFileName(url, false);
    // Copy
    const auto destFilePath = FileUtils::findUniqueFileName(m_currentDir.filePath(fileName));
    if (QFile::copy(filePath, destFilePath)) {
        setDirectory(m_currentDir);
    }
    else {
        qCWarning(lcController) << "Failed to copy attachment";
    }
}

void CloudFilesController::deleteFiles()
{
    qCDebug(lcController) << "Feature is under development";
}

void CloudFilesController::createDirectory(const QString &name)
{
    Q_UNUSED(name)
    qCDebug(lcController) << "Feature is under development";
}

CloudFilesModel *CloudFilesController::model()
{
    return m_models->cloudFiles();
}

void CloudFilesController::setRootDirectory(const QDir &dir)
{
    m_rootDir = dir;
    setDirectory(dir);
}

void CloudFilesController::setDirectory(const QDir &dir)
{
    m_currentDir = dir;
    model()->setDirectory(dir);
    // Update display path
    m_displayPath.clear();
    for (auto d = dir; d != m_rootDir; d.cdUp()) {
        m_displayPath.append(QLatin1String("  /  ") + d.dirName());
    }
    emit displayPathChanged(m_displayPath);
}
