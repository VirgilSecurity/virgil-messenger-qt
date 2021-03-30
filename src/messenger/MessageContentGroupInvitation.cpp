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

#include "MessageContentGroupInvitation.h"

using namespace vm;
using Self = MessageContentGroupInvitation;

constexpr static const auto kJsonKey_Owner = "owner";
constexpr static const auto kJsonKey_Tile = "title";
constexpr static const auto kJsonKey_HelloText = "greetings";
constexpr static const auto kJsonKey_InvitationStatus = "invitationStatus";

Self::MessageContentGroupInvitation(UserId superOwnerId, QString title, QString helloText)
    : m_superOwnerId(std::move(superOwnerId)),
      m_title(std::move(title)),
      m_helloText(std::move(helloText)),
      m_invitationStatus(GroupInvitationStatus::None)
{
}

Self::MessageContentGroupInvitation(UserId superOwnerId, QString title, GroupInvitationStatus invitationStatus,
                                    QString helloText)
    : m_superOwnerId(std::move(superOwnerId)),
      m_title(std::move(title)),
      m_helloText(std::move(helloText)),
      m_invitationStatus(invitationStatus)
{
}

UserId Self::superOwnerId() const
{
    return m_superOwnerId;
}

QString Self::title() const
{
    return m_title;
}

QString Self::helloText() const
{
    return m_helloText;
}

GroupInvitationStatus Self::invitationStatus() const
{
    return m_invitationStatus;
}

Self Self::newInvitationStatus(GroupInvitationStatus invitationStatus) const
{
    Self self = *this;
    self.m_invitationStatus = invitationStatus;
    return self;
}

void Self::writeJson(QJsonObject &json) const
{
    json[kJsonKey_Owner] = QString(m_superOwnerId);
    json[kJsonKey_Tile] = m_title;
    json[kJsonKey_HelloText] = m_helloText;
    json[kJsonKey_InvitationStatus] = GroupInvitationStatusToString(m_invitationStatus);
}

bool Self::readJson(const QJsonObject &json)
{
    auto ownerValue = json[kJsonKey_Owner];
    auto titleValue = json[kJsonKey_Tile];
    auto helloTextValue = json[kJsonKey_HelloText];
    auto invitationStatusValue = json[kJsonKey_InvitationStatus];

    if (!ownerValue.isString() || !titleValue.isString()) {
        return false;
    }

    m_superOwnerId = UserId(ownerValue.toString());
    m_title = titleValue.toString();
    m_helloText = helloTextValue.toString();

    if (!m_superOwnerId.isValid()) {
        return false;
    }

    if (invitationStatusValue.isString()) {
        const auto invitationStatus = invitationStatusValue.toString();
        if (!invitationStatus.isEmpty()) {
            m_invitationStatus = GroupInvitationStatusFromString(invitationStatus);
        }
    }

    return true;
}
