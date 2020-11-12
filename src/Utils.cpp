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
#include <QUuid>

Q_LOGGING_CATEGORY(lcUtils, "utils")

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
    const QFileInfo info(fileName);
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
    qCDebug(lcUtils) << "Android file url (before encoding):" << url.toString();
    auto res = QUrl::fromPercentEncoding(url.toString().toUtf8());
    qCDebug(lcUtils) << "Android file url:" << res;
    return res;
#else
    qCDebug(lcUtils) << "File url:" << url.toLocalFile();
    return url.toLocalFile();
#endif
}

bool Utils::forceCreateDir(const QString &absolutePath)
{
    qDebug(lcUtils) << "Force to create dir:" << absolutePath;
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

QString Utils::formattedElapsedSeconds(const Seconds &seconds, const Seconds &nowInterval)
{
    if (seconds < nowInterval) {
        return QObject::tr("now");
    }
    if (seconds <= 3 * nowInterval) {
        return QObject::tr("few seconds ago");
    }
    const Seconds minute(60);
    if (seconds < minute) {
        return QObject::tr("recently");
    }
    if (seconds < 2 * minute) {
        return QObject::tr("a minute ago");
    }
    if (seconds < 50 * minute) {
        return QObject::tr("%1 minutes ago").arg(seconds / minute);
    }
    const Seconds hour(60 * minute);
    if (seconds < 2 * hour) {
        return QObject::tr("an hour ago");
    }
    const Seconds day(24 * hour);
    if (seconds < day) {
        return QObject::tr("%1 hours ago").arg(seconds / hour);
    }
    if (seconds < 2 * day) {
        return QObject::tr("yesterday");
    }
    const Seconds month(30 * day);
    if (seconds < month) {
        return QObject::tr("%1 days ago").arg(seconds / day);
    }
    const Seconds year(12 * month);
    if (seconds < year) {
        return QObject::tr("%1 months ago").arg(seconds / month);
    }
    if (seconds < 2 * year) {
        return QObject::tr("year ago");
    }
    return QObject::tr("%1 years ago").arg(seconds / year);
}

QString Utils::formattedLastSeenActivity(const Seconds &seconds, const Seconds &updateInterval)
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

QString Utils::attachmentDisplayText(const Attachment &attachment)
{
    if (attachment.type == Attachment::Type::Picture) {
        return QObject::tr("picture");
    }
    else {
        return Utils::elidedText(attachment.fileName, 50);
    }
}

Contact::Id Utils::contactIdFromJid(const Jid &jid)
{
    return jid.split('@').front();
}

Jid Utils::createJid(const Contact::Id &contactId, const QString &xmppUrl)
{
    return contactId + '@' + xmppUrl;
}

QString Utils::printableMessageBody(const Message &message)
{
    return message.body.left(30).replace('\n', ' ');
}

QString Utils::printableLoadProgress(const DataSize &loaded, const DataSize &total)
{
    return QString("%1% (%2/%3)").arg(qRound(100 * qMin<double>(loaded, total) / total)).arg(loaded).arg(total);
}

Optional<QString> Utils::readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::Text | QFile::ReadOnly)) {
        return NullOptional;
    }
    return file.readAll();
}

bool Utils::fileExists(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    return QFileInfo::exists(filePath);
}

void Utils::removeFile(const QString &filePath)
{
    if (fileExists(filePath)) {
        QFile::remove(filePath);
        qCDebug(lcUtils) << "Removed file:" << filePath;
    }
}

QString Utils::attachmentFileName(const QUrl &url, const QFileInfo &localInfo)
{
    QString fileName;
#ifdef VS_ANDROID
    fileName = VSQAndroid::getDisplayName(url);
#elif defined(VS_IOS_SIMULATOR)
    fileName = url.fileName();
#elif defined(VS_IOS)
    if (type == Attachment::Type::Picture) {
        // Build file name from url, i.e. "file:assets-library://asset/asset.PNG?id=7CE20DC4-89A8-4079-88DC-AD37920581B5&ext=PNG"
        QUrl urlWithoutFileScheme{url.toLocalFile()};
        const QUrlQuery query(urlWithoutFileScheme.query());
        fileName = query.queryItemValue("id") + QChar('.') + query.queryItemValue("ext").toLower();
    }
#else
    Q_UNUSED(url)
#endif
    if (fileName.isEmpty()) {
        fileName = localInfo.fileName();
    }
    return fileName;
}

QString Utils::attachmentDisplayImagePath(const Attachment &attachment)
{
    const auto e = attachment.extras.value<PictureExtras>();
    if (fileExists(e.previewPath)) {
        return e.previewPath;
    }
    if (fileExists(e.thumbnailPath)) {
        return e.thumbnailPath;
    }
    return QString();
}

bool Utils::openUrl(const QUrl &url)
{
    return QDesktopServices::openUrl(url);
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

QString Utils::extrasToJson(const QVariant &extras, const Attachment::Type type, bool skipLocal)
{
    if (type != Attachment::Type::Picture) {
        return QString();
    }
    const auto e = extras.value<PictureExtras>();
    QJsonObject obj;
    obj.insert("thumbnailWidth", e.thumbnailSize.width());
    obj.insert("thumbnailHeight", e.thumbnailSize.height());
    obj.insert("thumbnailUrl", e.thumbnailUrl.toString());
    obj.insert("encryptedThumbnailSize", e.encryptedThumbnailSize);
    if (!skipLocal) {
        obj.insert("thumbnailPath", e.thumbnailPath);
        obj.insert("previewPath", e.previewPath);
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QVariant Utils::extrasFromJson(const QString &json, const Attachment::Type type, bool skipLocal)
{
    if (type != Attachment::Type::Picture) {
        return QVariant();
    }
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    PictureExtras extras;
    extras.thumbnailSize.setWidth(doc["thumbnailWidth"].toInt());
    extras.thumbnailSize.setHeight(doc["thumbnailHeight"].toInt());
    extras.thumbnailUrl = doc["thumbnailUrl"].toString();
    extras.encryptedThumbnailSize = doc["encryptedThumbnailSize"].toInt();
    if (!skipLocal) {
        extras.thumbnailPath = doc["thumbnailPath"].toString();
        extras.previewPath = doc["previewPath"].toString();
    }
    return QVariant::fromValue(extras);
}

Message Utils::messageFromJson(const QByteArray &json)
{
    // Get message from JSON
    auto doc = QJsonDocument::fromJson(json);
    const auto type = doc["type"].toString();
    const auto payloadObject = doc["payload"];

    Message message;
    if (type == QLatin1String("text")) {
        message.body = payloadObject["body"].toString();
        return message;
    }

    const auto at = doc["attachment"];
    Attachment attachment;
    if (type == QLatin1String("picture")) {
        attachment.type = Attachment::Type::Picture;
    }
    else {
        attachment.type = Attachment::Type::File;
    }
    attachment.id = Utils::createUuid();
    attachment.fileName = at["fileName"].toString();
    attachment.size = at["size"].toInt();
    attachment.url = at["url"].toString();
    attachment.encryptedSize = at["encryptedSize"].toInt();
    attachment.fingerprint = at["fingerprint"].toString();
    attachment.extras = extrasFromJson(at["extras"].toString(), attachment.type, true);
    message.attachment = attachment;
    if (!message.body.isEmpty()) {
        qCDebug(lcUtils) << "Parsed JSON message:" << message.body;
    }
    return message;
}

QByteArray Utils::messageToJson(const Message &message)
{
    QJsonObject mainObject;
    QJsonObject payloadObject;
    const auto &attachment = message.attachment;
    if (!attachment) {
        mainObject.insert("type", "text");
        payloadObject.insert("body", message.body);
    }
    else {
        QJsonObject at;
        if (attachment->type == Attachment::Type::Picture) {
            mainObject.insert("type", "picture");
        }
        else {
            mainObject.insert("type", "file");
        }
        at.insert("fileName", attachment->fileName);
        at.insert("size", attachment->size);
        at.insert("url", attachment->url.toString());
        at.insert("encryptedSize", attachment->encryptedSize);
        at.insert("fingerprint", attachment->fingerprint);
        at.insert("extras", extrasToJson(attachment->extras, attachment->type, true));
        mainObject.insert("attachment", at);
    }
    mainObject.insert("payload", payloadObject);

    QJsonDocument doc(mainObject);
    return doc.toJson(QJsonDocument::Compact);
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
