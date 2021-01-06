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
#include "models/ListProxyModel.h"

#include <algorithm>

using namespace vm;
using Self = MessagesModel;


Self::MessagesModel(QObject *parent)
    : ListModel(parent)
{
    qRegisterMetaType<MessagesModel *>("MessagesModel*");

    proxy()->setSortRole(SortRole);
    proxy()->sort(0, Qt::DescendingOrder);
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
        emit messageAdding();
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


bool Self::updateMessage(const MessageUpdate &messageUpdate, const bool apply) {
    auto messageId = MessageUpdateGetMessageId(messageUpdate);

    const auto messageRow = findRowById(messageId);
    if (!messageRow) {
        return false;
    }

    const auto row = *messageRow;
    if (apply) {
        m_messages[row]->applyUpdate(messageUpdate);
        // FIXME(fpohtmeh): merge logic with MessageOperation logic
    }

    const auto roles = rolesFromMessageUpdate(messageUpdate);
    invalidateRow(row, roles);
    return true;
}


ModifiableMessageHandler Self::findById(const MessageId &messageId) const
{
    auto messageIt = std::find_if(std::rbegin(m_messages), std::rend(m_messages), [&messageId](auto message) {
        return message->id() == messageId;
    });

    if (messageIt != std::rend(m_messages)) {
        return *messageIt;
    }

    return nullptr;
}

QString MessagesModel::lastMessageSenderId() const
{
    return m_messages.empty() ? QString() : m_messages.back()->senderId();
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

    case SenderIdRole:
        return QString(message->senderId());

    case SenderUsernameRole:
        return QString(message->senderUsername());

    case StatusIconRole:
        return statusIconPath(message);

    case IsBrokenRole:
        return message->status() == Message::Status::Broken;

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
        if (!attachment || message->status() == MessageStatus::Broken) {
            return false;
        }
        if (message->isOutgoing() && (attachment->uploadStage() != MessageContentUploadStage::Uploaded)) {
            return true; // uploading
        }
        const bool isDownloading =
                attachment->downloadStage() == MessageContentDownloadStage::Preloading ||
                attachment->downloadStage() == MessageContentDownloadStage::Downloading ||
                attachment->downloadStage() == MessageContentDownloadStage::Downloaded;
        return isDownloading;
    }

    case AttachmentIsLoadedRole: {
        if (!attachment || message->status() == MessageStatus::Broken) {
            return false;
        }
        if (message->isOutgoing() && (attachment->uploadStage() == MessageContentUploadStage::Uploaded)) {
            return true; // uploaded
        }
        return attachment->downloadStage() == MessageContentDownloadStage::Decrypted;
    }

    case AttachmentIconPathRole: {
        if (std::holds_alternative<MessageContentFile>(message->content())) {
            return QLatin1String("../resources/icons/File Selected Big.png");

        } else if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            const auto imagePath = picture->previewOrThumbnailPath();
            if (!imagePath.isEmpty()) {
                return FileUtils::localFileToUrl(imagePath);
            }
            if (message->status() != MessageStatus::New && message->status() != MessageStatus::Processing) {
                qCDebug(lcModel) << "Requesting of missing thumbnail/preview";
                emit pictureIconNotFound(message->id());
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
        if (!attachment) {
            return QString();
        }

        auto totalSize = attachment->encryptedSize();
        if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            totalSize += picture->thumbnail().encryptedSize();
        }
        return Utils::formattedDataSizeProgress(attachment->processedSize(), totalSize);
    }

    case AttachmentDisplayTextRole: {
        return attachment ? Utils::messageContentDisplayText(message->content()) : QString();
    }

    case AttachmentBytesTotalRole: {
        if (!attachment) {
            return 0;
        }

        auto totalSize = attachment->encryptedSize();
        if (auto picture = std::get_if<MessageContentPicture>(&message->content())) {
            totalSize += picture->thumbnail().encryptedSize();
        }
        return totalSize;
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

    case FirstInSectionRole: {
        const auto prevMessage = (row == 0) ? nullptr : m_messages[row - 1];
        return !prevMessage || prevMessage->createdAt().date() != message->createdAt().date();
    }

    case SortRole: {
        return message->createdAt();
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
        { SenderIdRole, "senderId" },
        { SenderUsernameRole, "senderUsername" },
        { StatusIconRole, "statusIcon" },
        { IsBrokenRole, "isBroken" },
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
        { FirstInSectionRole, "firstInSection" }
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
    // Invalidate row
    invalidateModel(index(row), roles);

    // Invalidate neighbour rows
    if (roles.isEmpty() || roles.contains(StatusIconRole)) {
        if (row > 0) {
            invalidateModel(index(row - 1), { StatusIconRole, IsBrokenRole, InRowRole });
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

    if(std::holds_alternative<IncomingMessageStageUpdate>(messageUpdate) ||
            std::holds_alternative<OutgoingMessageStageUpdate>(messageUpdate)) {
        return { StatusIconRole, IsBrokenRole, InRowRole };

    } else if(std::holds_alternative<MessageAttachmentUploadStageUpdate>(messageUpdate) ||
              std::holds_alternative<MessageAttachmentDownloadStageUpdate>(messageUpdate)) {
        return { AttachmentIsLoadingRole, AttachmentIsLoadedRole, AttachmentFileExistsRole };

    } else if(std::holds_alternative<MessageAttachmentLocalPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else if(std::holds_alternative<MessageAttachmentEncryptionUpdate>(messageUpdate)) {
        return { AttachmentBytesTotalRole, AttachmentDisplayProgressRole };

    } else if(std::holds_alternative<MessageAttachmentProcessedSizeUpdate>(messageUpdate)) {
        return { AttachmentBytesLoadedRole, AttachmentDisplayProgressRole };

    } else if(std::holds_alternative<MessagePictureThumbnailPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else if(std::holds_alternative<MessagePictureThumbnailSizeUpdate>(messageUpdate)) {
        return { AttachmentPictureThumbnailSizeRole };

    } else if(std::holds_alternative<MessagePicturePreviewPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else {
        return { };
    }
}

QString Self::statusIconPath(MessageHandler message)
{
    const QString path("../resources/icons/%1.png");

    switch (message->status()) {
        case Message::Status::New:
        case Message::Status::Processing:
            return path.arg("M-Sending");
        case Message::Status::Succeed:
            if (message->isOutgoing()) {
                // TODO(fpohtmeh): implement smarter check?
                if (message->stageString() == OutgoingMessageStageToString(OutgoingMessageStage::Delivered)) {
                    return path.arg("M-Delivered");
                }
                else if (message->stageString() == OutgoingMessageStageToString(OutgoingMessageStage::Read)) {
                    return path.arg("M-Read");
                }
            }
            return path.arg("M-Sent");
        case Message::Status::Failed:
            return path.arg("M-Sending");
        case Message::Status::Broken:
            return path.arg("M-Error");
        default:
            return QString();
    }
}
