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

#ifndef VM_DOWNLOADDECRYPTFILEOPERATION_H
#define VM_DOWNLOADDECRYPTFILEOPERATION_H

#include "NetworkOperation.h"
#include "FileLoader.h"

#include <QPointer>

class Settings;

namespace vm
{

class DownloadDecryptFileOperation : public NetworkOperation
{
    Q_OBJECT

public:
    DownloadDecryptFileOperation(NetworkOperation *parent, const Settings *settings, FileLoader *fileLoader,
                                 const QUrl &url, const quint64 &bytesTotal, const QString &filePath, const UserId &senderId);

signals:
    void progressChanged(const quint64 &bytesLoaded, const quint64 &bytesTotal);
    void decrypted(const QString &filePath);

private:
    bool populateChildren() override;
    void cleanup() override;

    const Settings *m_settings;
    QPointer<FileLoader> m_fileLoader;
    const QUrl m_url;
    const quint64 m_bytesTotal;
    QString m_tempPath;
    const QString m_filePath;
    const UserId m_senderId;
};
}

#endif // VM_DOWNLOADDECRYPTFILEOPERATION_H
