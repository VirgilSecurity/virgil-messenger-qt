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

#ifndef VIRGIL_IOTKIT_QT_SQL_CONVERSATION_MODEL_H
#define VIRGIL_IOTKIT_QT_SQL_CONVERSATION_MODEL_H

#include <QSqlTableModel>
#include <QTimer>

#include "Validator.h"
#include "VSQCommon.h"

class VSQCryptoTransferManager;

class VSQSqlConversationModel : public QSqlTableModel
{
    Q_OBJECT
    Q_PROPERTY(QString recipient READ recipient WRITE setRecipient NOTIFY recipientChanged)

    enum Roles
    {
        AuthorRole = Qt::UserRole,
        RecipientRole,
        TimestampRole,
        MessageRole,
        StatusRole,
        MessageIdRole,

        AttachmentIdRole,
        AttachmentBytesTotalRole,
        AttachmentTypeRole,
        AttachmentFilePathRole,
        AttachmentFileNameRole,
        AttachmentRemoteUrlRole,
        AttachmentThumbnailPathRole,
        AttachmentThumbnailWidthRole,
        AttachmentThumbnailHeightRole,
        AttachmentRemoteThumbnailUrlRole,
        AttachmentStatusRole,

        FirstInRowRole,
        InRowRole,
        DayRole,
        AttachmentDisplaySizeRole,
        AttachmentBytesLoadedRole,
        AttachmentDownloadedRole
    };

public:
    explicit VSQSqlConversationModel(vm::Validator *validator, QObject *parent = nullptr);

    QString
    user() const;

    Q_INVOKABLE void
    setUser(const QString &user);

    QString
    recipient() const;

    Q_INVOKABLE void
    setRecipient(const QString &recipient);

    QVariant
    data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray>
    roleNames() const override;

    Q_INVOKABLE void
    setAsRead(const QString &author);

    QList<StMessage> getMessages(const QString &user, const StMessage::Status status);

    Optional<StMessage> getMessage(const QString &messageId) const;
    StMessage getMessage(const QSqlRecord &record) const;

signals:
    void createMessage(const QString recipient, const QString message, const QString messageId, const OptionalAttachment attachment);
    void receiveMessage(const QString messageId, const QString author, const QString message, const OptionalAttachment attachment);
    void setMessageStatus(const QString messageId, const StMessage::Status status);
    void setAttachmentFilePath(const QString &messageId, const QString &filePath);
    void setAttachmentProgress(const QString &messageId, const DataSize bytesReceived, const DataSize bytesTotal);
    void setAttachmentThumbnailPath(const QString messageId, const QString filePath);
    void setAttachmentStatus(const QString messageId, const Enums::AttachmentStatus status);
    void setAttachmentRemoteUrl(const QString messageId, const QUrl url);
    void setAttachmentThumbnailRemoteUrl(const QString messageId, const QUrl url);
    void setAttachmentBytesTotal(const QString messageId, const DataSize size);

    void recipientChanged();

private:
    struct TransferInfo
    {
        DataSize bytesReceived = 0;
        AttachmentV0::Status status = AttachmentV0::Status::Loading;
    };

    vm::Validator *m_validator;
    QString m_user;
    QString m_recipient;
    std::map<QString, TransferInfo> m_transferMap;

    void
    _createTable();

    void
    _update();

    QString
    _tableName() const;

    QString
    _contactsTableName() const;

    void onCreateMessage(const QString recipient, const QString message, const QString messageId, const OptionalAttachment attachment);
    void onReceiveMessage(const QString messageId, const QString author, const QString message, const OptionalAttachment attachment);
    void onSetMessageStatus(const QString messageId, const StMessage::Status status);
    void onSetAttachmentStatus(const QString messageId, const Enums::AttachmentStatus status);
    void onSetAttachmentFilePath(const QString messageId, const QString filePath);
    void onSetAttachmentProgress(const QString messageId, const DataSize bytesReceived, const DataSize bytesTotal);
    void onSetAttachmentThumbnailPath(const QString messageId, const QString filePath);
    void onSetAttachmentRemoteUrl(const QString messageId, const QUrl url);
    void onSetAttachmentThumbnailRemoteUrl(const QString messageId, const QUrl url);
    void onSetAttachmentBytesTotal(const QString messageId, const DataSize size);

    void scheduleSelect(const QVector<int> &roles);
    void performSelect();

    QVector<int> m_selectRoles;
    QTimer m_selectTimer;
};

#endif // VIRGIL_IOTKIT_QT_SQL_CONVERSATION_MODEL_H
