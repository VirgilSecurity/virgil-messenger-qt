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

#include "ChatObject.h"

#include "ListSelectionModel.h"
#include "Messenger.h"

using namespace vm;

ChatObject::ChatObject(Messenger *messenger, QObject *parent)
    : QObject(parent)
    , m_groupMembersModel(new GroupMembersModel(this, true))
{
    m_groupMembersModel->selection()->setMultiSelect(true);

    connect(messenger, &Messenger::lastActivityTextChanged, this, &ChatObject::setLastActivityText);
}

void ChatObject::setChat(const ChatHandler &chat)
{
    const auto oldTitle = title();
    const auto oldIsGroup = isGroup();
    m_chat = chat;
    m_groupInvitationOwnerId = UserId();
    m_groupMembersModel->setGroupMembers({});
    if (oldTitle != title()) {
        emit titleChanged(title());
    }
    if (oldIsGroup != isGroup()) {
        emit isGroupChanged(isGroup());
    }
}

ChatHandler ChatObject::chat() const
{
    return m_chat;
}

QString ChatObject::title() const
{
    return m_chat ? m_chat->title() : QString();
}

bool ChatObject::isGroup() const
{
    return m_chat && m_chat->type() == ChatType::Group;
}

void ChatObject::setCurrentUser(const UserHandler &user)
{
    m_groupMembersModel->setCurrentUser(user);
}

void ChatObject::setGroupInvitationOwnerId(const UserId &ownerId)
{
    m_groupInvitationOwnerId = ownerId;
}

UserId ChatObject::groupInvitationOwnerId() const
{
    return m_groupInvitationOwnerId;
}

void ChatObject::setGroupMembers(const GroupMembers &groupMembers)
{
    m_groupMembersModel->setGroupMembers(groupMembers);
}

GroupMembers ChatObject::selectedGroupMembers() const
{
    return ContactsToGroupMembers(GroupId(m_chat->id()), m_groupMembersModel->selectedContacts());
}

void ChatObject::setLastActivityText(const QString &text)
{
    if (text == m_lastActivityText) {
        return;
    }
    m_lastActivityText = text;
    emit lastActivityTextChanged(text);
}
