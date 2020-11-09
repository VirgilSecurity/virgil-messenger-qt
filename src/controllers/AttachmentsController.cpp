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

#include <QDesktopServices>

#include "Settings.h"
#include "Utils.h"
#include "models/Models.h"
#include "models/MessagesModel.h"
#include "models/MessagesQueue.h"

using namespace vm;

AttachmentsController::AttachmentsController(Settings *settings, Models *models, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_models(models)
{}

void AttachmentsController::saveAs(const Message::Id &messageId, const QVariant &fileUrl)
{
    if (const auto message = findMessage(messageId)) {
        const auto &a = *message->attachment;
        if (a.localPath.isEmpty() || !QFile::exists(a.localPath)) {
            downloadAttachment(*message);
        }
        else {
            qCDebug(lcController) << "Saving of attachment" << messageId << "as" << fileUrl;
            const auto filePath = Utils::urlToLocalFile(fileUrl.toUrl());
            QFile::copy(a.localPath, filePath);
            emit notificationCreated(tr("Attachment was saved"));
        }
    }
}

void AttachmentsController::download(const Message::Id &messageId)
{
    if (const auto message = findMessage(messageId)) {
        downloadAttachment(*message);
    }
}

void AttachmentsController::open(const Message::Id &messageId)
{
    if (const auto message = findMessage(messageId)) {
        const auto &a = *message->attachment;
        if (a.localPath.isEmpty() || !QFile::exists(a.localPath)) {
            downloadAttachment(*message);
        }
        else {
            const auto url = Utils::localFileToUrl(a.localPath);
            if (a.type == Attachment::Type::Picture) {
                qCDebug(lcController) << "Opening of preview for" << url;
                emit openPreviewRequested(url);
            }
            else {
                qCDebug(lcController) << "Opening of url:" << url;
                QDesktopServices::openUrl(url);
            }
        }
    }
}

void AttachmentsController::setUserId(const UserId &userId)
{
    m_userId = userId;
}

void AttachmentsController::setContactId(const Contact::Id &contactId)
{
    m_contactId = contactId;
}

void AttachmentsController::downloadAttachment(const GlobalMessage &message)
{
    qCDebug(lcController) << "Downloading of attachment" << message.id;
    const auto fileName = message.attachment->fileName;
    const auto filePath = Utils::findUniqueFileName(m_settings->downloadsDir().filePath(fileName));
    m_models->messagesQueue()->pushMessageDownload(message, filePath);
}

Optional<GlobalMessage> AttachmentsController::findMessage(const Message::Id &messageId) const
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        qCWarning(lcController) << "Message not found:" << messageId;
        return NullOptional;
    }
    if (!message->attachment) {
        qCWarning(lcController) << "Message has not attachment:" << messageId;
        return NullOptional;
    }
    return message;
}
