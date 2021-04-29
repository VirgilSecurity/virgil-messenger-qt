//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#include "PlatformFsAndroid.h"

#include <QtCore>
#include <QtAndroid>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcPlatformFsAndroid, "platform-fs-android");

using namespace vm;
using namespace platform;

using Self = PlatformFsAndroid;

PlatformFs &PlatformFs::instance()
{
    static Self impl;
    return impl;
}

QString Self::urlToLocalFile(const QUrl &url) const
{
    qCDebug(lcPlatformFsAndroid) << "File URL (before urlToLocalFile):" << url.toString();
    const auto options = url.isLocalFile() ? QUrl::FormattingOptions(QUrl::RemoveScheme) : QUrl::PrettyDecoded;
    const auto res = QUrl::fromPercentEncoding(url.toString(options).toUtf8());
    qCDebug(lcPlatformFsAndroid) << "File URL path (after urlToLocalFile):" << res;
    return res;
}

QString Self::fileDisplayName(const QUrl &url, bool /* isPicture */) const
{
    const QString urlString = url.toString();
    const auto javaUrl = QAndroidJniObject::fromString(urlString);
    const auto javaDisplayName =
            QAndroidJniObject::callStaticObjectMethod("org/virgil/utils/Utils", "getDisplayName",
                                                      "(Landroid/content/Context;Ljava/lang/String;)Ljava/lang/String;",
                                                      QtAndroid::androidContext().object(), javaUrl.object<jstring>());
    return javaDisplayName.toString();
}
