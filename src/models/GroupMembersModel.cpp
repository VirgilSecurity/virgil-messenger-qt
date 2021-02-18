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

using namespace vm;

void GroupMembersModel::setGroupMembers(const GroupMembers &groupMembers)
{
    setContacts(GroupMembersToContacts(groupMembers));

    // Update isReadOnly & isOwnedByUser
    bool isReadOnly = true;
    bool isOwnedByUser = false;
    if (m_currentUser) {
        const auto it = std::find_if(groupMembers.begin(), groupMembers.end(), [id = m_currentUser->id()](auto m) {
            return m->memberId() == id && (m->memberAffiliation() == GroupAffiliation::Owner || m->memberAffiliation() == GroupAffiliation::Admin);
        });
        if (it != groupMembers.end()) {
            isReadOnly = false;
            isOwnedByUser = (*it)->memberAffiliation() == GroupAffiliation::Owner;
        }
    }

    if (m_isReadOnly != isReadOnly) {
        m_isReadOnly = isReadOnly;
        emit isReadOnlyChanged(isReadOnly);
    }
    if (m_isOwnedByUser != isOwnedByUser) {
        m_isOwnedByUser = isOwnedByUser;
        emit isOwnedByUserChanged(isOwnedByUser);
    }
}

void GroupMembersModel::setCurrentUser(const UserHandler &user)
{
    m_currentUser = user;
}

void GroupMembersModel::updateGroup(const GroupUpdate &groupUpdate)
{
    if (auto upd = std::get_if<RemoveGroupMembersUpdate>(&groupUpdate)) {
        for (auto &member : upd->members) {
            if (auto index = findByUserId(member->id()); index.isValid()) {
                removeContactByRow(index.row());
            }
        }
    }
}

QVariant GroupMembersModel::data(const QModelIndex &index, int role) const
{
    const auto &contact = getContact(index.row());
    switch (role) {
        case DetailsRole:
            return GroupAffiliationToDisplayString(contact->groupAffiliation());

        case SortRole:
            return QString::number(sortOrder(contact)) + contact->displayName();

        default:
            return ContactsModel::data(index, role);
    }
}

int GroupMembersModel::sortOrder(const ContactHandler contact)
{
    switch (contact->groupAffiliation()) {
        case GroupAffiliation::Owner:
            return 0;
        case GroupAffiliation::Admin:
            return 1;
        default:
            return 2;
    }
}
