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

#include <QDesktopServices>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QThread>
#include <QUrlQuery>
#include <QUuid>
#include <QLoggingCategory>

#include "android/VSQAndroid.h"

Q_LOGGING_CATEGORY(lcUtils, "utils")

using namespace vm;

QString Utils::createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toLower();
}

QString Utils::formattedSize(quint64 fileSize)
{
    static QLocale locale = QLocale::system();
    return locale.formattedDataSize(fileSize);
}

QString Utils::formattedElapsedSeconds(std::chrono::seconds seconds, std::chrono::seconds nowInterval)
{
    if (seconds < nowInterval) {
        return QObject::tr("now");
    }
    if (seconds <= 3 * nowInterval) {
        return QObject::tr("few seconds ago");
    }
    const std::chrono::seconds minute(60);
    if (seconds < minute) {
        return QObject::tr("recently");
    }
    if (seconds < 2 * minute) {
        return QObject::tr("a minute ago");
    }
    if (seconds < 50 * minute) {
        return QObject::tr("%1 minutes ago").arg(seconds / minute);
    }
    const std::chrono::seconds hour(60 * minute);
    if (seconds < 2 * hour) {
        return QObject::tr("an hour ago");
    }
    const std::chrono::seconds day(24 * hour);
    if (seconds < day) {
        return QObject::tr("%1 hours ago").arg(seconds / hour);
    }
    if (seconds < 2 * day) {
        return QObject::tr("yesterday");
    }
    const std::chrono::seconds month(30 * day);
    if (seconds < month) {
        return QObject::tr("%1 days ago").arg(seconds / day);
    }
    const std::chrono::seconds year(12 * month);
    if (seconds < year) {
        return QObject::tr("%1 months ago").arg(seconds / month);
    }
    if (seconds < 2 * year) {
        return QObject::tr("year ago");
    }
    return QObject::tr("%1 years ago").arg(seconds / year);
}

QString Utils::formattedLastSeenActivity(std::chrono::seconds seconds, std::chrono::seconds updateInterval)
{
    if (seconds < updateInterval) {
        return QObject::tr("Online");
    }
    return QObject::tr("Last seen %1").arg(formattedElapsedSeconds(seconds, updateInterval));
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

QString Utils::messageContentDisplayText(const MessageContent &messageContent)
{
    if (auto text = std::get_if<MessageContentText>(&messageContent)) {
        return text->text().left(250).replace('\n', ' ');

    } else if (std::holds_alternative<MessageContentPicture>(messageContent)) {
        return QObject::tr("picture");

    } else if (auto file = std::get_if<MessageContentFile>(&messageContent)) {
        return Utils::elidedText(file->fileName(), 50);

    } else {
        return {};
    }
}

QString Utils::printableLoadProgress(quint64 loaded, quint64 total)
{
    return QString("%1% (%2/%3)").arg(qRound(100 * qMin<double>(loaded, total) / total)).arg(loaded).arg(total);
}

void Utils::printThreadId(const QString &message)
{
#ifdef VS_DEVMODE
    qDebug(lcDev).noquote().nospace()
            << "Thread " << QThread::currentThread()->objectName() << "(" << QThread::currentThreadId() << "): "
            << message;
#else
    Q_UNUSED(message)
#endif
}

QSize Utils::applyOrientation(const QSize &size, int orientation)
{
    if (orientation & QImageIOHandler::TransformationRotate90) {
        return QSize(size.height(), size.width());
    }
    return size;
}

QImage Utils::applyOrientation(const QImage &image, const int orientation)
{
    auto result = image;
    const bool horizontally = orientation & QImageIOHandler::TransformationMirror;
    const bool vertically = orientation & QImageIOHandler::TransformationFlip;
    if (horizontally || vertically) {
        result = result.mirrored(horizontally, vertically);
    }
    if (orientation & QImageIOHandler::TransformationRotate90) {
        result = result.transformed(QTransform().rotate(90.0));
    }
    return result;
}

QSize Utils::calculateThumbnailSize(const QSize &size, const QSize &maxSize, const int orientation)
{
    QSizeF s = applyOrientation(size, orientation);
    const double ratio = s.height() / s.width();
    if (s.width() > maxSize.width()) {
        s.setWidth(maxSize.width());
        s.setHeight(maxSize.width() * ratio);
    }
    if (s.height() > maxSize.height()) {
        s.setHeight(maxSize.height());
        s.setWidth(maxSize.height() / ratio);
    }
    return s.toSize();
}

bool Utils::readImage(QImageReader *reader, QImage *image)
{
    if (!reader->read(image)) {
        qCDebug(lcUtils) << "Image reader error" << reader->errorString() << reader->fileName();
        return false;
    }
    if (image->size().isEmpty()) {
        qCDebug(lcUtils) << "Read image is invalid" << reader->fileName();
        return false;
    }
    return true;
}
