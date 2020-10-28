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

#include "Utils.h"

#include <QDir>
#include <QLocale>
#include <QThread>
#include <QUuid>

using namespace vm;

QString Utils::createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toLower();
}

QString Utils::formattedDataSize(DataSize fileSize)
{
    static QLocale locale = QLocale::system();
    return locale.formattedDataSize(fileSize);
}

QString Utils::findUniqueFileName(const QString &fileName)
{
    QFileInfo info(fileName);
    if (!info.exists()) {
        return fileName;
    }
    auto dir = info.absoluteDir();
    auto baseName = info.baseName();
    auto suffix = info.completeSuffix();
    for(;;) {
        auto uuid = createUuid().remove('-').left(6);
        auto newFileName = dir.filePath(QString("%1-%2.%3").arg(baseName, uuid, suffix));
        if (!QFile::exists(newFileName)) {
            return newFileName;
        }
    }
}

bool Utils::isValidUrl(const QUrl &url)
{
    bool isValid = url.isValid();
#if !defined(Q_OS_ANDROID)
    isValid = isValid && url.isLocalFile();
#endif
    return isValid;
}

QString Utils::urlToLocalFile(const QUrl &url)
{
#if defined (Q_OS_ANDROID)
    qDebug() << "Android file url (before encoding):" << url.toString();
    auto res = QUrl::fromPercentEncoding(url.toString().toUtf8());
    qDebug() << "Android file url:" << res;
    return res;
#else
    qDebug() << "File url:" << url.toLocalFile();
    return url.toLocalFile();
#endif
}

bool Utils::forceCreateDir(const QString &absolutePath)
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
    qFatal("Unable to create directory: %s", qPrintable(absolutePath));
    return false;
}

QUrl Utils::localFileToUrl(const QString &filePath)
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

QString Utils::formattedLastSeenActivity(const Seconds &seconds, const Seconds &updateInterval)
{
    const auto preffix = QObject::tr("Last seen %1");
    if (seconds < updateInterval) {
        return QObject::tr("Online");
    }
    if (seconds <= 3 * updateInterval) {
        return preffix.arg(QObject::tr("few seconds ago"));
    }
    const Seconds minute(60);
    if (seconds < minute) {
        return preffix.arg(QObject::tr("recently"));
    }
    if (seconds < 2 * minute) {
        return preffix.arg(QObject::tr("a minute ago"));
    }
    if (seconds < 50 * minute) {
        return preffix.arg(QObject::tr("%1 minutes ago").arg(seconds / minute));
    }
    const Seconds hour(60 * minute);
    if (seconds < 2 * hour) {
        return preffix.arg(QObject::tr("an hour ago"));
    }
    const Seconds day(24 * hour);
    if (seconds < day) {
        return preffix.arg(QObject::tr("%1 hours ago").arg(seconds / hour));
    }
    if (seconds < 2 * day) {
        return preffix.arg(QObject::tr("yesterday"));
    }
    const Seconds month(30 * day);
    if (seconds < month) {
        return preffix.arg(QObject::tr("%1 days ago").arg(seconds / day));
    }
    const Seconds year(12 * month);
    if (seconds < year) {
        return preffix.arg(QObject::tr("%1 months ago").arg(seconds / month));
    }
    if (seconds < 2 * year) {
        return preffix.arg(QObject::tr("year ago"));
    }
    return preffix.arg(QObject::tr("%1 years ago").arg(seconds / year));
}

QString Utils::formattedLastSeenNoActivity()
{
    return QObject::tr("Offline");
}

QString Utils::elidedText(const QString &text, const int maxLength)
{
    const int max = qMax(5, maxLength);
    if (text.size() <= max) {
        return text;
    }
    const int half = (max - 1) / 2;
    const QChar ellipsisChar(0x2026);
    return text.left(half) + ellipsisChar + text.right(max - 1 - half);
}

Optional<QString> Utils::readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::Text | QFile::ReadOnly)) {
        return NullOptional;
    }
    return file.readAll();
}

void Utils::printThreadId(const QString &message)
{
#ifdef VS_DEVMODE
    qDebug(lcDev).noquote().nospace()
            << "Thread " << QThread::currentThread()->objectName() << "(" << QThread::currentThreadId() << "): "
            << message;
#endif
}
