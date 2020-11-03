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

#include "VSQCommon.h"

class VSQMessenger;
class VSQSettings;

namespace vm
{
class CopyFileOperation;
class DecryptFileOperation;
class DownloadFileOperation;
class EncryptFileOperation;
class MakeThumbnailOperation;
class MessageOperation;
class OpenPreviewOperation;
class Operation;
class SendMessageOperation;
class UploadFileOperation;
class FileLoader;

class MessageOperationFactory : public QObject
{
    Q_OBJECT

public:
    explicit MessageOperationFactory(const VSQSettings *settings, VSQMessenger *messenger, FileLoader *fileLoader,
                                     QObject *parent);

    SendMessageOperation *createSendMessageOperation(MessageOperation *parent);
    EncryptFileOperation *createEncryptFileOperation(MessageOperation *parent);
    DecryptFileOperation *createDecryptFileOperation(MessageOperation *parent, const QString &encFilePath, const QString &filePath);
    UploadFileOperation *createUploadFileOperation(MessageOperation *parent);
    DownloadFileOperation *createDownloadFileOperation(MessageOperation *parent, const QString &filePath);
    MakeThumbnailOperation *createMakeThumbnailOperation(MessageOperation *parent);
    OpenPreviewOperation *createOpenPreviewOperation(MessageOperation *parent);
    CopyFileOperation *createCopyFileOperation(MessageOperation *parent);

    void populateChildren(MessageOperation *messageOp);
    void populateDownloadDecryptChildren(MessageOperation *messageOp, const QString &filePath);

private:
    void populateAttachmentOperations(MessageOperation *messageOp);
    void populateSendPictureOperations(MessageOperation *messageOp);
    void populateSendFileOperations(MessageOperation *messageOp);
    void populateMessageOperations(MessageOperation *messageOp);

    const QDir m_cacheDir;
    VSQMessenger *m_messenger;
    FileLoader *m_fileLoader;
};
}

#endif // VS_MESSAGEOPERATIONFACTORY_H
