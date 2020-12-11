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

#ifndef VM_MESSAGESCONTROLLER_H
#define VM_MESSAGESCONTROLLER_H

#include <QObject>

#include "Messages.h"
#include "Messenger.h"
#include "Models.h"
#include "UserDatabase.h"

namespace vm
{

class MessagesController : public QObject
{
    Q_OBJECT

public:
    MessagesController(Messenger *messenger, Models *models, UserDatabase *userDatabase, QObject *parent);

    void setUserId(const UserId &userId);
    void loadMessages(const Chat &chat);
    Q_INVOKABLE void createSendMessage(const QString &body, const QVariant &attachmentUrl, const Enums::AttachmentType attachmentType);

signals:
    void errorOccurred(const QString &errorText);
    void notificationCreated(const QString &notification, const bool error);

    void messageCreated(const GlobalMessage &message);
    void messageStatusChanged(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status &status);

    void displayImageNotFound(const QString &messageId);

private:
    void setupTableConnections();


    void setMessageStatus(const Message::Id &messageId, const Contact::Id &contactId, const Message::Status &status);
    void setAttachmentStatus(const Attachment::Id &attachmentId, const Contact::Id &contactId, const Attachment::Status &status);
    void setAttachmentUrl(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QUrl &url);
    void setAttachmentLocalPath(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QString &localPath);
    void setAttachmentFingerprint(const Attachment::Id &attachmentId, const Contact::Id &contactId, const QString &fingerpint);
    void setAttachmentExtras(const Attachment::Id &attachmentId, const Contact::Id &contactId, const Attachment::Type &type, const QVariant &extras);
    void setAttachmentProcessedSize(const Attachment::Id &attachmentId, const Contact::Id &contactId, const DataSize &processedSize);
    void setAttachmentEncryptedSize(const Attachment::Id &attachmentId, const Contact::Id &contactId, const DataSize &encryptedSize);

private slots:
    void onChatUpdated(const Chat &chat);
    void onMessageReceived(GlobalMessage message);
    void setDeliveredStatus(const QString &recipientId, const QString &messageId);

private:
    QPointer<Messenger> m_messenger;
    QPointer<Models> m_models;
    QPointer<UserDatabase> m_userDatabase;

    UserId m_userId;
    Chat m_chat;

    // NOTE(fpohtmeh): this workaround is needed when messages are received before chat list loading
    // it will be remove right after offline mode
    struct PostponedMessage
    {
        struct DeliverInfo
        {
            const QString recipientId;
            const QString messageId;
        };

        std::vector<GlobalMessage> receivedMessages;
        std::vector<DeliverInfo> deliverInfos;

        void addMessage(GlobalMessage message);
        void addDeliverInfo(QString recipientId, QString messageId);
        void process(MessagesController *controller);
    };

    PostponedMessage m_postponedMessages;
};
} // namespace vm

#endif // VM_MESSAGESCONTROLLER_H
