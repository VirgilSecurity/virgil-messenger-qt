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

#include "AttachmentId.h"
#include "Chat.h"
#include "GroupUpdate.h"
#include "ListModel.h"
#include "Message.h"

namespace vm {
class MessagesProxyModel;
class Messenger;

class MessagesModel : public ListModel
{
    Q_OBJECT

public:
    enum Roles {
        // Common
        IdRole = Qt::UserRole,
        DayRole,
        DisplayTimeRole,
        SenderIdRole,
        SenderUsernameRole,
        StatusIconRole,
        IsBrokenRole,
        // Text
        BodyRole,
        // Attachment
        AttachmentIdRole,
        AttachmentTypeIsFileRole,
        AttachmentTypeIsPictureRole,
        AttachmentIsLoadingRole,
        AttachmentIsLoadedRole,
        AttachmentIconPathRole,
        AttachmentPictureThumbnailSizeRole,
        AttachmentDisplaySizeRole,
        AttachmentDisplayProgressRole,
        AttachmentDisplayTextRole,
        // Loading
        AttachmentBytesTotalRole,
        AttachmentBytesLoadedRole,
        AttachmentFileExistsRole,
        // Grouping
        FirstInRowRole,
        InRowRole,
        FirstInSectionRole,
        // Sorting
        SortRole
    };

    MessagesModel(Messenger *messenger, QObject *parent);
    ~MessagesModel() override = default;

    //
    //  Current chat.
    //
    ChatHandler chat() const;

    //
    //  Change current chat.
    //
    void setChat(ChatHandler chat);

    //
    //  Set messages for the current chat.
    //
    void setMessages(ModifiableMessages messages);

    //
    //  Add message to the current chat if ids match, otherwise - ignore.
    //
    void addMessage(ModifiableMessageHandler message);

    //
    //  Get message by row.
    //
    MessageHandler getMessage(int row) const;

    //
    //  Invalidate chat.
    //
    void clearChat();

    //
    //  Clear messages
    //
    void clearMessages();

    //
    // Update message. Returns false if message had the same status.
    //
    bool updateMessage(const MessageUpdate &messageUpdate);

    //
    // Update group
    //
    void updateGroup(const GroupUpdate &groupUpdate);

    //
    //  Return message if found, nullptr otherwise.
    //
    ModifiableMessageHandler findById(const MessageId &messageId) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QString lastMessageSenderId() const; // TODO(fpohtmeh): remove
    MessageHandler findIncomingInvitationMessage() const;

signals:
    void pictureIconNotFound(const MessageId &messageId) const;
    void messageAdding(); // TODO(fpohtmeh): remove
    void messagesReset();
    void groupInvitationReceived(const QString &ownerUsername, const MessageHandler &message);

private:
    static QVector<int> rolesFromMessageUpdate(const MessageUpdate &messageUpdate);
    static QString statusIconPath(MessageHandler message);

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QModelIndex findIndexById(const MessageId &messageId) const;
    MessageHandler getNeighbourMessage(int row, int offset) const;
    void invalidateRow(const int row, const QVector<int> &roles = {});
    void invalidateModel(const QModelIndex &index, const QVector<int> &roles);

private:
    QPointer<Messenger> m_messenger;
    QPointer<MessagesProxyModel> m_proxy;
    ModifiableMessages m_messages;
    ChatHandler m_currentChat;
};
} // namespace vm

#endif // VM_MESSAGESMODEL_H
