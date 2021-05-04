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

#include "FileUtils.h"

#include "UidUtils.h"
#include "PlatformFs.h"

#include <QCryptographicHash>
#include <QDir>
#include <QLoggingCategory>
#include <QMimeDatabase>


Q_LOGGING_CATEGORY(lcFileUtils, "file-utils");

using namespace vm;
using namespace platform;
using Self = vm::FileUtils;

QString Self::calculateFingerprint(const QString &path)
{
    if (!Self::fileExists(path)) {
        qCWarning(lcFileUtils) << "Failed to find fingerprint. File doesn't exist:" << path;
        return QString();
    }
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcFileUtils) << "Failed to find fingerprint. File can't be opened:" << path;
        return QString();
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    const QString fingerpint = hash.result().toHex().left(8);
    qCDebug(lcFileUtils) << "File fingerprint:" << path << "=>" << fingerpint;
    return fingerpint;
}

QString Self::findUniqueFileName(const QString &fileName)
{
    const QFileInfo info(fileName);
    if (!info.exists()) {
        return fileName;
    }
    auto dir = info.absoluteDir();
    auto baseName = info.baseName();
    auto suffix = info.completeSuffix();
    for (;;) {
        auto uuid = UidUtils::createUuid().remove('-').left(6);
        auto newFileName = dir.filePath(QString("%1-%2.%3").arg(baseName, uuid, suffix));
        if (!QFile::exists(newFileName)) {
            return newFileName;
        }
    }
}

bool Self::isValidUrl(const QUrl &url)
{
    // TODO(fpohtmeh): review previous implementation
    return url.isValid();
}

QString Self::urlToLocalFile(const QUrl &url)
{
    return PlatformFs::instance().urlToLocalFile(url);
}

QUrl Self::localFileToUrl(const QString &filePath)
{
    return PlatformFs::instance().localFileToUrl(filePath);
}

bool Self::forceCreateDir(const QString &absolutePath, bool isFatal)
{
    const QFileInfo info(absolutePath);
    if (info.exists()) {
        if (info.isDir()) {
            return true;
        } else {
            QFile::remove(absolutePath);
        }
    }
    if (QDir().mkpath(absolutePath)) {
        return true;
    }
    if (isFatal) {
        qFatal("Failed to create directory: %s", qPrintable(absolutePath));
    } else {
        qWarning(lcFileUtils()) << "Failed to create directory:" << absolutePath;
    }
    return false;
}

std::optional<QString> Self::readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::Text | QFile::ReadOnly)) {
        qCWarning(lcFileUtils) << "Failed to read text file";
        return std::nullopt;
    }
    return file.readAll();
}

bool Self::fileExists(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    return QFileInfo::exists(filePath);
}

bool Self::fileExists(const QUrl &fileUrl)
{
    return fileExists(urlToLocalFile(fileUrl));
}

quint64 Self::fileSize(const QUrl &fileUrl)
{
    return fileSize(urlToLocalFile(fileUrl));
}

quint64 Self::fileSize(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        return 0;
    }

    return fileInfo.size();
}

void Self::removeFile(const QString &filePath)
{
    if (fileExists(filePath)) {
        if (QFile::remove(filePath)) {
            qCDebug(lcFileUtils) << "File was removed:" << filePath;
        }
    }
}

void FileUtils::removeDir(const QString &dirPath)
{
    if (fileExists(dirPath)) {
        if (QDir(dirPath).removeRecursively()) {
            qCDebug(lcFileUtils) << "Dir was removed recursively:" << dirPath;
        }
    }
}

QString FileUtils::fileName(const QString &filePath)
{
    return QFileInfo(filePath).fileName();
}

QString FileUtils::fileExt(const QString &filePath)
{
    return QFileInfo(filePath).completeSuffix();
}

QString Self::attachmentFileName(const QUrl &url, bool isPicture)
{
    return PlatformFs::instance().fileDisplayName(url, isPicture);
}

QString FileUtils::fileMimeType(const QString &filePath)
{
    static QMimeDatabase db;
    return db.mimeTypeForFile(filePath).name();
}
