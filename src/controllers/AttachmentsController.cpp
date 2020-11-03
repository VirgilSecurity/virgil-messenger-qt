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

#include "Utils.h"
#include "models/Models.h"
#include "models/MessagesModel.h"
#include "models/MessagesQueue.h"

using namespace vm;

AttachmentsController::AttachmentsController(Models *models, QObject *parent)
    : QObject(parent)
    , m_models(models)
{}

void AttachmentsController::saveAs(const Message::Id &messageId, const QVariant &fileUrl)
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        qWarning() << "Message not found! Id:" << messageId;
        return;
    }
    //pushMessageOptions(*message, { QueueFlag::SaveAttachmentAs, Utils::urlToLocalFile(fileUrl.toUrl()) });
}

void AttachmentsController::download(const Message::Id &messageId)
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        qWarning() << "Message not found! Id:" << messageId;
        return;
    }
    //pushMessageOptions(*message, { QueueFlag::DownloadAttachment, QString() });
}

void AttachmentsController::open(const Message::Id &messageId)
{
    const auto message = m_models->messages()->findById(messageId);
    if (!message) {
        qWarning() << "Message not found! Id:" << messageId;
        return;
    }
    //pushMessageOptions(*message, { QueueFlag::OpenAttachment, QString() });
}

void AttachmentsController::setUserId(const UserId &userId)
{
    m_userId = userId;
}

void AttachmentsController::setContactId(const Contact::Id &contactId)
{
    m_contactId = contactId;
}

//void AttachmentsController::pushMessageOptions(const Message &message, const QueueOptions &options)
//{
//    Contact::Id senderId = m_userId;
//    Contact::Id recipientId = m_contactId;
//    if (message.authorId == recipientId) {
//        std::swap(senderId, recipientId);
//    }
//    m_models->messagesQueue()->pushMessageOptions(message, senderId, recipientId, options);
//}
