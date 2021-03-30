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

#include "CloudFileMembersModel.h"

#include "ListSelectionModel.h"
#include "Messenger.h"

using namespace vm;
using Self = CloudFileMembersModel;

Self::CloudFileMembersModel(Messenger *messenger, QObject *parent) : ContactsModel(parent), m_messenger(messenger) { }

void Self::setMembers(const CloudFileMembers &members)
{
    m_members = members;
    setContacts(CloudFileMembersToContacts(members));

    const auto it = std::find_if(members.begin(), members.end(),
                                 [](auto m) { return m->type() == CloudFileMember::Type::Owner; });

    const auto owner = (it == members.end()) ? ContactHandler() : (*it)->contact();
    const auto currentUser = m_messenger->currentUser();
    const bool isOwnedByUser = currentUser && owner && currentUser->id() == owner->userId();
    const bool isReadOnly = !isOwnedByUser;

    if (m_isReadOnly != isReadOnly) {
        m_isReadOnly = isReadOnly;
        emit isReadOnlyChanged(isReadOnly);
    }
    if (m_isOwnedByUser != isOwnedByUser) {
        m_isOwnedByUser = isOwnedByUser;
        emit isOwnedByUserChanged(isOwnedByUser);
    }
}

CloudFileMembers Self::members() const
{
    return m_members;
}

[[nodiscard]] CloudFileMemberHandler Self::findMemberById(const UserId &memberId) const
{
    const auto it = std::find_if(m_members.cbegin(), m_members.cend(),
                                 [&memberId](const auto &member) { return memberId == member->memberId(); });

    if (it != m_members.cend()) {
        return *it;
    }

    return nullptr;
}

CloudFileMembers Self::selectedMembers() const
{
    CloudFileMembers result;
    for (auto &i : selection()->selectedIndexes()) {
        result.push_back(m_members[i.row()]);
    }
    return result;
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case DetailsRole:
        return m_members[index.row()]->typeAsDisplayString();

    case SortRole: {
        const auto &m = m_members[index.row()];
        return (m->type() == CloudFileMember::Type::Owner ? QLatin1Char('0') : QLatin1Char('1'))
                + m->contact()->displayName();
    }

    default:
        return ContactsModel::data(index, role);
    }
}
