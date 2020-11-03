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
#include "VSQMessenger.h"
#include "operations/CopyFileOperation.h"
#include "operations/DownloadFileOperation.h"
#include "operations/DecryptFileOperation.h"
#include "operations/EncryptFileOperation.h"
#include "operations/MessageOperation.h"
#include "operations/OpenPreviewOperation.h"
#include "operations/SendMessageOperation.h"
#include "operations/UploadFileOperation.h"

using namespace vm;

MessageOperationFactory::MessageOperationFactory(const VSQSettings *settings, VSQMessenger *messenger, FileLoader *fileLoader,
                                                 QObject *parent)
    : QObject(parent)
    , m_cacheDir(settings->attachmentCacheDir())
    , m_messenger(messenger)
    , m_fileLoader(fileLoader)
{}

SendMessageOperation *MessageOperationFactory::createSendMessageOperation(MessageOperation *parent)
{
    return new SendMessageOperation(parent, m_messenger->xmpp(), m_messenger->xmppURL());
}

EncryptFileOperation *MessageOperationFactory::createEncryptFileOperation(MessageOperation *parent)
{
    return new EncryptFileOperation(parent, m_cacheDir);
}

DecryptFileOperation *MessageOperationFactory::createDecryptFileOperation(MessageOperation *parent, const QString &encFilePath, const QString &filePath)
{
    return new DecryptFileOperation(parent, encFilePath, filePath);
}

UploadFileOperation *MessageOperationFactory::createUploadFileOperation(MessageOperation *parent)
{
    return new UploadFileOperation(parent, m_fileLoader);
}

DownloadFileOperation *MessageOperationFactory::createDownloadFileOperation(MessageOperation *parent, const QString &filePath)
{
    return new DownloadFileOperation(parent, m_fileLoader, filePath);
}

MakeThumbnailOperation *MessageOperationFactory::createMakeThumbnailOperation(MessageOperation *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}

OpenPreviewOperation *MessageOperationFactory::createOpenPreviewOperation(MessageOperation *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}

CopyFileOperation *MessageOperationFactory::createCopyFileOperation(MessageOperation *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}

void MessageOperationFactory::populateChildren(MessageOperation *messageOp)
{
    populateAttachmentOperations(messageOp);
    auto firstChild = messageOp->firstChild();
    if (firstChild) {
        connect(firstChild, &Operation::started, messageOp, std::bind(&MessageOperation::setAttachmentStatus, messageOp, Attachment::Status::Loading));
        auto lastChild = messageOp->lastChild();
        connect(lastChild, &Operation::finished, messageOp, std::bind(&MessageOperation::setAttachmentStatus, messageOp, Attachment::Status::Loaded));
    }

    populateMessageOperations(messageOp);
}

void MessageOperationFactory::populateDownloadDecryptChildren(MessageOperation *messageOp, const QString &filePath)
{
    const auto tempFilePath = m_cacheDir.filePath("dl-dec-" + messageOp->message()->attachment->id);
    auto downloadOp = createDownloadFileOperation(messageOp, tempFilePath);
    connect(downloadOp, &Operation::started, messageOp, std::bind(&MessageOperation::setAttachmentStatus, messageOp, Attachment::Status::Loading));
    messageOp->appendChild(downloadOp);

    auto decryptOp = createDecryptFileOperation(messageOp, tempFilePath, filePath);
    connect(decryptOp, &Operation::finished, messageOp, std::bind(&MessageOperation::setAttachmentLocalPath, messageOp, filePath));
    connect(decryptOp, &Operation::finished, messageOp, std::bind(&MessageOperation::setAttachmentStatus, messageOp, Attachment::Status::Loaded));
    messageOp->appendChild(decryptOp);
}

void MessageOperationFactory::populateAttachmentOperations(MessageOperation *messageOp)
{
    const auto message = messageOp->message();
    const auto attachment = message->attachment;
    if (!attachment) {
        return;
    }
    const bool needSending = message->senderId == message->userId && (message->status == Message::Status::Created || message->status == Message::Status::Failed);
    if (needSending) {
        if (attachment->type == Attachment::Type::Picture) {
            populateSendPictureOperations(messageOp);
        }
        else if (attachment->type == Attachment::Type::File) {
            populateSendFileOperations(messageOp);
        }
        return;
    }
}

void MessageOperationFactory::populateSendPictureOperations(MessageOperation *messageOp)
{
}

void MessageOperationFactory::populateSendFileOperations(MessageOperation *messageOp)
{
    if (messageOp->message()->attachment->url.isEmpty()) {
        auto encryptionOperation = createEncryptFileOperation(messageOp);
        messageOp->appendChild(encryptionOperation);

        auto uploadOperation = createUploadFileOperation(messageOp);
        uploadOperation->setAutoDeleteFile(true);
        messageOp->appendChild(uploadOperation);

        connect(encryptionOperation, &EncryptFileOperation::fileEncrypted, uploadOperation, &UploadFileOperation::setFilePath);
    }
}

void MessageOperationFactory::populateMessageOperations(MessageOperation *messageOp)
{
    const auto message = messageOp->message();
    // Send message operation
    const bool needSending = message->senderId == message->userId && (message->status == Message::Status::Created || message->status == Message::Status::Failed);
    if (needSending) {
        messageOp->appendChild(createSendMessageOperation(messageOp));
    }
}
