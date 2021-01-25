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

#include "controllers/AttachmentsController.h"

#include "Settings.h"
#include "Utils.h"
#include "FileUtils.h"
#include "models/Models.h"
#include "models/MessagesModel.h"
#include "models/MessagesQueue.h"
#include "MessageContentAttachment.h"

#include "Controller.h"

using namespace vm;
using Self = AttachmentsController;

Self::AttachmentsController(const Settings *settings, Models *models, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_models(models)
{
}

void Self::saveAs(const QString &messageId, const QVariant &fileUrl)
{
    // TODO(fpohtmeh): don't use messageId, use proxyRow
    const auto message = findMessageById(messageId);
    if (!message) {
        return;
    }

    const auto filePath = FileUtils::urlToLocalFile(fileUrl.toUrl());
    downloadAttachment(message, [this, message, filePath]() {
        QFile::copy(message->contentAsAttachment()->localPath(), filePath);
        emit notificationCreated(tr("Attachment was saved"), false);
    });
}

void Self::download(const QString &messageId)
{
    const auto message = findMessageById(messageId);
    if (!message) {
        return;
    }

    downloadAttachment(message, [this]() {
        emit notificationCreated(tr("File was downloaded"), false);
    });
}

void Self::open(const QString &messageId)
{
    const auto message = findMessageById(messageId);
    if (!message) {
        return;
    }

    downloadAttachment(message, [this, message]() {
        const auto attachment = message->contentAsAttachment();
        const auto url = FileUtils::localFileToUrl(attachment->localPath());
        if (std::holds_alternative<MessageContentPicture>(message->content())) {
            qCDebug(lcController) << "Opening of preview for" << url.fileName();
            emit openPreviewRequested(url);
        }
        else {
            qCDebug(lcController) << "Opening of file:" << url.fileName();
            FileUtils::openUrl(url);
        }
    });
}

ModifiableMessageHandler Self::findMessageById(const QString &messageId) const
{
    const auto message = m_models->messages()->findById(MessageId{ messageId });
    if (!message) {
        qCWarning(lcController) << "Message not found: " << messageId;
        return nullptr;
    }

    if (!message->contentIsAttachment()) {
        qCWarning(lcController) << "Message has no attachment: " << messageId;
        return nullptr;
    }

    return message;
}

void AttachmentsController::downloadAttachment(const ModifiableMessageHandler &message, const MessagesQueue::PostFunction &function)
{
    const auto attachment = message->contentAsAttachment();
    auto localPath = attachment->localPath();

    // Check if downloaded is needed
    bool needDownload = false;
    if (!FileUtils::fileExists(localPath)) {
        needDownload = true;
    }
    else if (const auto fingerprint = FileUtils::calculateFingerprint(localPath); fingerprint.isEmpty()) {
        qCritical(lcController) << "Failed to calculate fingerprint for file:" << localPath;
        emit notificationCreated(tr("Attachment file is broken"), true);
        return;
    }
    else if (fingerprint != attachment->fingerprint()) {
        qWarning(lcController) << "Fingerprint mismatch, downloading attachment";
        emit notificationCreated(tr("Downloading attachment..."), true);
        needDownload = true;
    }

    if (!needDownload) {
        function();
    }
    else {
        localPath = FileUtils::findUniqueFileName(m_settings->downloadsDir().filePath(attachment->fileName()));

        m_models->messagesQueue()->pushMessageDownload(message, localPath, function);
    }
}
