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
#include "FileUtils.h"
#include "Model.h"

#include <algorithm>

using namespace vm;
using Self = MessagesModel;


Self::MessagesModel(QObject *parent)
    : ListModel(parent)
{
    qRegisterMetaType<MessagesModel *>("MessagesModel*");
}


ChatHandler Self::chat() const {
    return m_currentChat;
}


void Self::setChat(ChatHandler chat) {
    qCDebug(lcModel) << "Set chat to the messages model: " << chat->id();
    m_currentChat = std::move(chat);
}


void Self::setMessages(ModifiableMessages messages)
{
    qCDebug(lcModel) << "Set messages to the messages model";
    beginResetModel();
    m_messages = std::move(messages);
    endResetModel();
}

void Self::addMessage(ModifiableMessageHandler message) {
    if (m_currentChat && (m_currentChat->id() == message->chatId())) {
        const auto count = rowCount();
        beginInsertRows(QModelIndex(), count, count);
        m_messages.emplace_back(std::move(message));
        endInsertRows();
        invalidateRow(count);
    }
}


void Self::clearChat() {
    qCDebug(lcModel) << "Clear all messages";
    beginResetModel();
    m_messages.clear();
    m_currentChat = nullptr;
    endResetModel();
}


bool Self::updateMessage(const MessageUpdate &messageUpdate) {
    auto messageId = MessageUpdateGetMessageId(messageUpdate);

    const auto messageRow = findRowById(messageId);
    if (!messageRow) {
        qCWarning(lcModel) << "Message not found! Id" << messageId;
        return false;
    }

    auto &message = m_messages[*messageRow];

    if (!message->applyUpdate(messageUpdate)) {
        auto roles = rolesFromMessageUpdate(messageUpdate);
        invalidateRow(*messageRow, roles);
        return true;
    }

    return false;
}


MessageHandler Self::findById(const MessageId &messageId) const
{
    auto messageIt = std::find_if(std::rbegin(m_messages), std::rend(m_messages), [&messageId](auto message) {
        return message->id() == messageId;
    });

    if (messageIt != std::rend(m_messages)) {
        return *messageIt;
    }

    return nullptr;
}

int Self::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto message = m_messages[row];
    const auto attachment = message->contentIsAttachment() ? message->contentAsAttachment() : nullptr;

    switch (role) {
    case IdRole:
        return QString(message->id());

    case DayRole:
        return message->createdAt().date();

    case DisplayTimeRole:
        return message->createdAt().toString("• hh:mm");

    case AuthorIdRole:
        return QString(message->senderId());

    case StatusRole: {
        const auto nextMessage = (row + 1 == rowCount()) ? nullptr : m_messages[row + 1];
        if (nextMessage && nextMessage->senderId() == message->senderId() &&
                    nextMessage->status() == message->status()) {
            return QString();
        }
        return message->statusString();
    }

    case BodyRole: {
        if (auto textContent = std::get_if<MessageContentText>(&message->content())) {
            return textContent->text().split('\n').join("<br/>");
        }
        return QString();
    }

    case AttachmentIdRole: {
        if (attachment) {
            return QString(attachment->id());
        }
        return QString();
    }

    case AttachmentTypeIsFileRole: {
        return std::holds_alternative<MessageContentFile>(message->content());
    }

    case AttachmentTypeIsPictureRole: {
        return std::holds_alternative<MessageContentPicture>(message->content());
    }

    case AttachmentIsLoadingRole: {
        if (!attachment) {
            return false;
        }

        const bool isUploading = message->isOutgoing() && (attachment->uploadStage() != MessageContentUploadStage::Uploaded);
        const bool isDownloading = attachment->downloadStage() != MessageContentDownloadStage::Decrypted;

        return isUploading || isDownloading;
    }

    case AttachmentIsLoadedRole: {
        if (!attachment) {
            return false;
        }

        const bool isUploaded = message->isOutgoing() && (attachment->uploadStage() == MessageContentUploadStage::Uploaded);
        const bool isDownloaded = attachment->downloadStage() == MessageContentDownloadStage::Decrypted;

        return isUploaded || isDownloaded;
    }

    case AttachmentIconPathRole: {
        if (std::holds_alternative<MessageContentFile>(message->content())) {
            return QLatin1String("../resources/icons/File Selected Big.png");

        } else if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            const auto imagePath = picture->previewOrThumbnailPath();
            if (!imagePath.isEmpty()) {
                return FileUtils::localFileToUrl(imagePath);
            }
            if (message->status() != Message::Status::New) {
                qCDebug(lcModel) << "Requesting of missing thumbnail/preview";
                emit displayImageNotFound(message->id());
            }
        }
        return QString();
    }

    case AttachmentPictureThumbnailSizeRole: {
        if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            return picture->thumbnailSize();
        }

        return QSize();
    }

    case AttachmentDisplaySizeRole: {
        if (attachment) {
            return (attachment->size() > 0) ? Utils::formattedSize(attachment->size()) : QLatin1String("...");
        }
        return QString();
    }

    case AttachmentDisplayProgressRole: {
        if (attachment) {
            return Utils::formattedDataSizeProgress(attachment->processedSize(), attachment->encryptedSize());
        }
        return QString();
    }

    case AttachmentDisplayTextRole: {
        return attachment ? Utils::messageContentDisplayText(message->content()) : QString();
    }

    case AttachmentBytesTotalRole: {
        if (!attachment) {
            return 0;
        }

        if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            return picture->encryptedSize() + picture->thumbnail().encryptedSize();
        }

        return attachment->encryptedSize();
    }

    case AttachmentBytesLoadedRole: {
        return attachment ? attachment->processedSize() : 0;
    }

    case AttachmentFileExistsRole: {
        return attachment ? FileUtils::fileExists(attachment->localPath()) : false;
    }

    case FirstInRowRole: {
        const auto prevMessage = (row == 0) ? nullptr : m_messages[row - 1];
        return !prevMessage || prevMessage->senderId() != message->senderId() ||
                prevMessage->createdAt().addSecs(5 * 60) <= message->createdAt();
    }

    case InRowRole: {
        const auto nextMessage = (row + 1 == rowCount()) ? nullptr : m_messages[row + 1];
        return nextMessage && message->senderId() == nextMessage->senderId();
    }

    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Self::roleNames() const
{
    return {
        { IdRole, "id" },
        { DayRole, "day" },
        { DisplayTimeRole, "displayTime" },
        { AuthorIdRole, "authorId" },
        { StatusRole, "status" },
        { BodyRole, "body" },
        { AttachmentIdRole, "attachmentId" },
        { AttachmentTypeIsFileRole, "attachmentTypeIsFile" },
        { AttachmentTypeIsPictureRole, "attachmentTypeIsPicture" },
        { AttachmentIsLoadingRole, "attachmentIsLoading" },
        { AttachmentIsLoadedRole, "attachmentIsLoaded" },
        { AttachmentIconPathRole, "attachmentIconPath" },
        { AttachmentPictureThumbnailSizeRole, "attachmentPictureThumbnailSize" },
        { AttachmentDisplaySizeRole, "attachmentDisplaySize" },
        { AttachmentDisplayProgressRole, "attachmentDisplayProgress" },
        { AttachmentDisplayTextRole, "attachmentDisplayText" },
        { AttachmentBytesTotalRole, "attachmentBytesTotal" },
        { AttachmentBytesLoadedRole, "attachmentBytesLoaded" },
        { AttachmentFileExistsRole, "attachmentFileExists" },
        { FirstInRowRole, "firstInRow" },
        { InRowRole, "inRow" },
    };
}


std::optional<int> Self::findRowById(const MessageId &messageId) const
{
    auto messageIt = std::find_if(std::rbegin(m_messages), std::rend(m_messages), [&messageId](auto message) {
        return message->id() == messageId;
    });

    if (messageIt != std::rend(m_messages)) {
        return std::distance(std::begin(m_messages), messageIt.base()) - 1;
    }

    return {};
}

void Self::invalidateRow(const int row, const QVector<int> &roles)
{
    auto allRoles = roles;

    if (!allRoles.empty()) {
        invalidateModel(index(row), allRoles);
    }

    if (allRoles.contains(StatusRole)) {
        if (row > 0) {
            invalidateModel(index(row - 1), { StatusRole, InRowRole });
        }
        if (row < rowCount() - 1) {
            invalidateModel(index(row + 1), { FirstInRowRole });
        }
    }
}

void Self::invalidateModel(const QModelIndex &index, const QVector<int> &roles)
{
    emit dataChanged(index, index, roles);
}

QVector<int> Self::rolesFromMessageUpdate(const MessageUpdate& messageUpdate) {

    if(std::holds_alternative<MessageStatusUpdate>(messageUpdate)) {
        return { StatusRole };

    } else if(std::holds_alternative<IncomingMessageStageUpdate>(messageUpdate)) {
        return { StageRole };

    } else if(std::holds_alternative<OutgoingMessageStageUpdate>(messageUpdate)) {
        return { StatusRole };

    } else if(std::holds_alternative<MessageAttachmentUploadStageUpdate>(messageUpdate)) {
        return { AttachmentIsLoadingRole };

    } else if(std::holds_alternative<MessageAttachmentDownloadStageUpdate>(messageUpdate)) {
        return { AttachmentIsLoadingRole };

    } else if(std::holds_alternative<MessageAttachmentFingerprintUpdate>(messageUpdate)) {
        return {  };

    } else if(std::holds_alternative<MessageAttachmentSizeUpdate>(messageUpdate)) {
        return { AttachmentPictureThumbnailSizeRole };

    } else if(std::holds_alternative<MessageAttachmentRemoteUrlUpdate>(messageUpdate)) {
        return {  };

    } else if(std::holds_alternative<MessageAttachmentLocalPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else if(std::holds_alternative<MessageAttachmentEncryptedSizeUpdate>(messageUpdate)) {
        return {  };

    } else if(std::holds_alternative<MessageAttachmentProcessedSizeUpdate>(messageUpdate)) {
        return { AttachmentBytesLoadedRole };

    } else {
        return { };
    }
}

