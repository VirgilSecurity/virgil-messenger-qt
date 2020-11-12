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
#include "VSQMessenger.h"
#include "operations/CalculateAttachmentFingerprintOperation.h"
#include "operations/ConvertToPngOperation.h"
#include "operations/CreateAttachmentPreviewOperation.h"
#include "operations/CreateAttachmentThumbnailOperation.h"
#include "operations/CreateThumbnailOperation.h"
#include "operations/DecryptFileOperation.h"
#include "operations/EncryptFileOperation.h"
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

MessageOperationFactory::MessageOperationFactory(const Settings *settings, VSQMessenger *messenger, FileLoader *fileLoader,
                                                 QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_messenger(messenger)
    , m_fileLoader(fileLoader)
{}

void MessageOperationFactory::populateAll(MessageOperation *messageOp)
{
    populateAttachmentOperation(messageOp);
    populateMessageOperation(messageOp);
}

void MessageOperationFactory::populateDownload(MessageOperation *messageOp, const QString &filePath)
{
    messageOp->appendChild(new DownloadAttachmentOperation(messageOp, m_settings, { DownloadType::Full, filePath }));
}

void MessageOperationFactory::populateUpload(MessageOperation *messageOp)
{
    messageOp->appendChild(new UploadAttachmentOperation(messageOp, m_settings));
}

void MessageOperationFactory::populatePreload(MessageOperation *messageOp)
{
    messageOp->appendChild(new DownloadAttachmentOperation(messageOp, m_settings, { DownloadType::Preload, QString() }));
}

DownloadDecryptFileOperation *MessageOperationFactory::populateDownloadDecrypt(const QString &name, Operation *parent, const QUrl &url, const DataSize &bytesTotal, const QString &destPath, const Contact::Id &senderId)
{
    auto op = new DownloadDecryptFileOperation(name, parent, m_settings, m_fileLoader, url, bytesTotal, destPath, senderId);
    parent->appendChild(op);
    return op;
}

EncryptUploadFileOperation *MessageOperationFactory::populateEncryptUpload(const QString &name, Operation *parent, const QString &sourcePath, const Contact::Id &recipientId)
{
    auto op = new EncryptUploadFileOperation(name, parent, m_settings, sourcePath, recipientId, m_fileLoader);
    parent->appendChild(op);
    return op;
}

ConvertToPngOperation *MessageOperationFactory::populateConvertToPngOperation(const QString &name, Operation *parent, const QString &sourcePath)
{
    auto op = new ConvertToPngOperation(name, m_settings, sourcePath, parent);
    parent->appendChild(op);
    return op;
}

CreateAttachmentThumbnailOperation *MessageOperationFactory::populateCreateAttachmentThumbnail(const QString &name, MessageOperation *messageOp, Operation *parent, const QString &sourcePath, const QString &filePath)
{
    auto op = new CreateAttachmentThumbnailOperation(name, messageOp, m_settings, sourcePath, filePath);
    parent->appendChild(op);
    return op;
}

CreateAttachmentPreviewOperation *MessageOperationFactory::populateCreateAttachmentPreview(const QString &name, MessageOperation *messageOp, Operation *parent, const QString &sourcePath, const QString &destPath)
{
    auto op = new CreateAttachmentPreviewOperation(name, messageOp, m_settings, sourcePath, destPath);
    parent->appendChild(op);
    return op;
}

CalculateAttachmentFingerprintOperation *MessageOperationFactory::populateCalculateAttachmentFingerprint(const QString &name, MessageOperation *messageOp, Operation *parent, const QString &sourcePath)
{
    auto op = new CalculateAttachmentFingerprintOperation(name, messageOp, sourcePath);
    parent->appendChild(op);
    return op;
}

void MessageOperationFactory::populateAttachmentOperation(MessageOperation *messageOp)
{
    const auto &m = *messageOp->message();
    if (!m.attachment) {
        return;
    }
    const auto &a = *m.attachment;
    const bool isPicture = a.type == Attachment::Type::Picture;
    if (m.senderId == m.userId) {
        if (m.status == Message::Status::Created || m.status == Message::Status::Failed) {
            populateUpload(messageOp);
        }
    }
    else if (m.senderId == m.contactId) {
        if (isPicture && (m.status == Message::Status::Created || m.status == Message::Status::Read)) {
            populatePreload(messageOp);
        }
    }
}

void MessageOperationFactory::populateMessageOperation(MessageOperation *messageOp)
{
    const auto &message = messageOp->message();
    if (message->senderId == message->userId && (message->status == Message::Status::Created || message->status == Message::Status::Failed)) {
        messageOp->appendChild(new SendMessageOperation(messageOp, m_messenger->xmpp(), m_messenger->xmppURL()));
    }
}
