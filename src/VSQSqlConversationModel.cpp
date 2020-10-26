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

#include "VSQSqlConversationModel.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include "VSQCryptoTransferManager.h"
#include "Utils.h"

Q_DECLARE_METATYPE(StMessage::Status)

/******************************************************************************/
void
VSQSqlConversationModel::_createTable() {
    QSqlQuery query;
    if (!query.exec(
        QString("CREATE TABLE IF NOT EXISTS %1 ("
        "'author' TEXT NOT NULL,"
        "'recipient' TEXT NOT NULL,"
        "'timestamp' TEXT NOT NULL,"
        "'message' TEXT NOT NULL,"
        "'status' int NOT NULL,"
        "'message_id' TEXT NOT NULL,"
        ""
        "attachment_id TEXT,"
        "attachment_bytes_total INTEGER,"
        "attachment_type INTEGER,"
        "attachment_file_path TEXT,"
        "attachment_file_name TEXT,"
        "attachment_remote_url TEXT,"
        "attachment_thumbnail_path TEXT,"
        "attachment_thumbnail_width INTEGER,"
        "attachment_thumbnail_height INTEGER,"
        "attachment_remote_thumbnail_url TEXT,"
        "attachment_status INT,"
        ""
        "FOREIGN KEY('author') REFERENCES %2 ( name ),"
        "FOREIGN KEY('recipient') REFERENCES %3 ( name )"
        ")").arg(_tableName())
            .arg(_contactsTableName())
            .arg(_contactsTableName()))) {
        qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
    }

    QSqlQuery indexQuery;
    if (!indexQuery.exec(
        QString("CREATE UNIQUE INDEX IF NOT EXISTS idx_%1_message_id ON %1 (message_id);").arg(_tableName()))) {
        qFatal("Failed to query database: %s", qPrintable(indexQuery.lastError().text()));
    }
}

/******************************************************************************/
void
VSQSqlConversationModel::_update() {
    setTable(_tableName());
    setSort(2, Qt::DescendingOrder);
    // Ensures that the model is sorted correctly after submitting a new row.
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    setFilter("(recipient = '')");
    select();

    emit recipientChanged();
}

/******************************************************************************/
VSQSqlConversationModel::VSQSqlConversationModel(vm::Validator *validator, QObject *parent)
    : QSqlTableModel(parent)
    , m_validator(validator)
{
    qRegisterMetaType<StMessage::Status>("StMessage::Status");

    connect(this, &VSQSqlConversationModel::createMessage, this, &VSQSqlConversationModel::onCreateMessage);
    connect(this, &VSQSqlConversationModel::receiveMessage, this, &VSQSqlConversationModel::onReceiveMessage);
    connect(this, &VSQSqlConversationModel::setMessageStatus, this, &VSQSqlConversationModel::onSetMessageStatus);
    connect(this, &VSQSqlConversationModel::setAttachmentStatus, this, &VSQSqlConversationModel::onSetAttachmentStatus);
    connect(this, &VSQSqlConversationModel::setAttachmentFilePath, this, &VSQSqlConversationModel::onSetAttachmentFilePath);
    connect(this, &VSQSqlConversationModel::setAttachmentProgress, this, &VSQSqlConversationModel::onSetAttachmentProgress);
    connect(this, &VSQSqlConversationModel::setAttachmentThumbnailPath, this, &VSQSqlConversationModel::onSetAttachmentThumbnailPath);
    connect(this, &VSQSqlConversationModel::setAttachmentRemoteUrl, this, &VSQSqlConversationModel::onSetAttachmentRemoteUrl);
    connect(this, &VSQSqlConversationModel::setAttachmentThumbnailRemoteUrl, this, &VSQSqlConversationModel::onSetAttachmentThumbnailRemoteUrl);
    connect(this, &VSQSqlConversationModel::setAttachmentBytesTotal, this, &VSQSqlConversationModel::onSetAttachmentBytesTotal);

    connect(this, &VSQSqlConversationModel::modelReset, this, []() {
        static int reloadCounter = 0;
        qWarning() << "Full model reset" << ++reloadCounter;
    });

    m_selectTimer.setSingleShot(true);
    m_selectTimer.setInterval(1000);
    connect(&m_selectTimer, &QTimer::timeout, this, &VSQSqlConversationModel::performSelect);
}

/******************************************************************************/
QString
VSQSqlConversationModel::recipient() const {
    return m_recipient;
}

/******************************************************************************/
void
VSQSqlConversationModel::setRecipient(const QString &recipient) {
    if (recipient == m_recipient) {
        return;
    }

    // TODO: Prevent SQL injection !!!
    m_recipient = recipient;

    const QString filterString = QString::fromLatin1(
        "(recipient = '%1' AND author = '%2') OR (recipient = '%2' AND author='%1')").arg(m_recipient, user());

    setSort(2, Qt::AscendingOrder);
    setFilter(filterString);

    emit recipientChanged();
}

/******************************************************************************/
QVariant
VSQSqlConversationModel::data(const QModelIndex &index, int role) const {
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
    }

    if (role == AttachmentIdRole) {
        static int reloadCounter = 0;
        qDebug() << "Re-loading of message" << ++reloadCounter << "model row" << index.row();
    }

    const QSqlRecord currRecord = record(index.row());
    const int authorColumn = AuthorRole - Qt::UserRole;
    const int timestampColumn = TimestampRole - Qt::UserRole;

    if (role == FirstInRowRole) {
        const QSqlRecord prevRecord = record(index.row() - 1);
        const QVariant prevMsgAuthor = prevRecord.value(authorColumn);
        const QVariant currMsgAuthor = currRecord.value(authorColumn);
        const QVariant prevTimestamp = prevRecord.value(timestampColumn);
        const QVariant currTimestamp = currRecord.value(timestampColumn);

        // Check if previous message is from the same author
        const bool isAuthor = currMsgAuthor.toString() != prevMsgAuthor.toString();

        // Check if the message was sent in last 5 min
        const bool isInFiveMinRange = prevTimestamp.toDateTime().addSecs(5 * 60) > currTimestamp.toDateTime();

        // Message is considered to be the first in a row when it
        // sends in a range of 1 min with previous message and
        // from the same author
        return isAuthor || !isInFiveMinRange;
    }

    if (role == InRowRole) {
        const QSqlRecord nextRecord = record(index.row() + 1);
        const QVariant nextMsgAuthor = nextRecord.value(authorColumn);
        const QVariant currMsgAuthor = currRecord.value(authorColumn);

        return currMsgAuthor.toString() == nextMsgAuthor.toString();
    }

    if (role == DayRole) {
        const QVariant timestamp = currRecord.value(timestampColumn);
        return timestamp.toDate();
    }

    if (role == StatusRole) {
        const int statusColumn = StatusRole - Qt::UserRole;
        const QVariant status = currRecord.value(statusColumn);
        const QSqlRecord nextRecord = record(index.row() + 1);
        if (currRecord.value(authorColumn) == nextRecord.value(authorColumn) && status == nextRecord.value(statusColumn)) {
            return QString();
        }
        return status;
    }

    const auto attachmentId = currRecord.value(AttachmentIdRole - Qt::UserRole).toString();

    if (role == AttachmentDisplaySizeRole) {
        if (attachmentId.isEmpty()) {
            return QString();
        }
        const auto bytesTotal = currRecord.value(AttachmentBytesTotalRole - Qt::UserRole).toInt();
        return (bytesTotal > 0) ? vm::Utils::formattedDataSize(bytesTotal) : " ";
    }

    if (role == AttachmentStatusRole) {
        if (attachmentId.isEmpty()) {
            return 0;
        }
        const auto messageId = currRecord.value(MessageIdRole - Qt::UserRole).toString();
        const auto it = m_transferMap.find(messageId);
        if (it == m_transferMap.end()) {
            return currRecord.value(AttachmentStatusRole - Qt::UserRole).toInt();
        }
        return static_cast<int>(it->second.status);
    }

    if (role == AttachmentBytesLoadedRole) {
        if (attachmentId.isEmpty()) {
            return 0;
        }
        const auto messageId = currRecord.value(MessageIdRole - Qt::UserRole).toString();
        const auto it = m_transferMap.find(messageId);
        return (it == m_transferMap.end()) ? 0 : it->second.bytesReceived;
    }

    if (role == AttachmentDownloadedRole) {
        if (attachmentId.isEmpty()) {
            return false;
        }
        const auto filePath = currRecord.value(AttachmentFilePathRole - Qt::UserRole).toString();
        return QFile::exists(filePath);
    }

    if (role == AttachmentFilePathRole || role == AttachmentThumbnailPathRole) {
        if (attachmentId.isEmpty()) {
            return QUrl();
        }
        const auto filePath = currRecord.value(role - Qt::UserRole).toString();
        return QUrl::fromLocalFile(filePath);
    }

    return currRecord.value(role - Qt::UserRole);
}

/******************************************************************************/
QHash<int, QByteArray>
VSQSqlConversationModel::roleNames() const {
    QHash<int, QByteArray> names;
    names[AuthorRole] = "author";
    names[RecipientRole] = "recipient";
    names[TimestampRole] = "timestamp";
    names[MessageRole] = "message";
    names[StatusRole] = "status";
    names[MessageIdRole] = "messageId";
    names[FirstInRowRole] = "firstMessageInARow";
    names[InRowRole] = "messageInARow";
    names[DayRole] = "day";
    names[AttachmentIdRole] = "attachmentId";
    names[AttachmentBytesTotalRole] = "attachmentBytesTotal";
    names[AttachmentTypeRole] = "attachmentType";
    names[AttachmentFilePathRole] = "attachmentFilePath";
    names[AttachmentFileNameRole] = "attachmentFileName";
    names[AttachmentThumbnailPathRole] = "attachmentThumbnailPath";
    names[AttachmentThumbnailWidthRole] = "attachmentThumbnailWidth";
    names[AttachmentThumbnailHeightRole] = "attachmentThumbnailHeight";
    names[AttachmentStatusRole] = "attachmentStatus";
    names[AttachmentDisplaySizeRole] = "attachmentDisplaySize";
    names[AttachmentBytesLoadedRole] = "attachmentBytesLoaded";
    names[AttachmentDownloadedRole] = "attachmentDownloaded";
    return names;
}

/******************************************************************************/

QString
VSQSqlConversationModel::user() const {
    return m_user;
}

/******************************************************************************/
void
VSQSqlConversationModel::setUser(const QString &user) {
    m_recipient = "";
    if (user == m_user) {
        return;
    }

    m_user = user;

    _createTable();
    _update();
}

/******************************************************************************/
void VSQSqlConversationModel::setAsRead(const QString &author) {
    QSqlQuery model;
    QString query;

    query = QString("UPDATE %1 SET status = %2 WHERE author = \"%3\"")
            .arg(_tableName()).arg(static_cast<int>(StMessage::Status::MST_READ)).arg(author);

    model.prepare(query);

    qDebug() << query << model.exec();
}

QList<StMessage> VSQSqlConversationModel::getMessages(const QString &user, const StMessage::Status status) {
    QSqlQueryModel model;
    QString query;
    query = QString("SELECT * FROM %1 WHERE status = %2 AND author = '%3' ORDER BY timestamp")
            .arg(_tableName()).arg(static_cast<int>(status)).arg(user);
    model.setQuery(query);
    int c = model.rowCount();
    qDebug() << c << user << query;

    QList<StMessage> messages;
    for (int i = 0; i < c; i++) {
        messages.append(getMessage(model.record(i)));
    }
    return messages;
}

Optional<StMessage> VSQSqlConversationModel::getMessage(const QString &messageId) const
{
    QSqlQueryModel model;
    model.setQuery(QString("SELECT * FROM %1 WHERE message_id = \"%2\"").arg(_tableName()).arg(messageId));
    if (model.rowCount() == 0) {
        return NullOptional;
    }
    return getMessage(model.record(0));
}

StMessage VSQSqlConversationModel::getMessage(const QSqlRecord &record) const
{
    StMessage message;
    message.messageId = record.value("message_id").toString();
    message.message = record.value("message").toString();
    message.sender = record.value("author").toString();
    message.recipient = record.value("recipient").toString();
    const auto attachmentId = record.value("attachment_id").toString();
    if (!attachmentId.isEmpty()) {
        Attachment attachment;
        attachment.id = attachmentId;
        attachment.bytesTotal = record.value("attachment_bytes_total").toInt();
        attachment.type = static_cast<Attachment::Type>(record.value("attachment_type").toInt());
        attachment.filePath = record.value("attachment_file_path").toString();
        attachment.fileName = record.value("attachment_file_name").toString();
        attachment.remoteUrl = record.value("attachment_remote_url").toString();
        if (attachment.type == Attachment::Type::Picture) {
            attachment.thumbnailPath = record.value("attachment_thumbnail_path").toString();
            attachment.thumbnailSize.setWidth(record.value("attachment_thumbnail_width").toInt());
            attachment.thumbnailSize.setHeight(record.value("attachment_thumbnail_height").toInt());
            attachment.remoteThumbnailUrl = record.value("attachment_remote_thumbnail_url").toString();
        }
        attachment.status = static_cast<Attachment::Status>(record.value("attachment_status").toInt());
        attachment.displayName = message.message;
        message.attachment = attachment;
    }
    return message;
}

/******************************************************************************/
QString VSQSqlConversationModel::_tableName() const {
    return QString("Conversations_") + m_validator->databaseUsername(m_user);
}

/******************************************************************************/
QString VSQSqlConversationModel::_contactsTableName() const {
    return QString("Contacts_") + m_validator->databaseUsername(m_user);
}

void VSQSqlConversationModel::onCreateMessage(const QString recipient, const QString message, const QString messageId,
                                              const OptionalAttachment attachment)
{
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlRecord newRecord = record();
    newRecord.setValue("author", user());
    newRecord.setValue("recipient", recipient);
    newRecord.setValue("timestamp", timestamp);
    newRecord.setValue("message", message);
    newRecord.setValue("status", static_cast<int>(StMessage::Status::MST_CREATED));
    newRecord.setValue("message_id", messageId);
    if (attachment) {
        newRecord.setValue("attachment_id", attachment->id);
        newRecord.setValue("attachment_bytes_total", attachment->bytesTotal);
        newRecord.setValue("attachment_type", static_cast<int>(attachment->type));
        newRecord.setValue("attachment_file_path", attachment->filePath);
        newRecord.setValue("attachment_file_name", attachment->fileName);
        newRecord.setValue("attachment_remote_url", attachment->remoteUrl.toString());
        if (attachment->type == Attachment::Type::Picture) {
            newRecord.setValue("attachment_thumbnail_path", attachment->thumbnailPath);
            newRecord.setValue("attachment_thumbnail_width", attachment->thumbnailSize.width());
            newRecord.setValue("attachment_thumbnail_height", attachment->thumbnailSize.height());
            newRecord.setValue("attachment_remote_thumbnail_url", attachment->remoteThumbnailUrl.toString());
        }
        newRecord.setValue("attachment_status", static_cast<int>(attachment->status));
    }
    if (!insertRowIntoTable(newRecord)) {
        qWarning() << "Failed to create message:" << lastError().text();
        return;
    }

    submitAll();
    qDebug() << "Submitted!";
    scheduleSelect({});
}

void VSQSqlConversationModel::onReceiveMessage(const QString messageId, const QString author, const QString message, const OptionalAttachment attachment)
{
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlRecord newRecord = record();
    newRecord.setValue("author", author);
    newRecord.setValue("recipient", user());
    newRecord.setValue("timestamp", timestamp);
    newRecord.setValue("message", message);
    newRecord.setValue("status", static_cast<int>(StMessage::Status::MST_RECEIVED));
    newRecord.setValue("message_id", messageId);
    if (attachment) {
        // TODO(fpohtmeh): merge with onCreateMessage
        newRecord.setValue("attachment_id", attachment->id);
        newRecord.setValue("attachment_bytes_total", attachment->bytesTotal);
        newRecord.setValue("attachment_type", static_cast<int>(attachment->type));
        newRecord.setValue("attachment_file_path", attachment->filePath);
        newRecord.setValue("attachment_file_name", attachment->fileName);
        newRecord.setValue("attachment_remote_url", attachment->remoteUrl.toString());
        if (attachment->type == Attachment::Type::Picture) {
            newRecord.setValue("attachment_thumbnail_path", attachment->thumbnailPath);
            newRecord.setValue("attachment_thumbnail_width", attachment->thumbnailSize.width());
            newRecord.setValue("attachment_thumbnail_height", attachment->thumbnailSize.height());
            newRecord.setValue("attachment_remote_thumbnail_url", attachment->remoteThumbnailUrl.toString());
        }
        newRecord.setValue("attachment_status", static_cast<int>(attachment->status));
    }
    if (!insertRowIntoTable(newRecord)) {
        qWarning() << "Failed to save received message:" << lastError().text();
        return;
    }

    submitAll();
    qDebug() << "Submitted!";
    scheduleSelect({});
}

void VSQSqlConversationModel::onSetMessageStatus(const QString messageId, const StMessage::Status status)
{
    qDebug() << "SQL message status:" << messageId << "=>" << status;
    QString query = QString("UPDATE %1 SET status = %2 WHERE message_id = '%3'")
            .arg(_tableName()).arg(static_cast<int>(status)).arg(messageId);
    QSqlQuery().exec(query);
    scheduleSelect({ StatusRole });
}

void VSQSqlConversationModel::onSetAttachmentStatus(const QString messageId, const Enums::AttachmentStatus status)
{
    qDebug() << "SQL attachment status:" << messageId << "=>" << status;
    QString query = QString("UPDATE %1 SET attachment_status = %2 WHERE message_id = '%3'")
            .arg(_tableName()).arg(static_cast<int>(status)).arg(messageId);
    QSqlQuery().exec(query);

    if (status == Attachment::Status::Loading) {
        m_transferMap[messageId] = TransferInfo();
    }
    else if (status == Attachment::Status::Failed || status == Attachment::Status::Loaded) {
        const auto it = m_transferMap.find(messageId);
        if (it != m_transferMap.end()) {
            m_transferMap.erase(it);
        }
    }
    scheduleSelect({ AttachmentBytesLoadedRole, AttachmentStatusRole });
}

void VSQSqlConversationModel::onSetAttachmentRemoteUrl(const QString messageId, const QUrl url)
{
    QString query = QString("UPDATE %1 SET attachment_remote_url = '%2' WHERE message_id = '%3'")
            .arg(_tableName()).arg(url.toString()).arg(messageId);
    QSqlQuery().exec(query);
    qDebug() << "SQL attachment remote url:" << messageId << "=>" << url.toString();
    scheduleSelect({ AttachmentRemoteUrlRole });
}

void VSQSqlConversationModel::onSetAttachmentThumbnailRemoteUrl(const QString messageId, const QUrl url)
{
    QString query = QString("UPDATE %1 SET attachment_remote_thumbnail_url = '%2' WHERE message_id = '%3'")
            .arg(_tableName()).arg(url.toString()).arg(messageId);
    QSqlQuery().exec(query);
    qDebug() << "SQL attachment remote thumbnail url:" << messageId << "=>" << url.toString();
    scheduleSelect({ AttachmentRemoteThumbnailUrlRole });
}

void VSQSqlConversationModel::onSetAttachmentBytesTotal(const QString messageId, const DataSize size)
{
    QString query = QString("UPDATE %1 SET attachment_bytes_total = %2 WHERE message_id = '%3'")
            .arg(_tableName()).arg(size).arg(messageId);
    QSqlQuery().exec(query);
    qDebug() << "SQL attachment filesize:" << messageId << "=>" << size;
    scheduleSelect({ AttachmentBytesTotalRole, AttachmentDisplaySizeRole });
}

void VSQSqlConversationModel::onSetAttachmentFilePath(const QString messageId, const QString filePath)
{
    QString query = QString("UPDATE %1 SET attachment_file_path = '%2' WHERE message_id = '%3'")
            .arg(_tableName()).arg(filePath).arg(messageId);
    QSqlQuery().exec(query);
    qDebug() << "SQL attachment filePath:" << messageId << "=>" << filePath;
    scheduleSelect({ AttachmentDownloadedRole });
}

void VSQSqlConversationModel::onSetAttachmentProgress(const QString messageId, const DataSize bytesReceived, const DataSize bytesTotal)
{
    Q_UNUSED(bytesTotal)
    //qCDebug(lcTransferManager) << "Transfer progress" << bytesReceived << bytesTotal << messageId;
    const auto it = m_transferMap.find(messageId);
    if (it != m_transferMap.end()) {
        it->second.bytesReceived = bytesReceived;
    }
    scheduleSelect({ AttachmentBytesLoadedRole });
}

void VSQSqlConversationModel::onSetAttachmentThumbnailPath(const QString messageId, const QString filePath)
{
    QString query = QString("UPDATE %1 SET attachment_thumbnail_path = '%2' WHERE message_id = '%3'")
            .arg(_tableName()).arg(filePath).arg(messageId);
    QSqlQuery().exec(query);
    qDebug() << "SQL attachment thumbnail path:" << messageId << "=>" << filePath;
    scheduleSelect({ AttachmentThumbnailPathRole });
}

void VSQSqlConversationModel::scheduleSelect(const QVector<int> &roles)
{
    for (auto role : roles) {
        if (!m_selectRoles.contains(role)) {
            m_selectRoles << role;
        }
    }
    if (!m_selectTimer.isActive()) {
        m_selectTimer.start();
    }
}

void VSQSqlConversationModel::performSelect()
{
    qDebug() << "Performing of select. Roles:" << m_selectRoles;
    m_selectTimer.stop();
    select();
    emit dataChanged(index(0, 0), index(0, rowCount() - 1), m_selectRoles);
    m_selectRoles.clear();
}
