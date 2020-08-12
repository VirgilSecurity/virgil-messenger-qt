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

#include "VSQUploader.h"
#include "VSQUtils.h"

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
        "attachment_local_url TEXT,"
        "attachment_enc_local_url TEXT,"
        "attachment_remote_url TEXT,"
        "attachment_thumbnail_url TEXT,"
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
VSQSqlConversationModel::VSQSqlConversationModel(QObject *parent) :
    QSqlTableModel(parent) {

    qRegisterMetaType<StMessage::Status>("StMessage::Status");

    connect(this, &VSQSqlConversationModel::createMessage, this, &VSQSqlConversationModel::onCreateMessage);
    connect(this, &VSQSqlConversationModel::receiveMessage, this, &VSQSqlConversationModel::onReceiveMessage);
    connect(this, &VSQSqlConversationModel::setMessageStatus, this, &VSQSqlConversationModel::onSetMessageStatus);
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

        // select();

    emit recipientChanged();
}

/******************************************************************************/
QVariant
VSQSqlConversationModel::data(const QModelIndex &index, int role) const {
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
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

    if (role == AttachmentDisplaySizeRole) {
        const QString attachmentId = currRecord.value(AttachmentIdRole - Qt::UserRole).toString();
        if (attachmentId.isEmpty()) {
            return QString();
        }
        return VSQUtils::formattedDataSize(currRecord.value(AttachmentBytesTotalRole - Qt::UserRole).toInt());
    }

    if (role == AttachmentStatusRole) {
        auto it = m_uploadInfos.find(currRecord.value(MessageIdRole - Qt::UserRole).toString());
        if (it == m_uploadInfos.end())
            return currRecord.value(AttachmentStatusRole - Qt::UserRole).toInt();
        else
            return static_cast<int>(it->second.status);
    }

    if (role == AttachmentBytesLoadedRole) {
        auto it = m_uploadInfos.find(currRecord.value(MessageIdRole - Qt::UserRole).toString());
        return (it == m_uploadInfos.end()) ? 0 : it->second.bytesUploaded;
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
    names[MessageIdRole] = "message_id";
    names[FirstInRowRole] = "firstMessageInARow";
    names[InRowRole] = "messageInARow";
    names[DayRole] = "day";
    names[AttachmentIdRole] = "attachmentId";
    names[AttachmentBytesTotalRole] = "attachmentBytesTotal";
    names[AttachmentTypeRole] = "attachmentType";
    names[AttachmentLocalUrlRole] = "attachmentLocalUrl";
    names[attachmentThumbnailUrlRole] = "attachmentThumbnailUrl";
    names[AttachmentStatusRole] = "attachmentStatus";
    names[AttachmentDisplaySizeRole] = "attachmentDisplaySize";
    names[AttachmentBytesLoadedRole] = "attachmentBytesLoaded";
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

/******************************************************************************/
int
VSQSqlConversationModel::getCountOfUnread(const QString &user) {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT COUNT(*) AS C FROM %1 WHERE status = %2 AND recipient = \"%3\"")
            .arg(_tableName()).arg(static_cast<int>(StMessage::Status::MST_RECEIVED)).arg(user);

    model.setQuery(query);
    int c = model.record(0).value("C").toInt();

    qDebug() << c << user << query;

    return c;
}


/******************************************************************************/
int
VSQSqlConversationModel::getMessageCount(const QString &user, const StMessage::Status status) {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT COUNT(*) AS C FROM %1 WHERE status = %2 AND recipient = \"%3\"")
            .arg(_tableName()).arg(static_cast<int>(status)).arg(user);

    model.setQuery(query);
    int c = model.record(0).value("C").toInt();

    qDebug() << c << user << query;

    return c;
}

/******************************************************************************/
QString
VSQSqlConversationModel::getLastMessage(const QString &user) const {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT * FROM %1 WHERE recipient = \"%2\" ORDER BY timestamp DESC LIMIT 1").arg(_tableName()).arg(user);

    model.setQuery(query);
    QString message = model.record(0).value("message").toString();

    qDebug() << message << user << query;

    return message;
}

QList<StMessage> VSQSqlConversationModel::getMessages(const QString &user, const StMessage::Status status) {
    QSqlQueryModel model;
    QString query;

    QList<StMessage> messages;

    query = QString("SELECT message, recipient, message_id FROM %1 WHERE status = %2 AND author = \"%3\"")
            .arg(_tableName()).arg(static_cast<int>(status)).arg(user);

    model.setQuery(query);
    int c = model.rowCount();

    if (c == 0) {
        return messages;
    }

    for (int i = 0; i < c; i++) {
        StMessage message;
        message.message = model.record(i).value("message").toString();
        message.recipient = model.record(i).value("recipient").toString();
        message.message_id = model.record(i).value("message_id").toString();
        messages.append(message);
    }

    qDebug() << c << user << query;

    return messages;
}

void VSQSqlConversationModel::connectUploader(VSQUploader *uploader)
{
    connect(uploader, &VSQUploader::uploadStatusChanged, this, &VSQSqlConversationModel::onUploadStatusChanged);
    connect(uploader, &VSQUploader::uploadProgressChanged, this, &VSQSqlConversationModel::onUploadProgressChanged);
}

/******************************************************************************/
QString VSQSqlConversationModel::escapedUserName() const
{
    QString name(m_user);
    name.remove(QRegExp("[^a-z0-9_]"));
    return name;
}

/******************************************************************************/
QString
VSQSqlConversationModel::getLastMessageTime(const QString &user) const {
    QSqlQueryModel model;
    QString query;

    query = QString("SELECT * FROM %1 WHERE recipient = \"%2\" ORDER BY timestamp DESC LIMIT 1").arg(_tableName()).arg(user);

    model.setQuery(query);
    QString timestamp = model.record(0).value("timestamp").toString();

    qDebug() << timestamp << user << query;

    return timestamp;
}

/******************************************************************************/
QString VSQSqlConversationModel::_tableName() const {
    return QString("Conversations_") + escapedUserName();
}

/******************************************************************************/
QString VSQSqlConversationModel::_contactsTableName() const {
    return QString("Contacts_") + escapedUserName();
}

void VSQSqlConversationModel::onCreateMessage(const QString &recipient, const QString &message, const QString &messageId,
                                              const OptionalAttachment &attachment)
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
        newRecord.setValue("attachment_local_url", attachment->localUrl);
        newRecord.setValue("attachment_enc_local_url", attachment->encLocalUrl);
        newRecord.setValue("attachment_thumbnail_url", attachment->thumbnailUrl);
    }
    if (!insertRecord(rowCount(), newRecord)) {
        qWarning() << "Failed to create message:" << lastError().text();
        return;
    }

    submitAll();
    select();
}

void VSQSqlConversationModel::onReceiveMessage(const QString &messageId, const QString &author, const QString &message, const OptionalAttachment &attachment)
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
        newRecord.setValue("attachment_id", attachment->id);
        newRecord.setValue("attachment_bytes_total", attachment->bytesTotal);
        newRecord.setValue("attachment_type", static_cast<int>(attachment->type));
        newRecord.setValue("attachment_remove_url", attachment->remoteUrl);
    }
    if (!insertRowIntoTable(newRecord)) {
        qWarning() << "Failed to save received message:" << lastError().text();
        return;
    }

    // qDebug() << newRecord

    submitAll();
}

void VSQSqlConversationModel::onSetMessageStatus(const QString &messageId, const StMessage::Status status)
{
    QSqlQuery model;
    QString query;

    query = QString("UPDATE %1 SET status = %2 WHERE message_id = \"%3\"")
            .arg(_tableName()).arg(static_cast<int>(status)).arg(messageId);

    model.prepare(query);

    qDebug() << query << model.exec();

    select();
}

void VSQSqlConversationModel::onUploadProgressChanged(const QString &messageId, const DataSize bytesUploaded)
{
    //qCDebug(lcUploader) << "Upload progress" << bytesUploaded << messageId;
    auto it = m_uploadInfos.find(messageId);
    if (it != m_uploadInfos.end()) {
        it->second.bytesUploaded = bytesUploaded;
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { AttachmentBytesLoadedRole });
    }
}

void VSQSqlConversationModel::onUploadStatusChanged(const QString &messageId, const Enums::AttachmentStatus status)
{
    qCDebug(lcUploader) << "Upload status" << status << messageId;
    if (status == Attachment::Status::Loading) {
        m_uploadInfos[messageId] = UploadInfo();
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0), { AttachmentStatusRole });
    }
    else if (status == Attachment::Status::Loaded || status == Attachment::Status::Failed) {
        auto it = m_uploadInfos.find(messageId);
        if (it != m_uploadInfos.end()) {
            m_uploadInfos.erase(it);
        }
        // Write status to DB
        QSqlQuery model;
        auto query = QString("UPDATE %1 SET attachment_status = %2 WHERE message_id = \"%3\"")
                .arg(_tableName()).arg(static_cast<int>(status)).arg(messageId);
        model.prepare(query);
        model.exec();
        select();
    }
}
