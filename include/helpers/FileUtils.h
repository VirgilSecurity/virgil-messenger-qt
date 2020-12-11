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

#ifndef VM_FILE_UTILS_H
#define VM_FILE_UTILS_H

#include <Messages.h>

#include <QString>


Q_DECLARE_LOGGING_CATEGORY(lcFileUtils);


namespace vm
{
class FileUtils {
public:
    static Optional<QString> calculateFingerprint(const QString &path);

    static QString findUniqueFileName(const QString &fileName);

    static bool forceCreateDir(const QString &absolutePath);

    static Optional<QString> readTextFile(const QString &filePath);

    static bool fileExists(const QString &filePath);

    static void removeFile(const QString &filePath);

    static QString attachmentFileName(const QUrl &url, const QFileInfo &localInfo, bool isPicture);

    static QString attachmentDisplayImagePath(const Attachment &attachment);

    static bool openUrl(const QUrl &url);

    static bool isValidUrl(const QUrl &url);

    static QString urlToLocalFile(const QUrl &url);

    static QUrl localFileToUrl(const QString &filePath);
};
}; // vm

#endif // VM_FILE_UTILS_H
