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

#include "ListModel.h"
#include "AttachmentId.h"
#include "Chat.h"
#include "Message.h"

#include <optional>

namespace vm
{
class MessagesModel : public ListModel
{
    Q_OBJECT

public:
    explicit MessagesModel(QObject *parent);
    ~MessagesModel() override = default;

    //
    //  Change current chat.
    //
    void setChat(ChatHandler chat);

    //
    //  Set messages for the current chat.
    //
    void setMessages(ModifiableMessages messages);

    //
    //  Invalidate chat.
    //
    void cleartChat();

    //
    //  Create text message within current chat.
    //
    MessageHandler createTextMessage(const QString &body);

    //
    //  Create message with file attachment within current chat.
    //
    MessageHandler createFileMessage(const QUrl &localFileUrl);

    //
    //  Create message with file attachment within current chat.
    //
    MessageHandler createPictureMessage(const QUrl &localFileUrl);

    //
    // Update message. Returns false if message had the same status.
    //
    bool applyMessageUpdate(const MessageUpdate &messageUpdate);

    //
    //  Return message if found, nullptr otherwise.
    //
    MessageHandler findById(const MessageId &messageId) const;

signals:
    void displayImageNotFound(const MessageId &messageId) const;

private:
    std::unique_ptr<OutgoingMessage> createOutgoingMessage();

    static QVector<int> rolesFromMessageUpdate(const MessageUpdate& messageUpdate);

private:
    enum Roles
    {
        // Common
        IdRole = Qt::UserRole,
        DayRole,
        DisplayTimeRole,
        AuthorIdRole,
        StatusRole,
        StageRole,
        // Text
        BodyRole,
        // Attachment
        AttachmentIdRole,
        AttachmentTypeIsFileRole,
        AttachmentTypeIsPictureRole,
        AttachmentIsLoadingRole,
        AttachmentIconPathRole,
        AttachmentPictureThumbnailSizeRole,
        AttachmentDisplaySizeRole,
        AttachmentDisplayTextRole,
        // Loading
        AttachmentBytesTotalRole,
        AttachmentBytesLoadedRole,
        AttachmentFileExistsRole,
        // ???
        FirstInRowRole,
        InRowRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;


    std::optional<int> findRowById(const MessageId &messageId) const;
    void invalidateRow(const int row, const QVector<int> &roles = {});
    void invalidateModel(const QModelIndex &index, const QVector<int> &roles);

    //
    //  Append a new message to the in-memory collection and update UI.
    //
    MessageHandler appendMessage(std::unique_ptr<Message> message);

private:
    ModifiableMessages m_messages;
    ChatHandler m_currentChat;
};
}

#endif // VM_MESSAGESMODEL_H
