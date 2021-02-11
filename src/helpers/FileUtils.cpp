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

#include "Utils.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDesktopServices>
#include <QLoggingCategory>

#include <android/VSQAndroid.h>

Q_LOGGING_CATEGORY(lcFileUtils, "file-utils");

using namespace vm;
using Self = vm::FileUtils;

QString
Self::calculateFingerprint(const QString &path) {
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
    for(;;) {
        auto uuid = Utils::createUuid().remove('-').left(6);
        auto newFileName = dir.filePath(QString("%1-%2.%3").arg(baseName, uuid, suffix));
        if (!QFile::exists(newFileName)) {
            return newFileName;
        }
    }
}

bool Self::isValidUrl(const QUrl &url)
{
    bool isValid = url.isValid();
#if !defined(Q_OS_ANDROID)
    isValid = isValid && url.isLocalFile();
#endif
    return isValid;
}

QString Self::urlToLocalFile(const QUrl &url)
{
#if defined (Q_OS_ANDROID)
    qCDebug(lcFileUtils) << "Android file url (before encoding):" << url.toString();
    auto res = QUrl::fromPercentEncoding(url.toString().toUtf8());
    qCDebug(lcFileUtils) << "Android file url:" << res;
    return res;
#else
    qCDebug(lcFileUtils) << "File url:" << url.toLocalFile();
    return url.toLocalFile();
#endif
}

bool Self::forceCreateDir(const QString &absolutePath)
{
    const QFileInfo info(absolutePath);
    if (info.exists()) {
        if (info.isDir()) {
            return true;
        }
        else {
            QFile::remove(absolutePath);
        }
    }
    if (QDir().mkpath(absolutePath)) {
        return true;
    }
    qFatal("Failed to create directory: %s", qPrintable(absolutePath));
    return false;
}

QUrl Self::localFileToUrl(const QString &filePath)
{
#if defined (Q_OS_ANDROID)
    QUrl url(filePath);
    if (url.scheme().isEmpty()) {
        return QUrl::fromLocalFile(filePath);
    }
    return url;
#else
    return QUrl::fromLocalFile(filePath);
#endif
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

QString Self::attachmentFileName(const QUrl &url, bool isPicture)
{
    QString fileName;
#ifdef VS_ANDROID
    Q_UNUSED(isPicture)
    fileName = VSQAndroid::getDisplayName(url);
#elif defined(VS_IOS_SIMULATOR)
    fileName = url.fileName();
#elif defined(VS_IOS)
    if (isPicture) {
        // Build file name from url, i.e. "file:assets-library://asset/asset.PNG?id=7CE20DC4-89A8-4079-88DC-AD37920581B5&ext=PNG"
        QUrl urlWithoutFileScheme{url.toLocalFile()};
        const QUrlQuery query(urlWithoutFileScheme.query());
        fileName = query.queryItemValue("id") + QChar('.') + query.queryItemValue("ext").toLower();
    }
#else
    Q_UNUSED(url)
    Q_UNUSED(isPicture)
#endif
    if (fileName.isEmpty()) {
        fileName = FileUtils::fileName(urlToLocalFile(url));
    }
    return fileName;
}

QString FileUtils::fileMimeType(const QString &filePath)
{
    static QMimeDatabase db;
    return db.mimeTypeForFile(filePath).name();
}

bool Self::openUrl(const QUrl &url)
{
    return QDesktopServices::openUrl(url);
}
