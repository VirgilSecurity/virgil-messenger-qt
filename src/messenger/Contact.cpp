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

#include "Contact.h"

#include "Utils.h"

using namespace vm;
using Self = Contact;

Self::Contact()
    : m_userId(""),
    m_username(""),
    m_name(""),
    m_phone(""),
    m_email(""),
    m_platformId(""),
    m_avatarLocalPath(""),
    m_isBanned(false),
    m_groupAffiliation(GroupAffiliation::None)
{
}

UserId Self::userId() const {
    return m_userId;
}


void Self::setUserId(UserId userId) {
    m_userId = std::move(userId);
}


QString Self::username() const {
    return m_username;
}


void Self::setUsername(QString username) {
    m_username = std::move(username);
}


QString Self::name() const {
    return m_name;
}


void Self::setName(QString name) {
    m_name = std::move(name);
}


QString Self::phone() const {
    return m_phone;
}


void Self::setPhone(QString phone) {
    m_phone = std::move(phone);
}


QString Self::email() const {
    return m_email;
}


void Self::setEmail(QString email) {
    m_email = std::move(email);
}


QString Self::platformId() const {
    return m_platformId;
}


void Self::setPlatformId(QString platformId) {
    m_platformId = std::move(platformId);
}


QString Self::avatarLocalPath() const {
    return m_avatarLocalPath;
}


void Self::setAvatarLocalPath(QString avatarLocalPath) {
    m_avatarLocalPath = std::move(avatarLocalPath);
}


bool Self::isBanned() const {
    return m_isBanned;
}


void Self::setIsBanned(bool isBanned) {
    m_isBanned = isBanned;
}

GroupAffiliation Self::groupAffiliation() const
{
    return m_groupAffiliation;
}

void Self::setGroupAffiliation(GroupAffiliation groupAffiliation)
{
    m_groupAffiliation = groupAffiliation;
}

QString Self::displayName() const {
    return Utils::contactDisplayName(m_name, m_username, m_phone, m_email);
}
