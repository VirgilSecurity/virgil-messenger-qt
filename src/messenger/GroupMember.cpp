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

#include "GroupMember.h"

using namespace vm;
using Self = GroupMember;

Self::GroupMember(GroupId groupId, UserId groupOwnerId, UserId memberId, QString memberNickName,
                  GroupAffiliation memberAffiliation)
    : m_groupId(std::move(groupId)),
      m_groupOwnerId(std::move(groupOwnerId)),
      m_memberId(std::move(memberId)),
      m_memberNickName(std::move(memberNickName)),
      m_memberAffiliation(memberAffiliation)
{
}

GroupId Self::groupId() const
{
    return m_groupId;
}

UserId Self::groupOwnerId() const
{
    return m_groupOwnerId;
}

UserId Self::memberId() const
{
    return m_memberId;
}

QString Self::memberNickName() const
{
    return m_memberNickName;
}

GroupAffiliation Self::memberAffiliation() const
{
    return m_memberAffiliation;
}

Contacts vm::GroupMembersToContacts(const GroupMembers &groupMembers)
{
    Contacts contacts;
    for (auto &member : groupMembers) {
        auto contact = std::make_shared<Contact>();
        contact->setUserId(member->memberId());
        contact->setUsername(member->memberId());
        contact->setName(member->memberNickName());
        contacts.push_back(std::move(contact));
    }
    return contacts;
}

GroupMembers vm::ContactsToGroupMembers(const GroupId &groupId, const UserId &groupOwnerId, const Contacts &contacts)
{
    GroupMembers members;
    for (auto &contact : contacts) {
        const auto member = std::make_shared<GroupMember>(groupId, groupOwnerId, contact->userId(), contact->name(),
                                                          GroupAffiliation::Member);
        members.push_back(std::move(member));
    }
    return members;
}

GroupMemberHandler vm::FindGroupMemberById(const GroupMembers &groupMembers, const UserId &memberId)
{
    const auto it = std::find_if(groupMembers.cbegin(), groupMembers.cend(),
                                 [memberId](auto member) { return memberId == member->memberId(); });
    return (it == groupMembers.cend()) ? GroupMemberHandler() : *it;
}

GroupMemberHandler vm::FindGroupOwner(const GroupMembers &groupMembers)
{
    const auto it = std::find_if(groupMembers.cbegin(), groupMembers.cend(),
                                 [](auto member) { return member->memberAffiliation() == GroupAffiliation::Owner; });
    return (it == groupMembers.cend()) ? GroupMemberHandler() : *it;
}
