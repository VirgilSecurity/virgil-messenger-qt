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

#include <cmath>

#include <QDesktopServices>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QThread>
#include <QUrlQuery>
#include <QUuid>
#include <QLoggingCategory>

#if VS_ANDROID
#    include "VSQAndroid.h"
#endif

Q_LOGGING_CATEGORY(lcUtils, "utils")

using namespace vm;

namespace {
#define VS_DUMMY_CONTACTS 0

#if VS_DUMMY_CONTACTS
Contacts getDummyContacts()
{
    auto c0 = std::make_shared<Contact>();
    c0->setName("John Doe");
    c0->setAvatarLocalPath("https://avatars.mds.yandex.net/get-zen_doc/1779726/"
                           "pub_5d32ac8bf2df2500adb00103_5d32aeae21f9ff00ad9973ee/scale_1200");
    c0->setEmail("johndoe@gmail.com");

    auto c1 = std::make_shared<Contact>();
    c1->setName("Bon Min");
    c1->setAvatarLocalPath(
            "https://peopletalk.ru/wp-content/uploads/2016/10/orig_95f063cefa53daf194fa9f6d5e20b86c.jpg");

    auto c2 = std::make_shared<Contact>();
    c2->setName("Tin Bin");
    c2->setAvatarLocalPath("https://i.postimg.cc/wBJKr6CR/K5-W-z1n-Lqms.jpg");

    auto c3 = std::make_shared<Contact>();
    c3->setName("Mister Bean");
    c3->setAvatarLocalPath("https://avatars.mds.yandex.net/get-zen_doc/175962/"
                           "pub_5a7b1334799d9dbfb9cc0f46_5a7b135b57906a1b6eb710eb/scale_1200");
    c3->setPhone("+12345678");

    auto c4 = std::make_shared<Contact>();
    c4->setName("Erick Helicopter");
    c4->setEmail("heli@copt.er");

    auto c5 = std::make_shared<Contact>();
    c5->setName("Peter Griffin");

    Contacts contacts { c0, c1, c2, c3, c4, c5 };
    int i = 0;
    for (auto &c : contacts) {
        c->setUserId(UserId(QLatin1String("dummy/%1").arg(i)));
        c->setPlatformId(c->userId());
        ++i;
    }
    return contacts;
}
#endif // VS_DUMMY_CONTACTS
} // namespace

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

    } else if (std::holds_alternative<MessageContentGroupInvitation>(messageContent)) {
        return QObject::tr("...");

    } else {
        return {};
    }
}

QString Utils::printableLoadProgress(quint64 loaded, quint64 total)
{
    return QString("%1% (%2/%3)").arg(qRound(100 * qMin<double>(loaded, total) / total)).arg(loaded).arg(total);
}

QString Utils::printableContactsList(const Contacts &contacts)
{
    QStringList list;
    for (auto &c : contacts) {
        list << c->displayName();
    }
    return list.join(QLatin1String(","));
}

void Utils::printThreadId(const QString &message)
{
    qCDebug(lcUtils).noquote().nospace() << "Thread " << QThread::currentThread()->objectName() << "("
                                         << QThread::currentThreadId() << "): " << message;
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

Contacts Utils::getDeviceContacts(const Contacts &cachedContacts)
{
    Contacts contacts;
#if VS_DUMMY_CONTACTS
    contacts = getDummyContacts();
#elif VS_ANDROID
    contacts = VSQAndroid::getContacts();
#endif // VS_ANDROID
    return contacts;
}

QUrl Utils::getContactAvatarUrl(const ContactHandler contact)
{
#if VS_ANDROID
    return VSQAndroid::getContactAvatarUrl(contact);
#else
    Q_UNUSED(contact)
    return QUrl();
#endif // VS_ANDROID
}

QString Utils::displayUsername(const QString &username, const UserId &userId)
{
    return username.isEmpty() ? userId : username;
}
