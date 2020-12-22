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
{}

void Self::saveAs(const MessageId &messageId, const QVariant &fileUrl)
{
    const auto message = findMessageWithAttachment(messageId);
    if (!message) {
        return;
    }

    const auto attachment = std::get_if<MessageContentAttachment>(&message->content());

    switch (attachment->downloadStage()) {
        case MessageContentDownloadStage::Initial:
            downloadAttachment(message);
            break;

        case MessageContentDownloadStage::Downloaded:
            decryptAttachment(message);
            break;

        case MessageContentDownloadStage::Decrypted:
            saveAttachment(message, fileUrl.toUrl());
            break;
    }
}

void Self::download(const MessageId &messageId)
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        return;
    }
    else {
        downloadAttachment(message);
    }
}

void Self::open(const MessageId &messageId)
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        return;
    }

    const auto attachment = std::get_if<MessageContentAttachment>(&message->content());

    switch (attachment->downloadStage()) {
        case MessageContentDownloadStage::Initial:
            downloadAttachment(message);
            break;

        case MessageContentDownloadStage::Downloaded:
            decryptAttachment(message);
            break;

        case MessageContentDownloadStage::Decrypted: {
            const auto url = FileUtils::localFileToUrl(attachment->localPath());
            if (std::holds_alternative<MessageContentPicture>(message->content())) {
                qCDebug(lcController) << "Opening of preview for" << url.fileName();
                emit openPreviewRequested(url);
            }
            else {
                qCDebug(lcController) << "Opening of file:" << url.fileName();
                FileUtils::openUrl(url);
            }
            break;
        }
    }
}

void Self::downloadDisplayImage(const MessageId &messageId)
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        return;
    }

    qCDebug(lcController) << "Downloading of display image for:" << messageId;
    m_models->messagesQueue()->pushMessagePreload(message);
}

void Self::downloadAttachment(const MessageHandler &message)
{
    qCDebug(lcController) << "Downloading attachment of message: " << message->id();
    const auto attachment = std::get_if<MessageContentAttachment>(&message->content());
    const auto fileName = attachment->fileName();
    const auto filePath = FileUtils::findUniqueFileName(m_settings->downloadsDir().filePath(fileName));
    m_models->messagesQueue()->pushMessageDownload(message, filePath);
}

void Self::decryptAttachment(const MessageHandler &message)
{
    qCDebug(lcController) << "Decrypting attachment of message: " << message->id();
    const auto attachment = std::get_if<MessageContentAttachment>(&message->content());
    const auto fileName = attachment->fileName();
    const auto filePath = FileUtils::findUniqueFileName(m_settings->downloadsDir().filePath(fileName));
    m_models->messagesQueue()->pushMessageDownload(message, filePath); // FIXME: change to the separate "decrypt" only operation.
}

void Self::saveAttachment(const MessageHandler &message, const QUrl &fileUrl)
{
    qCDebug(lcController) << "Checking file consistency: " << message->id();

    const auto attachment = std::get_if<MessageContentAttachment>(&message->content());
    if (!FileUtils::fileExists(attachment->localPath()) || attachment->fingerprint().isEmpty()) {
        downloadAttachment(message);
        return;
    }

    const auto fingerprint = FileUtils::calculateFingerprint(attachment->localPath());
    if (fingerprint.isEmpty()) {
        qCritical(lcController()) << "Can not calculate fingerprint when saving file";
        emit notificationCreated(tr("Attachment file is broken"), true);
        return;
    }

    if (fingerprint != attachment->fingerprint()) {
        qCritical(lcController()) << "Fingerprint mismatch download file again";
        emit notificationCreated(tr("Attachment file is broken, downloading again..."), true);
        downloadAttachment(message);
        return;
    }

    qCDebug(lcController) << "Saving attachment of message: " << message->id();
    const auto filePath = FileUtils::urlToLocalFile(fileUrl);
    QFile::copy(attachment->localPath(), filePath);

    emit notificationCreated(tr("Attachment was saved"), false);
}

MessageHandler Self::findMessageWithAttachment(const MessageId &messageId) const
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        qCWarning(lcController) << "Message not found: " << messageId;
        return nullptr;
    }

    if (!std::holds_alternative<MessageContentAttachment>(message->content())) {
        qCWarning(lcController) << "Message has no attachment: " << messageId;
        return nullptr;
    }

    return message;
}
