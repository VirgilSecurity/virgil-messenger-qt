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

#include <algorithm>

using namespace vm;
using Self = GroupMember;

Self::GroupMember(GroupId groupId, UserId memberId, GroupAffiliation memberAffiliation)
    : m_groupId(std::move(groupId)),
      m_memberId(std::move(memberId)),
      m_memberNickName(m_memberId),
      m_memberAffiliation(memberAffiliation),
      m_contact()
{
}

Self::GroupMember(GroupId groupId, ContactHandler contact, GroupAffiliation memberAffiliation)
    : m_groupId(std::move(groupId)),
      m_memberId(contact->userId()),
      m_memberNickName(m_memberId),
      m_memberAffiliation(memberAffiliation),
      m_contact(std::move(contact))
{
}

GroupId Self::groupId() const
{
    return m_groupId;
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

ContactHandler Self::contact() const
{
    return m_contact;
}

Contacts vm::GroupMembersToContacts(const GroupMembers &groupMembers)
{
    Contacts contacts;
    std::transform(groupMembers.cbegin(), groupMembers.cend(), std::back_inserter(contacts),
                   [](const auto &member) -> ContactHandler {
                       if (auto contact = member->contact()) {
                           return contact;
                       }

                       auto contact = std::make_shared<Contact>();
                       contact->setUserId(member->memberId());
                       contact->setUsername(member->memberId());
                       contact->setName(member->memberNickName());

                       return contact;
                   });

    return contacts;
}

GroupMembers vm::ContactsToGroupMembers(const GroupId &groupId, const Contacts &contacts)
{
    GroupMembers members;
    std::transform(contacts.cbegin(), contacts.cend(), std::back_inserter(members),
                   [&groupId](const auto &contact) -> GroupMemberHandler {
                       return std::make_shared<GroupMember>(groupId, contact, GroupAffiliation::Member);
                   });
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
