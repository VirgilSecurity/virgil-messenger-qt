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

#include "operations/MessageOperationFactory.h"

#include "Settings.h"
#include "Utils.h"
#include "Messenger.h"
#include "operations/CalculateAttachmentFingerprintOperation.h"
#include "operations/ConvertToPngOperation.h"
#include "operations/CreateAttachmentPreviewOperation.h"
#include "operations/CreateAttachmentThumbnailOperation.h"
#include "operations/CreateThumbnailOperation.h"
#include "operations/EncryptUploadFileOperation.h"
#include "operations/DownloadFileOperation.h"
#include "operations/DownloadDecryptFileOperation.h"
#include "operations/DownloadAttachmentOperation.h"
#include "operations/MessageOperation.h"
#include "operations/SendMessageOperation.h"
#include "operations/UploadAttachmentOperation.h"
#include "operations/UploadFileOperation.h"

using namespace vm;

using DownloadType = DownloadAttachmentOperation::Parameter::Type;

MessageOperationFactory::MessageOperationFactory(Messenger *messenger, QObject *parent)
    : QObject(parent), m_messenger(messenger)
{
}

void MessageOperationFactory::populateAll(MessageOperation *messageOp)
{
    populateAttachmentOperation(messageOp);
    populateMessageOperation(messageOp);
}

void MessageOperationFactory::populateDownload(MessageOperation *messageOp, const QString &filePath)
{
    messageOp->appendChild(
            new DownloadAttachmentOperation(messageOp, settings(), { DownloadType::Download, filePath }));
}

void MessageOperationFactory::populateUpload(MessageOperation *messageOp)
{
    messageOp->appendChild(new UploadAttachmentOperation(messageOp, settings()));
}

void MessageOperationFactory::populatePreload(MessageOperation *messageOp)
{
    messageOp->appendChild(
            new DownloadAttachmentOperation(messageOp, settings(), { DownloadType::Preload, QString() }));
}

DownloadDecryptFileOperation *
MessageOperationFactory::populateDownloadDecrypt(NetworkOperation *parent, const QUrl &url, quint64 bytesTotal,
                                                 const QString &destPath, const QByteArray &decryptionKey,
                                                 const QByteArray &signature, const UserId &senderId)
{
    auto op = new DownloadDecryptFileOperation(parent, m_messenger, url, bytesTotal, destPath, decryptionKey, signature,
                                               senderId);
    parent->appendChild(op);
    return op;
}

EncryptUploadFileOperation *MessageOperationFactory::populateEncryptUpload(NetworkOperation *parent,
                                                                           const QString &sourcePath)
{
    auto op = new EncryptUploadFileOperation(parent, m_messenger, sourcePath);
    parent->appendChild(op);
    return op;
}

ConvertToPngOperation *MessageOperationFactory::populateConvertToPngOperation(Operation *parent,
                                                                              const QString &sourcePath,
                                                                              const QString &destFileName)
{
    auto op = new ConvertToPngOperation(settings(), sourcePath, destFileName, parent);
    parent->appendChild(op);
    return op;
}

CreateAttachmentThumbnailOperation *
MessageOperationFactory::populateCreateAttachmentThumbnail(MessageOperation *messageOp, Operation *parent,
                                                           const QString &sourcePath, const QString &filePath)
{
    auto op = new CreateAttachmentThumbnailOperation(messageOp, settings(), sourcePath, filePath);
    parent->appendChild(op);
    return op;
}

CreateAttachmentPreviewOperation *MessageOperationFactory::populateCreateAttachmentPreview(MessageOperation *messageOp,
                                                                                           Operation *parent,
                                                                                           const QString &sourcePath,
                                                                                           const QString &destPath)
{
    auto op = new CreateAttachmentPreviewOperation(messageOp, settings(), sourcePath, destPath);
    parent->appendChild(op);
    return op;
}

CalculateAttachmentFingerprintOperation *
MessageOperationFactory::populateCalculateAttachmentFingerprint(MessageOperation *messageOp, Operation *parent,
                                                                const QString &sourcePath)
{
    auto op = new CalculateAttachmentFingerprintOperation(messageOp, sourcePath);
    parent->appendChild(op);
    return op;
}

void MessageOperationFactory::populateAttachmentOperation(MessageOperation *messageOp)
{
    const auto message = messageOp->message();
    if (!message->contentIsAttachment()) {
        return;
    }

    if (message->isOutgoing()) {
        populateUpload(messageOp);
    } else {
        if (std::holds_alternative<MessageContentPicture>(message->content())) {
            populatePreload(messageOp);
        }
    }
}

void MessageOperationFactory::populateMessageOperation(MessageOperation *messageOp)
{
    const auto message = messageOp->message();
    if (message->isOutgoing() && (message->status() == Message::Status::New)) {
        messageOp->appendChild(new SendMessageOperation(messageOp, m_messenger));
    }
}

const QPointer<Settings> MessageOperationFactory::settings() const
{
    return m_messenger->settings();
}
