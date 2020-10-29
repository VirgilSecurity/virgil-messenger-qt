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

#ifndef VM_UTILS_H
#define VM_UTILS_H

#include "VSQCommon.h"

namespace vm
{
namespace Utils
{
    QString createUuid();

    // String processing/format

    QString formattedDataSize(DataSize fileSize);

    QString formattedLastSeenActivity(const Seconds &seconds, const Seconds &updateInterval);

    QString formattedLastSeenNoActivity();

    QString elidedText(const QString &text, const int maxLength);

    QString attachmentDisplayText(const Attachment &attachment);

    // File functions

    QString findUniqueFileName(const QString &fileName);

    bool forceCreateDir(const QString &absolutePath);

    Optional<QString> readTextFile(const QString &filePath);

    // Url functions

    bool isValidUrl(const QUrl &url);

    QString urlToLocalFile(const QUrl &url);

    QUrl localFileToUrl(const QString &filePath);

    // Debug

    void printThreadId(const QString &message);

    // JSON functions

    QString extrasToJson(const QVariant &extras, const Attachment::Type type);

    QVariant extrasFromJson(const QString &json, const Attachment::Type type);
}
}

#endif // VM_UTILS_H
