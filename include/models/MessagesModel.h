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

#ifndef VM_MESSAGESMODEL_H
#define VM_MESSAGESMODEL_H

#include <QAbstractListModel>

#include "VSQCommon.h"

namespace vm
{
class MessagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit MessagesModel(QObject *parent);
    ~MessagesModel() override;

    void setUserId(const UserId &userId);
    void setContactId(const Contact::Id &contactId);

    void setMessages(const Messages &messages);
    Message createMessage(const Chat::Id &chatId, const Contact::Id &authorId, const QString &body, const Optional<Attachment> &attachment);
    void writeMessage(const Message &message);

    // Sets message status. Returns false if message had the same status
    bool setMessageStatus(const Message::Id &messageId, const Message::Status &status);
    void markAllAsRead();

    void setAttachmentStatus(const Attachment::Id &attachmentId, const Attachment::Status &status);
    void setAttachmentUrl(const Attachment::Id &attachmentId, const QUrl &url);
    void setAttachmentExtras(const Attachment::Id &attachmentId, const QVariant &extras);
    void setAttachmentLocalPath(const Attachment::Id &attachmentId, const QString &localPath);
    void setAttachmentEncryptedSize(const Attachment::Id &attachmentId, const DataSize &size);
    void setAttachmentProcessedSize(const Attachment::Id &attachmentId, const DataSize &size);

    Optional<GlobalMessage> findById(const Message::Id &messageId) const;

signals:
    void displayImageNotFound(const Message::Id &messageId) const;

private:
    enum Roles
    {
        IdRole = Qt::UserRole,
        DayRole,
        DisplayTimeRole,
        AuthorIdRole,
        StatusRole,
        BodyRole,
        // Attachment
        AttachmentIdRole,
        AttachmentTypeRole,
        AttachmentStatusRole,
        AttachmentImagePathRole,
        AttachmentImageSizeRole,
        AttachmentDisplaySizeRole,
        AttachmentDisplayTextRole,
        // Loading
        AttachmentBytesTotalRole,
        AttachmentBytesLoadedRole,
        AttachmentFileExistsRole,
        //
        FailedRole,
        FirstInRowRole,
        InRowRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateAttachment(const Attachment::Id &attachmentId, const QVector<int> &roles, const std::function<bool (Attachment &)> &update);

    Optional<int> findRowById(const Message::Id &messageId) const;
    Optional<int> findRowByAttachmentId(const Attachment::Id &attachmentId) const;
    void invalidateRow(const int row, const QVector<int> &roles = {});
    void invalidateModel(const QModelIndex &index, const QVector<int> &roles);

    Messages m_messages;
    UserId m_userId;
    Contact::Id m_contactId;
};
}

#endif // VM_MESSAGESMODEL_H
