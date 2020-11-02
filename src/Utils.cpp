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
#include <QJsonDocument>
#include <QJsonObject>
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

QString Utils::attachmentDisplayText(const Attachment &attachment)
{
    if (attachment.type == Attachment::Type::Picture) {
        return QObject::tr("picture");
    }
    else {
        return Utils::elidedText(attachment.fileName, 50); // TODO(fpohtmeh): move to settings?
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

QString Utils::extrasToJson(const QVariant &extras, const Attachment::Type type)
{
    if (type != Attachment::Type::Picture) {
        return QString();
    }
    // FIXME(fpohtmeh): add type parameter (DB or XMPP)
    const auto e = extras.value<PictureExtras>();
    QJsonObject obj;
    // FIXME(fpohtmeh): refine properties
//    obj.insert("width", e.size.width());
//    obj.insert("height", e.size.height());
//    obj.insert("orientation", e.orientation);
//    obj.insert("thumbnailWidth", e.thumbnailSize.width());
//    obj.insert("thumbnailHeight", e.thumbnailSize.height());
//    obj.insert("thumbnailPath", e.thumbnailPath);
//    obj.insert("previewPath", e.previewPath);
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QVariant Utils::extrasFromJson(const QString &json, const Attachment::Type type)
{
    if (type != Attachment::Type::Picture) {
        return QVariant();
    }
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    PictureExtras extras;
    // FIXME(fpohtmeh): refine properties
//    extras.size.setWidth(doc["width"].toInt());
//    extras.size.setHeight(doc["height"].toInt());
//    extras.orientation = doc["orientation"].toInt();
//    extras.thumbnailSize.setWidth(doc["thumbnailWidth"].toInt());
//    extras.thumbnailSize.setHeight(doc["thumbnailHeight"].toInt());
//    extras.thumbnailPath = doc["thumbnailPath"].toString();
//    extras.previewPath = doc["previewPath"].toString();
    return QVariant::fromValue(extras);
}

Message Utils::messageFromJson(const QByteArray &json)
{
    // Get message from JSON
    auto doc = QJsonDocument::fromJson(json);
    qDebug() << "JSON for parsing:" << doc;
    const auto type = doc["type"].toString();
    const auto payload = doc["payload"];

    Message message;
    if (type == QLatin1String("text")) {
        message.body = payload["body"].toString();
        return message;
    }
    // FIXME(fpohtmeh): implement
    /*
    AttachmentV0 attachment;
    attachment.id = Utils::createUuid();
    attachment.remoteUrl = payload["url"].toString();
    attachment.fileName = payload["fileName"].toString();
    attachment.displayName = payload["displayName"].toString();
    attachment.bytesTotal = payload["bytesTotal"].toInt();
    message.message = attachment.displayName;
    if (type == QLatin1String("picture")) {
    attachment.type = AttachmentV0::Type::Picture;
    attachment.remoteThumbnailUrl = payload["thumbnailUrl"].toString();
    attachment.thumbnailSize = QSize(payload["thumbnailWidth"].toInt(), payload["thumbnailHeight"].toInt());
    }
    else {
    attachment.type = AttachmentV0::Type::File;
    }
    message.attachment = attachment;
    return message;
    */
    qDebug() << "Received message: " << message.body;
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
        if (attachment->type == AttachmentV0::Type::Picture) {
            mainObject.insert("type", "picture");
// FIXME(fpohtmeh): implement
//            payloadObject.insert("thumbnailUrl", attachment->remoteThumbnailUrl.toString());
//            payloadObject.insert("thumbnailWidth", attachment->thumbnailSize.width());
//            payloadObject.insert("thumbnailHeight", attachment->thumbnailSize.height());
        }
        else {
            mainObject.insert("type", "file");
        }
        // FIXME(fpohtmeh): implement
//        payloadObject.insert("url", attachment->remoteUrl.toString());
//        payloadObject.insert("fileName", attachment->fileName);
//        payloadObject.insert("displayName", attachment->displayName);
//        payloadObject.insert("bytesTotal", attachment->bytesTotal);
    }
    mainObject.insert("payload", payloadObject);

    QJsonDocument doc(mainObject);
    return doc.toJson(QJsonDocument::Compact);
}
