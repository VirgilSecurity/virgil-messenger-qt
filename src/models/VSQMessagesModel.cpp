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

#include "models/VSQMessagesModel.h"

#include "VSQUtils.h"

Q_LOGGING_CATEGORY(lcMessagesModel, "messagesModel")

void VSQMessagesModel::addMessage(const Message &message)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_messages.push_back(message);
    endInsertRows();
    emit messageAdded(message);
}

void VSQMessagesModel::setMessageStatus(const Message &message, const Message::Status status)
{
    setMessageStatusById(message.id, status);
}

void VSQMessagesModel::setMessageStatusById(const QString &messageId, const Message::Status status)
{
    const auto row = findMessageRow(messageId);
    if (!row)
        qCCritical(lcMessagesModel) << "Message status can't be changed, message doesn't exist";
    else
        setMessageStatusByRow(*row, status);
}

void VSQMessagesModel::setUploadProgress(const Message &message, DataSize uploaded)
{
    const auto messageRow = findMessageRow(message.id);
    if (!messageRow) {
        qCCritical(lcMessagesModel) << "Upload progress can't be changed, message doesn't exist";
        return;
    }
    const int row = *messageRow;
    auto &msg = m_messages[row];
    if (!msg.attachment) {
        qCCritical(lcMessagesModel) << "Upload progress can't be set, attachment is empty";
        return;
    }
    if (msg.attachment->uploaded == uploaded)
        return;
    msg.attachment->uploaded = uploaded;
    emit dataChanged(index(row), index(row), { AttachmentUploadedRole });
}

void VSQMessagesModel::setUploadFailed(const Message &message, bool failed)
{
    const auto messageRow = findMessageRow(message.id);
    if (!messageRow) {
        qCCritical(lcMessagesModel) << "Upload failed flag can't be changed, message doesn't exist";
        return;
    }
    const int row = *messageRow;
    auto &msg = m_messages[row];
    if (!msg.attachment) {
        qCCritical(lcMessagesModel) << "Upload failed flag can't be set, attachment is empty";
        return;
    }
    if (msg.attachment->loadingFailed == failed)
        return;
    msg.attachment->loadingFailed = failed;
    emit dataChanged(index(row), index(row), { AttachmentLoadingFailedRole });
}

void VSQMessagesModel::setUser(const QString &user)
{
    if (m_user == user)
        return;
    m_user = user;

    beginResetModel();
    m_messages.clear();
    endResetModel();
}

void VSQMessagesModel::setRecipient(const QString &recipient)
{
    // TODO(fpohtmeh): load from database
    // Marks all unread messages as read
    int row = -1;
    for (auto &message : m_messages) {
        ++row;
        if (message.contact != recipient)
            continue;
        if (message.author == Message::Author::User)
            continue;
        setMessageStatusByRow(row, Message::Status::Read);
    }
}

int VSQMessagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QHash<int, QByteArray> VSQMessagesModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[BodyRole] = "body";
    names[TimeRole] = "time";
    names[NicknameRole] = "nickname";
    names[IsUserRoles] = "isUser";
    names[StatusRole] = "status";
    names[FailedRole] = "failed";
    names[InRowRole] = "inRow";
    names[FirstInRowRole] = "firstInRow";
    names[AttachmentIdRole] = "attachmentId";
    names[AttachmentSizeRole] = "attachmentSize";
    names[AttachmentDisplaySizeRole] = "attachmentDisplaySize";
    names[AttachmentTypeRole] = "attachmentType";
    names[AttachmentLocalUrlRole] = "attachmentLocalUrl";
    names[AttachmentLocalPreviewRole] = "attachmentLocalPreview";
    names[AttachmentUploadedRole] = "attachmentUploaded";
    names[AttachmentLoadingFailedRole] = "attachmentLoadingFailed";
    return names;
}

QVariant VSQMessagesModel::data(const QModelIndex &index, int role) const
{
    const auto &message = m_messages[index.row()];
    switch (role) {
    case BodyRole:
        return message.body.split("\n").join("<br/>");
    case TimeRole:
        return message.timestamp.toString(QLatin1String("hh:mm"));
    case NicknameRole:
        return (message.author == Message::Author::User) ? m_user : message.contact;
    case IsUserRoles:
        return message.author == Message::Author::User;
    case StatusRole:
        return displayStatus(message.status);
    case FailedRole:
        return message.status == Message::Status::Failed;
    case InRowRole:
        return isInRow(message, index.row());
    case FirstInRowRole:
        return isFirstInRow(message, index.row());
    case AttachmentIdRole:
        return message.attachment ? message.attachment->id : QString();
    case AttachmentSizeRole:
        return message.attachment ? message.attachment->size : 0;
    case AttachmentDisplaySizeRole:
        return message.attachment ? VSQUtils::formattedDataSize(message.attachment->size) : QString();
    case AttachmentTypeRole:
        return QVariant::fromValue(message.attachment ? message.attachment->type : Attachment::Type::File);
    case AttachmentLocalUrlRole:
        return message.attachment ? message.attachment->local_url : QString();
    case AttachmentLocalPreviewRole:
        return message.attachment ? message.attachment->local_preview : QString();
    case AttachmentUploadedRole:
        return message.attachment ? message.attachment->uploaded : 0;
    case AttachmentLoadingFailedRole:
        return message.attachment ? message.attachment->loadingFailed : 0;
    default:
        return QVariant();
    }
}

Optional<int> VSQMessagesModel::findMessageRow(const QString &id) const
{
    // TODO(fpohtmeh): add caching
    for (int i = m_messages.size() - 1; i >= 0; --i)
        if (m_messages[i].id == id)
            return i;
    return NullOptional;
}

void VSQMessagesModel::setMessageStatusByRow(int row, const Message::Status status)
{
    auto &message = m_messages[row];
    if (message.status == status)
        return;
    message.status = status;
    emit dataChanged(index(row), index(row), { StatusRole });
    emit messageStatusChanged(message);
}

QString VSQMessagesModel::displayStatus(const Message::Status status) const
{
    switch (status) {
    case Message::Status::Created:
        return tr("Created");
    case Message::Status::Sent:
        return tr("Sent");
    case Message::Status::Received:
        return tr("Received");
    case Message::Status::Read:
        return tr("Read");
    case Message::Status::Failed:
        return tr("Failed");
    default:
        qCritical() << "Invalid message status";
        return QLatin1String();
    }
}

bool VSQMessagesModel::isInRow(const Message &message, int row) const
{
    if (row == 0)
        return false;
    return m_messages[row - 1].author == message.author;
}

bool VSQMessagesModel::isFirstInRow(const Message &message, int row) const
{
    if (row == 0)
        return true;
    const Message &prevMessage = m_messages[row - 1];
    // Message is considered to be the first in a row when it
    // sends in a range of 5 min with previous message and from the same author
    return prevMessage.author == message.author || prevMessage.timestamp.addSecs(5 * 60) <= message.timestamp;
}
