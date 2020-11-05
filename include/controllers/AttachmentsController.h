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

#ifndef VM_ATTACHMENTSCONTROLLER_H
#define VM_ATTACHMENTSCONTROLLER_H

#include <QObject>

#include "VSQCommon.h"

class Settings;

namespace vm
{
class Models;

class AttachmentsController : public QObject
{
    Q_OBJECT

public:
    explicit AttachmentsController(Settings *settings, Models *models, QObject *parent);

    Q_INVOKABLE void saveAs(const Message::Id &messageId, const QVariant &fileUrl);
    Q_INVOKABLE void download(const Message::Id &messageId);
    Q_INVOKABLE void open(const Message::Id &messageId);

    void setUserId(const UserId &userId);
    void setContactId(const Contact::Id &contactId);

signals:
    void openPreviewRequested(const QUrl &url);
    void notificationCreated(const QString &notification);

private:
    void downloadAttachment(const GlobalMessage &message);
    Optional<GlobalMessage> findMessage(const Message::Id &messageId) const;

    Settings *m_settings;
    Models *m_models;
    UserId m_userId;
    Contact::Id m_contactId;
};
}

#endif // VM_ATTACHMENTSCONTROLLER_H
