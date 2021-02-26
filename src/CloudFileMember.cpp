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

#include "CloudFileMember.h"

#include <QObject>

using namespace vm;
using Self = CloudFileMember;

Self::CloudFileMember() : m_type(Type::Member) { }

Self::CloudFileMember(const ContactHandler &contact, const Type type) : m_contact(contact), m_type(type) { }

ContactHandler Self::contact() const
{
    return m_contact;
}

CloudFileMember::Type Self::type() const
{
    return m_type;
}

CloudFileMembers vm::ContactsToCloudFileMembers(const Contacts &contacts)
{
    CloudFileMembers members;
    for (auto &c : contacts) {
        members.push_back(std::make_shared<CloudFileMember>(c, CloudFileMember::Type::Member));
    }
    return members;
}

Contacts vm::CloudFileMembersToContacts(const CloudFileMembers &members)
{
    Contacts contacts;
    for (auto &m : members) {
        contacts.push_back(m->contact());
    }
    return contacts;
}

QString vm::CloudFileMemberTypeToDisplayString(const CloudFileMember::Type type)
{
    switch (type) {
    case CloudFileMember::Type::Owner:
        return QObject::tr("Owner");
    case CloudFileMember::Type::Member:
        return QObject::tr("Member");
    default:
        throw std::logic_error("Invalid CloudFileMember type");
    }
}

CloudFileMemberHandler vm::FindCloudFileMemberById(const CloudFileMembers &members, const UserId &memberId)
{
    const auto it = std::find_if(members.cbegin(), members.cend(),
                                 [memberId](auto member) { return memberId == member->contact()->userId(); });
    return (it == members.cend()) ? CloudFileMemberHandler() : *it;
}
