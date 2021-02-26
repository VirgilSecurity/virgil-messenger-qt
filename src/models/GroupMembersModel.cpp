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

#include "GroupMembersModel.h"

#include "ListSelectionModel.h"
#include "Messenger.h"

using namespace vm;
using Self = GroupMembersModel;

Self::GroupMembersModel(Messenger *messenger, QObject *parent) : ContactsModel(parent), m_messenger(messenger)
{
    selection()->setMultiSelect(true);
}

void Self::setGroupMembers(const GroupMembers &groupMembers)
{
    m_groupMembers = groupMembers;
    setContacts(GroupMembersToContacts(groupMembers));
}

GroupMembers Self::groupMembers() const
{
    return m_groupMembers;
}

GroupMembers Self::selectedGroupMembers() const
{
    GroupMembers result;
    for (auto &i : selection()->selectedIndexes()) {
        result.push_back(m_groupMembers[i.row()]);
    }
    return result;
}

void Self::updateGroup(const GroupUpdate &groupUpdate)
{
    if (auto upd = std::get_if<RemoveGroupMembersUpdate>(&groupUpdate)) {
        for (auto &member : upd->members) {
            if (auto index = findByUserId(member->id()); index.isValid()) {
                m_groupMembers.erase(m_groupMembers.begin() + index.row());
                removeContactByRow(index.row());
            }
        }
    }
}

QVariant Self::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case DetailsRole:
        return GroupAffiliationToDisplayString(m_groupMembers[index.row()]->memberAffiliation());

    case SortRole: {
        const auto m = m_groupMembers[index.row()];
        // TODO(fpohtmeh): remove getContact call after GroupMember refactoring
        return QString::number(sortOrder(m)) + getContact(index.row())->displayName();
    }

    default:
        return ContactsModel::data(index, role);
    }
}

int Self::sortOrder(const GroupMemberHandler member)
{
    switch (member->memberAffiliation()) {
    case GroupAffiliation::Owner:
        return 0;
    case GroupAffiliation::Admin:
        return 1;
    default:
        return 2;
    }
}
