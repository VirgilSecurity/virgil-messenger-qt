//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#ifndef VM_CHAT_OBJECT_H
#define VM_CHAT_OBJECT_H

#include <QObject>

#include "Chat.h"
#include "GroupMembersModel.h"
#include "GroupUpdate.h"
#include "User.h"

namespace vm {
class Messenger;

class ChatObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString lastActivityText MEMBER m_lastActivityText NOTIFY lastActivityTextChanged)
    Q_PROPERTY(bool isGroup READ isGroup NOTIFY isGroupChanged)
    Q_PROPERTY(GroupMembersModel *groupMembers MEMBER m_groupMembersModel CONSTANT)
    Q_PROPERTY(bool userIsOwner MEMBER m_userIsOwner NOTIFY userIsOwnerChanged)
    Q_PROPERTY(bool userCanEdit MEMBER m_userCanEdit NOTIFY userCanEditChanged)

public:
    ChatObject(Messenger *messenger, QObject *parent);

    void setChat(const ChatHandler &chat);
    ChatHandler chat() const;

    QString title() const;
    bool isGroup() const;

    void setGroupMembers(const GroupMembers &groupMembers);
    GroupMembers selectedGroupMembers() const;

    void updateGroup(const GroupUpdate &groupUpdate);

signals:
    void titleChanged(const QString &title);
    void lastActivityTextChanged(const QString &text);
    void isGroupChanged(bool isGroup);

    void userIsOwnerChanged(bool isOwner);
    void userCanEditChanged(bool canEdit);

private:
    void setLastActivityText(const QString &text);

    QPointer<Messenger> m_messenger;
    ChatHandler m_chat;
    GroupMembersModel *m_groupMembersModel;
    QString m_lastActivityText;
    bool m_userIsOwner = false;
    bool m_userCanEdit = false;
};
} // namespace vm

#endif // VM_CHAT_OBJECT_H
