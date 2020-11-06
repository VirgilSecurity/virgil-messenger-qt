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

#ifndef VM_CHATSMODEL_H
#define VM_CHATSMODEL_H

#include <QAbstractListModel>

#include "VSQCommon.h"

class QSortFilterProxyModel;

namespace vm
{
class ChatsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *proxy MEMBER m_proxy CONSTANT)
    Q_PROPERTY(QString filter MEMBER m_filter WRITE setFilter NOTIFY filterChanged)

public:
    explicit ChatsModel(QObject *parent);
    ~ChatsModel() override;

    void setChats(const Chats &chats);
    Chat createChat(const Contact::Id &contactId);

    void resetUnreadCount(const Chat::Id &chatId);
    void updateLastMessage(const Message &message, const Chat::UnreadCount &unreadMessageCount);
    void updateLastMessageStatus(const Message::Id &messageId, const Message::Status &status);
    void updateLastMessageAttachmentStatus(const Attachment::Id &attachmentId, const Attachment::Status &status);
    void updateLastMessageAttachmentUrl(const Attachment::Id &attachmentId, const QUrl &url);
    void updateLastMessageAttachmentLocalPath(const Attachment::Id &attachmentId, const QString &localPath);
    void updateLastMessageAttachmentExtras(const Attachment::Id &attachmentId, const QVariant &extras);
    void updateLastMessageAttachmentEncryptedSize(const Attachment::Id &attachmentId, const DataSize &encryptedSize);
    void updateLastMessageAttachmentProcessedSize(const Attachment::Id &attachmentId, const DataSize &processedSize);

    Optional<Chat> find(const Chat::Id &chatId) const;
    Optional<Chat> findByContact(const Contact::Id &contactId) const;
    bool hasChat(const Chat::Id &chatId) const;
    bool hasChatWithContact(const Contact::Id &contactId) const;

signals:
    void chatsSet();
    void chatCreated(const Chat &chat);
    void chatUpdated(const Chat &chat);

    void filterChanged(const QString &);

private:
    enum Roles
    {
        IdRole = Qt::UserRole,
        ContactIdRole,
        LastEventTimeRole,
        LastMessageBodyRole,
        UnreadMessagesCountRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setFilter(const QString &filter);

    Optional<int> findRowById(const Chat::Id &chatId) const;
    Optional<int> findRowByLastMessageId(const Message::Id &messageId) const;
    Optional<int> findRowByLastMessageAttachmentId(const Attachment::Id &attachmentId) const;
    Optional<int> findRowByContactId(const Contact::Id &contactId) const;

    void updateLastMessageAttachment(const Attachment::Id &attachmentId, const std::function<bool (Attachment &)> &update);

    Chats m_chats;
    QSortFilterProxyModel *m_proxy;
    QString m_filter;
};
}

#endif // VM_CHATSMODEL_H
