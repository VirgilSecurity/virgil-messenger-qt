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

#include <algorithm>

using namespace vm;
using Self = CloudFileMember;

Self::CloudFileMember() : m_memberId(), m_type(Type::Member), m_contact() { }

Self::CloudFileMember(UserId memberId, Type type) : m_memberId(std::move(memberId)), m_type(type), m_contact() { }

Self::CloudFileMember(ContactHandler contact, Type type)
    : m_memberId(contact->userId()), m_type(type), m_contact(std::move(contact))
{
}

UserId Self::memberId() const
{
    return m_memberId;
}

Self::Type Self::type() const
{
    return m_type;
}

QString Self::typeAsDisplayString() const
{
    switch (m_type) {
    case Self::Type::Owner:
        return QObject::tr("Owner");

    case Self::Type::Member:
        return QObject::tr("Member");

    default:
        throw std::logic_error("Invalid CloudFileMember type");
    }
}

ContactHandler Self::contact() const
{
    return m_contact;
}

CloudFileMember Self::cloneWithContact(ContactHandler contact) const
{
    auto self = *this;
    self.m_contact = std::move(contact);
    return self;
}

CloudFileMembers vm::ContactsToCloudFileMembers(const Contacts &contacts)
{
    CloudFileMembers members;
    std::transform(contacts.cbegin(), contacts.cend(), std::back_inserter(members), [](const auto &contact) {
        return std::make_unique<CloudFileMember>(contact, CloudFileMember::Type::Member);
    });
    return members;
}

Contacts vm::CloudFileMembersToContacts(const CloudFileMembers &members)
{
    Contacts contacts;
    std::transform(members.cbegin(), members.cend(), std::back_inserter(contacts),
                   [](const auto &member) { return member->contact(); });
    return contacts;
}
