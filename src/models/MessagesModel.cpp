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

#include "MessagesModel.h"

#include "FileUtils.h"
#include "MessagesProxyModel.h"
#include "Messenger.h"
#include "Model.h"
#include "Utils.h"

#include <algorithm>

using namespace vm;
using Self = MessagesModel;

Self::MessagesModel(Messenger *messenger, QObject *parent) : ListModel(parent, false), m_messenger(messenger)
{
    qRegisterMetaType<MessagesModel *>("MessagesModel*");
    m_proxy = new MessagesProxyModel(messenger, this);
    setProxy(m_proxy);
}

ChatHandler Self::chat() const
{
    return m_currentChat;
}

void Self::setChat(ChatHandler chat)
{
    qCDebug(lcModel) << "Set chat to the messages model: " << chat->id();
    m_currentChat = std::move(chat);
}

void Self::addMessages(ModifiableMessages messages)
{
    qCDebug(lcModel) << "Add messages to the messages model. Count" << messages.size();
    if (messages.empty()) {
        return;
    }

    const auto count = rowCount();
    beginInsertRows(QModelIndex(), count, count + messages.size() - 1);
    m_messages.insert(m_messages.end(), messages.begin(), messages.end());
    endInsertRows();

    if (auto message = findIncomingInvitationMessage()) {
        const auto displayUsername = Utils::displayUsername(message->senderUsername(), message->senderId());
        emit groupInvitationReceived(displayUsername, message);
    }
}

void Self::addMessage(ModifiableMessageHandler message)
{
    if (m_currentChat && (m_currentChat->id() == message->chatId()) && !findById(message->id())) {
        emit messageAdding();
        const auto count = rowCount();
        beginInsertRows(QModelIndex(), count, count);
        m_messages.push_back(std::move(message));
        endInsertRows();
        invalidateRow(count);
    }
}

MessageHandler Self::getMessage(const int row) const
{
    return m_messages.at(row);
}

void Self::clearChat()
{
    qCDebug(lcModel) << "Clear all messages";
    beginResetModel();
    m_messages.clear();
    m_currentChat = nullptr;
    endResetModel();
}

bool Self::updateMessage(const MessageUpdate &messageUpdate)
{
    const auto messageIndex = findIndexById(MessageUpdateGetMessageId(messageUpdate));
    if (!messageIndex.isValid()) {
        return false;
    }

    m_messages[messageIndex.row()]->applyUpdate(messageUpdate);

    const auto roles = rolesFromMessageUpdate(messageUpdate);
    invalidateRow(messageIndex.row(), roles);
    return true;
}

void Self::updateGroup(const GroupUpdate &groupUpdate)
{
    if (const auto upd = std::get_if<GroupInvitationUpdate>(&groupUpdate)) {
        if (findIncomingInvitationMessage()) {
            m_currentChat->group()->setInvitationStatus(upd->invitationStatus);
            m_proxy->invalidate();
        }
    }
}

ModifiableMessageHandler Self::findById(const MessageId &messageId) const
{
    const auto messageIt = std::find_if(std::rbegin(m_messages), std::rend(m_messages),
                                        [&messageId](auto message) { return message->id() == messageId; });
    if (messageIt != std::rend(m_messages)) {
        return *messageIt;
    }
    return nullptr;
}

QString MessagesModel::lastMessageSenderId() const
{
    const auto messageIndex = m_proxy->getLastIndex();
    return messageIndex.isValid() ? getMessage(messageIndex.row())->senderId() : QString();
}

int Self::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.size();
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    const auto message = m_messages.at(row);
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
        QString text;
        if (auto textContent = std::get_if<MessageContentText>(&message->content())) {
            text = textContent->text();
        } else if (auto groupInvitation = std::get_if<MessageContentGroupInvitation>(&message->content())) {
            if (groupInvitation->superOwnerId() == m_messenger->currentUser()->id()) {
                text = tr("Invitation sent to %1")
                               .arg(Utils::displayUsername(message->recipientUsername(), message->recipientId()));
            } else {
                text = tr("Invited by %1").arg(Utils::displayUsername(message->senderUsername(), message->senderId()));
            }
        }
        return text.split('\n').join("<br/>");
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
        const bool isDownloading = attachment->downloadStage() == MessageContentDownloadStage::Preloading
                || attachment->downloadStage() == MessageContentDownloadStage::Downloading
                || attachment->downloadStage() == MessageContentDownloadStage::Downloaded;
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
        return attachment && FileUtils::fileExists(attachment->localPath());
    }

    case FirstInRowRole: {
        const auto prevMessage = getNeighbourMessage(row, -1);
        return !prevMessage || prevMessage->senderId() != message->senderId()
                || prevMessage->createdAt().addSecs(5 * 60) <= message->createdAt();
    }

    case InRowRole: {
        const auto nextMessage = getNeighbourMessage(row, 1);
        return nextMessage && message->senderId() == nextMessage->senderId();
    }

    case FirstInSectionRole: {
        const auto prevMessage = getNeighbourMessage(row, -1);
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
    return { { IdRole, "id" },
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
             { FirstInSectionRole, "firstInSection" } };
}

QModelIndex Self::findIndexById(const MessageId &messageId) const
{
    auto it = std::find_if(std::rbegin(m_messages), std::rend(m_messages),
                           [&messageId](auto message) { return message->id() == messageId; });
    if (it != std::rend(m_messages)) {
        return index(std::distance(std::begin(m_messages), it.base()) - 1);
    }
    return QModelIndex();
}

MessageHandler Self::getNeighbourMessage(int row, int offset) const
{
    if (const auto index = m_proxy->getNeighbourIndex(row, offset); index.isValid()) {
        return getMessage(index.row());
    }
    return MessageHandler();
}

void Self::invalidateRow(const int row, const QVector<int> &roles)
{
    // Invalidate row
    invalidateModel(index(row), roles);

    // Invalidate neighbour rows
    if (roles.isEmpty() || roles.contains(StatusIconRole)) {
        if (const auto prevIndex = m_proxy->getNeighbourIndex(row, -1); prevIndex.isValid()) {
            invalidateModel(prevIndex, { StatusIconRole, IsBrokenRole, InRowRole });
        }
        if (const auto nextIndex = m_proxy->getNeighbourIndex(row, 1); nextIndex.isValid()) {
            invalidateModel(nextIndex, { FirstInRowRole });
        }
    }
}

void Self::invalidateModel(const QModelIndex &index, const QVector<int> &roles)
{
    emit dataChanged(index, index, roles);
}

MessageHandler Self::findIncomingInvitationMessage() const
{
    if (!chat() || !chat()->group() || chat()->group()->invitationStatus() != GroupInvitationStatus::Invited) {
        return nullptr;
    }
    if (m_messages.empty()) {
        return nullptr;
    }
    const auto message = getMessage(0);
    if (!message->isIncoming()) {
        return nullptr;
    }
    return std::holds_alternative<MessageContentGroupInvitation>(message->content()) ? message : nullptr;
}

QVector<int> Self::rolesFromMessageUpdate(const MessageUpdate &messageUpdate)
{
    if (std::holds_alternative<IncomingMessageStageUpdate>(messageUpdate)
        || std::holds_alternative<OutgoingMessageStageUpdate>(messageUpdate)) {
        return { AttachmentIsLoadingRole, StatusIconRole, IsBrokenRole, InRowRole };

    } else if (std::holds_alternative<MessageAttachmentUploadStageUpdate>(messageUpdate)
               || std::holds_alternative<MessageAttachmentDownloadStageUpdate>(messageUpdate)) {
        return { AttachmentIsLoadingRole, AttachmentIsLoadedRole, AttachmentFileExistsRole };

    } else if (std::holds_alternative<MessageAttachmentLocalPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else if (std::holds_alternative<MessageAttachmentEncryptionUpdate>(messageUpdate)) {
        return { AttachmentBytesTotalRole, AttachmentDisplayProgressRole };

    } else if (std::holds_alternative<MessageAttachmentProcessedSizeUpdate>(messageUpdate)) {
        return { AttachmentBytesLoadedRole, AttachmentDisplayProgressRole };

    } else if (std::holds_alternative<MessagePictureThumbnailPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else if (std::holds_alternative<MessagePicturePreviewPathUpdate>(messageUpdate)) {
        return { AttachmentIconPathRole };

    } else {
        return {};
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
                return path.arg("M-Delivered"); // TODO(fpohtmeh): should be "M-Delivered"
            } else if (message->stageString() == OutgoingMessageStageToString(OutgoingMessageStage::Read)) {
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
