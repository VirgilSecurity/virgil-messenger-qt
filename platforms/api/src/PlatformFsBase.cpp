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

#include "PlatformFsBase.h"

#include <QtCore>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcPlatformFsBase, "platform-fs-base");

using namespace vm;
using namespace platform;

using Self = PlatformFsBase;

QString Self::urlToLocalFile(const QUrl &url) const
{
    qCDebug(lcPlatformFsBase) << "File URL (before urlToLocalFile):" << url.toString();
    const auto res = url.toLocalFile();
    qCDebug(lcPlatformFsBase) << "File path (after urlToLocalFile):" << res;
    return res;
}

QUrl Self::localFileToUrl(const QString &path) const
{
    qCDebug(lcPlatformFsBase) << "File URL (before urlToLocalFile):" << path;
    const auto url = QUrl::fromLocalFile(path);
    qCDebug(lcPlatformFsBase) << "File path (after urlToLocalFile):" << url.toString();
    return url;
}

QString Self::fileDisplayName(const QUrl &url, bool /* isPicture */) const
{
    const auto filePath = urlToLocalFile(url);
    const auto res = QFileInfo(filePath).fileName();
    return res;
}

bool Self::requestExternalStorageWritePermission() const
{
    return true;
}
