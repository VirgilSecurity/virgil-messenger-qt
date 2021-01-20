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

#ifndef VS_MESSAGEOPERATIONFACTORY_H
#define VS_MESSAGEOPERATIONFACTORY_H

#include <QDir>
#include <QPointer>

#include "Messenger.h"

class Settings;

namespace vm
{
class Messenger;
class CalculateAttachmentFingerprintOperation;
class ConvertToPngOperation;
class CreateAttachmentPreviewOperation;
class CreateAttachmentThumbnailOperation;
class CreateThumbnailOperation;
class DecryptFileOperation;
class DownloadFileOperation;
class DownloadDecryptFileOperation;
class EncryptFileOperation;
class EncryptUploadFileOperation;
class MessageOperation;
class NetworkOperation;
class Operation;
class SendMessageOperation;
class UploadFileOperation;

class MessageOperationFactory : public QObject
{
    Q_OBJECT

public:
    MessageOperationFactory(Messenger *messenger, QObject *parent);

    void populateAll(MessageOperation *messageOp);
    void populateDownload(MessageOperation *messageOp, const QString &filePath);
    void populateUpload(MessageOperation *messageOp);
    void populatePreload(MessageOperation *messageOp);

    DownloadDecryptFileOperation *populateDownloadDecrypt(NetworkOperation *parent, const QUrl &url, quint64 bytesTotal,
                                                          const QString &destPath, const QByteArray &decryptionKey, const QByteArray& signature, const UserId &senderId);
    EncryptUploadFileOperation *populateEncryptUpload(NetworkOperation *parent, const QString &sourcePath);
    ConvertToPngOperation *populateConvertToPngOperation(Operation *parent, const QString &sourcePath, const QString &destFileName);
    CreateAttachmentThumbnailOperation *populateCreateAttachmentThumbnail(MessageOperation *messageOp, Operation *parent, const QString &sourcePath, const QString &filePath);
    CreateAttachmentPreviewOperation *populateCreateAttachmentPreview(MessageOperation *messageOp, Operation *parent, const QString &sourcePath, const QString &destPath);
    CalculateAttachmentFingerprintOperation *populateCalculateAttachmentFingerprint(MessageOperation *messageOp, Operation *parent, const QString &sourcePath);

private:
    SendMessageOperation *createSendMessageOperation(MessageOperation *parent);
    void populateAttachmentOperation(MessageOperation *messageOp);
    void populateMessageOperation(MessageOperation *messageOp);
    void populateDownloadOperation(MessageOperation *messageOp);

    const QPointer<Settings> settings() const;

    QPointer<Messenger> m_messenger;
};
}

#endif // VS_MESSAGEOPERATIONFACTORY_H
