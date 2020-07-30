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

#ifndef VSQ_CHATSMODEL_H
#define VSQ_CHATSMODEL_H

#include <QAbstractListModel>

#include "VSQCommon.h"

class VSQChatsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Columns
    {
        NicknameRole = Qt::UserRole,
        LastMessageBodyRole,
        LastEventTimeRole,
        UnreadMessagesCountRole
    };

    using QAbstractListModel::QAbstractListModel;

    void processMessage(const Message &message);
    void processContact(const QString &contact);
    void updateMessageStatus(const Message &message);

    void setRecipient(const QString &recipient);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    Optional<int> findChatRow(const QString &contact) const;
    void addChat(const QString &contact, const QString &messageBody, const QDateTime &eventTimestamp,
                 const Optional<Message::Status> status);
    void updateChat(const QString &contact, const QString &messageBody, const QDateTime &eventTimestamp,
                    const Optional<Message::Status> status);

    std::vector<Chat> m_chats;
    QString m_recipient;
};

#endif // VSQ_CHATSMODEL_H
