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

#include "operations/MessageOperation.h"

#include "operations/MessageOperationFactory.h"
#include "operations/SendMessageOperation.h"

using namespace vm;

MessageOperation::MessageOperation(const GlobalMessage &message, MessageOperationFactory *factory, NetworkOperation *parent)
    : NetworkOperation(parent)
    , m_factory(factory)
    , m_message(message)
{
    setName(message.id);
}

const GlobalMessage *MessageOperation::message() const
{
    return &m_message;
}

const Attachment *MessageOperation::attachment() const
{
    return &*m_message.attachment;
}

MessageOperationFactory *MessageOperation::factory()
{
    return m_factory;
}

void MessageOperation::setAttachmentStatus(const Attachment::Status status)
{
    auto a = writableAttachment();
    if (a->status == status) {
        return;
    }

    auto warnInvalidStatus = [=]() {
        qCWarning(lcOperation) << "Unsupported attachment status change:" << a->status << "=>" << status;;
    };

    switch (status) {
    case Attachment::Status::Created:
        warnInvalidStatus();
        return;
    case Attachment::Status::Loading:
        if (a->status == Attachment::Status::Invalid) {
            warnInvalidStatus();
            return;
        }
        break;
    case Attachment::Status::Loaded:
        if (a->status != Attachment::Status::Created && a->status != Attachment::Status::Loading) {
            warnInvalidStatus();
            return;
        }
        break;
    case Attachment::Status::Interrupted:
        if (a->status != Attachment::Status::Created && a->status != Attachment::Status::Loading) {
            warnInvalidStatus();
            return;
        }
        break;
    case Attachment::Status::Invalid:
        if (a->status != Attachment::Status::Loading) {
            warnInvalidStatus();
            return;
        }
        break;
    default:
        warnInvalidStatus();
        return;
    }

    a->status = status;
    emit attachmentStatusChanged(status);
}

void MessageOperation::setAttachmentUrl(const QUrl &url)
{
    auto a = writableAttachment();
    if (a->url == url) {
        return;
    }
    a->url = url;
    emit attachmentUrlChanged(url);
}

void MessageOperation::setAttachmentLocalPath(const QString &localPath)
{
    auto a = writableAttachment();
    // NOTE(fpohtmeh): don't compare values because file existence can be changed
    a->localPath = localPath;
    emit attachmentLocalPathChanged(localPath);
}

void MessageOperation::setAttachmentFignerprint(const QString &fingerprint)
{
    auto a = writableAttachment();
    if (a->fingerprint == fingerprint) {
        return;
    }
    a->fingerprint = fingerprint;
    emit attachmentFingerprintChanged(fingerprint);
}

void MessageOperation::setAttachmentExtras(const QVariant &extras)
{
    auto a = writableAttachment();
    // NOTE(fpohtmeh): don't compare values because file existence can be changed
    a->extras = QVariant::fromValue(extras);
    emit attachmentExtrasChanged(extras);
}

void MessageOperation::setAttachmentPreviewPath(const QString &previewPath)
{
    auto extras = writableAttachment()->extras.value<PictureExtras>();
    extras.previewPath = previewPath;
    setAttachmentExtras(QVariant::fromValue(extras));
}

void MessageOperation::setAttachmentThumbnailPath(const QString &thumbnailPath)
{
    auto extras = writableAttachment()->extras.value<PictureExtras>();
    extras.thumbnailPath = thumbnailPath;
    setAttachmentExtras(QVariant::fromValue(extras));
}

void MessageOperation::setAttachmentThumbnailUrl(const QUrl &thumbnailUrl)
{
    auto extras = writableAttachment()->extras.value<PictureExtras>();
    extras.thumbnailUrl = thumbnailUrl;
    setAttachmentExtras(QVariant::fromValue(extras));
}

void MessageOperation::setAttachmentProcessedSize(const DataSize &size)
{
    auto a = writableAttachment();
    if (a->processedSize == size) {
        return;
    }
    a->processedSize = size;
    emit attachmentProcessedSizeChanged(size);
}

void MessageOperation::setAttachmentEncryptedSize(const DataSize &size)
{
    auto a = writableAttachment();
    if (a->encryptedSize == size) {
        return;
    }
    a->encryptedSize = size;
    emit attachmentEncryptedSizeChanged(size);
}

void MessageOperation::setAttachmentEncryptedThumbnailSize(const DataSize &bytes)
{
    auto extras = writableAttachment()->extras.value<PictureExtras>();
    extras.encryptedThumbnailSize = bytes;
    setAttachmentExtras(QVariant::fromValue(extras));
}

void MessageOperation::connectChild(Operation *child)
{
    Operation::connectChild(child);
    connect(child, &Operation::failed, this, std::bind(&MessageOperation::setStatus, this, Message::Status::Failed));
    connect(child, &Operation::invalidated, this, std::bind(&MessageOperation::setStatus, this, Message::Status::InvalidM));
    if (dynamic_cast<SendMessageOperation *>(child)) {
        connect(child, &Operation::finished, this, std::bind(&MessageOperation::setStatus, this, Message::Status::Sent));
    }
}

Attachment *MessageOperation::writableAttachment()
{
    return &*m_message.attachment;
}

void MessageOperation::setStatus(const Message::Status &status)
{
    if (m_message.status == status) {
        return;
    }
    m_message.status = status;
    emit statusChanged(status);
}
