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

#include "models/MessagesModel.h"

#include "Utils.h"

using namespace vm;

MessagesModel::MessagesModel(QObject *parent)
    : QAbstractListModel(parent)
{}

MessagesModel::~MessagesModel()
{}

void MessagesModel::setMessages(const Messages &messages)
{
    beginResetModel();
    m_messages = messages;
    endResetModel();
}

Message MessagesModel::createMessage(const Chat::Id &chatId, const Contact::Id &authorId, const QString &body, const Optional<Attachment> &attachment)
{
    Message message;
    message.id = attachment ? attachment->messageId : Utils::createUuid();
    message.timestamp = QDateTime::currentDateTime();
    message.chatId = chatId;
    message.authorId = authorId;
    message.body = body;
    message.attachment = attachment;
    writeMessage(message);
    return message;
}

void MessagesModel::writeMessage(const Message &message)
{
    const auto count = rowCount();
    beginInsertRows(QModelIndex(), count, count);
    m_messages.push_back(message);
    endInsertRows();
    invalidateRow(count);
}

bool MessagesModel::setMessageStatus(const Message::Id &messageId, const Message::Status &status)
{
    const auto messageRow = findRowById(messageId);
    if (!messageRow) {
        qWarning() << "Message not found! Id" << messageId;
        return true;
    }
    auto &message = m_messages[*messageRow];
    if (message.status == status) {
        return false;
    }
    message.status = status;
    invalidateRow(*messageRow, { StatusRole });
    return true;
}

void MessagesModel::markAllAsRead()
{
    for (int i = 0, s = m_messages.size(); i < s; ++i) {
        auto &message = m_messages[i];
        if (message.status != Message::Status::Read) {
            message.status = Message::Status::Read;
            invalidateRow(i, { StatusRole });
        }
    }
}

int MessagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QVariant MessagesModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto &message = m_messages[row];
    const auto &attachment = message.attachment;

    switch (role) {
    case IdRole:
        return message.id;
    case DayRole:
        return message.timestamp.date();
    case DisplayTimeRole:
        return message.timestamp.toString("• hh:mm");
    case AuthorIdRole:
        return message.authorId;
    case StatusRole:
    {
        const auto nextMessage = (row + 1 == rowCount()) ? nullptr : &m_messages[row + 1];
        if (nextMessage && nextMessage->authorId == message.authorId && nextMessage->status == message.status) {
            return QString();
        }
        return static_cast<int>(message.status);
    }
    case BodyRole:
        return message.body.split('\n').join("<br/>");
    case AttachmentIdRole:
    {
        if (attachment) {
            return attachment->id;
        }
        return QString();
    }
    case AttachmentTypeRole:
    {
        if (attachment) {
            return QVariant::fromValue(attachment->type);
        }
        return QVariant();
    }
    case AttachmentStatusRole:
    {
        if (attachment) {
            return QVariant::fromValue(attachment->status);
        }
        return QVariant();
    }
    case AttachmentImagePathRole:
    {
        if (attachment) {
            if (attachment->type == Attachment::Type::File) {
                return QLatin1String("../resources/icons/File Selected Big.png");
            }
            else if (attachment->type == Attachment::Type::Picture) {
                switch (attachment->status) {
                case Attachment::Status::Loading:
                case Attachment::Status::Postloading:
                    return attachment->extras.value<PictureExtras>().thumbnailPath;
                case Attachment::Status::Loaded:
                    return attachment->extras.value<PictureExtras>().previewPath;
                default:
                    return QString();
                }
            }
        }
        return QString();
    }
    case AttachmentImageSizeRole:
    {
        if (attachment && attachment->type == Attachment::Type::Picture) {
            return attachment->extras.value<PictureExtras>().thumbnailSize;
        }
        return QSize();
    }
    case AttachmentDisplaySizeRole:
    {
        if (attachment) {
            return (attachment->size > 0) ? Utils::formattedDataSize(attachment->size) : QLatin1String("...");
        }
        return QString();
    }
    case AttachmentDisplayTextRole:
    {
        return attachment ? Utils::attachmentDisplayText(*attachment) : QString();
    }
    case AttachmentBytesTotalRole:
    {
        return attachment ? qMax(attachment->size, attachment->bytesTotal) : 0;
    }
    case AttachmentBytesLoadedRole:
    {
        return attachment ? attachment->bytesLoaded : 0;
    }
    case AttachmentFileExistsRole:
    {
        return attachment ? QFile::exists(attachment->localPath) : false;
    }
    case FailedRole:
    {
        if (message.status == Message::Status::Failed) {
            return true;
        }
        if (attachment) {
            const auto &status = attachment->status;
            return status == Attachment::Status::Interrupted || status == Attachment::Status::Invalid;
        }
        return false;
    }
    case FirstInRowRole:
    {
        const auto prevMessage = (row == 0) ? nullptr : &m_messages[row - 1];
        return !prevMessage || prevMessage->authorId != message.authorId || prevMessage->timestamp.addSecs(5 * 60) <= message.timestamp;
    }
    case InRowRole:
    {
        const auto nextMessage = (row + 1 == rowCount()) ? nullptr : &m_messages[row + 1];
        return nextMessage && message.authorId == nextMessage->authorId;
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MessagesModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { DayRole, "day" },
        { DisplayTimeRole, "displayTime" },
        { AuthorIdRole, "authorId" },
        { StatusRole, "status" },
        { BodyRole, "body" },
        { AttachmentIdRole, "attachmentId" },
        { AttachmentTypeRole, "attachmentType" },
        { AttachmentStatusRole, "attachmentStatus" },
        { AttachmentImagePathRole, "attachmentImagePath" },
        { AttachmentImageSizeRole, "attachmentImageSize" },
        { AttachmentDisplaySizeRole, "attachmentDisplaySize" },
        { AttachmentDisplayTextRole, "attachmentDisplayText" },
        { AttachmentBytesTotalRole, "attachmentBytesTotal" },
        { AttachmentBytesLoadedRole, "attachmentBytesLoaded" },
        { AttachmentFileExistsRole, "attachmentFileExists" },
        { FailedRole, "failed" },
        { FirstInRowRole, "firstInRow" },
        { InRowRole, "inRow" },
    };
}

Optional<int> MessagesModel::findRowById(const Message::Id &messageId) const
{
    for (int i = m_messages.size() - 1; i >= 0; --i) {
        if (m_messages[i].id == messageId) {
            return i;
        }
    }
    return NullOptional;
}

void MessagesModel::invalidateRow(const int row, const QVector<int> &roles)
{
    auto allRoles = roles;
    if (allRoles.contains(StatusRole) || allRoles.contains(AttachmentStatusRole)) {
        allRoles << FailedRole;
    }
    if (!allRoles.empty()) {
        const auto rowIndex = index(row);
        emit dataChanged(rowIndex, rowIndex, allRoles);
    }
    if (allRoles.isEmpty() || allRoles.contains(StatusRole)) {
        if (row > 0) {
            const auto rowIndex = index(row - 1);
            emit dataChanged(rowIndex, rowIndex, { StatusRole, InRowRole });
        }
        if (row < rowCount() - 1) {
            const auto rowIndex = index(row + 1);
            emit dataChanged(rowIndex, rowIndex, { FirstInRowRole });
        }
    }
}
