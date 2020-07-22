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

#ifndef VSQ_CONVERSATIONSMODEL_H
#define VSQ_CONVERSATIONSMODEL_H

#include <QSqlQueryModel>

#include "Common.h"

class AttachmentsModel;
class ChatsModel;

class ConversationsModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    enum Columns
    {
        IdColumn = 0,
        AuthorColumn,
        ChatIdColumn,
        TimestampColumn,
        MessageColumn,
        StatusColumn,
        AttachmentIdColumn,
        AttachmentSizeColumn,
        AttachmentTypeColumn,
        AttachmentLocalUrlColumn,
        AttachmentLocalPreviewColumn,
        FirstMessageInARowColumn,
        MessageInARowColumn,
        DayColumn
    };

    explicit ConversationsModel(ChatsModel *chats, AttachmentsModel *attachments, QObject *parent = nullptr);

    QString user() const;
    void setUser(const QString &userId);

    void filterByChat(const QString &chatId);

public slots:
    void createMessage(const QString &messageId, const QString &message, const OptionalAttachment &attachment, const QString &contactId);
    void receiveMessage(const QString &messageId, const QString &message, const OptionalAttachment &attachment, const QString &contactId);
    void markAsRead(const QString &chatId);

    int getMessageCount(const QString &chatId, const EnMessageStatus status);
    void setMessageStatus(const QString &messageId, const EnMessageStatus status);

    std::vector<StMessage> getMessages(const QString &user, const EnMessageStatus status);

private:
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void createTable();
    void resetModel(const QString &chatId);
    QSqlQuery buildQuery(const QString &chatId, const QString &condition) const;
    void writeMessage(const QString &messageId, const QString &message, const OptionalAttachment &attachment, const QString &chatId,
                      const StMessage::Author author, const EnMessageStatus status);

    ChatsModel *m_chats;
    AttachmentsModel *m_attachments;

    QString m_userId;
    QString m_tableName;
    QString m_chatId;
};

#endif // VSQ_CONVERSATIONSMODEL_H
