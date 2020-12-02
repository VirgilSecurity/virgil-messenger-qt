//  Copyright (C) 2015-2020 Virgil Security, Inc.
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

#include "states/EditProfileState.h"

#include "VSQMessenger.h"

using namespace vm;

EditProfileState::EditProfileState(QState *parent)
    : OperationState(parent)
{
    m_phoneNumber = "+380123456789";
    m_isPhoneNumberConfirmed = true;
    m_isEmailConfirmed = false;
}

QString EditProfileState::userId() const
{
    return m_userId;
}

void EditProfileState::setUserId(const QString &userId)
{
    if (m_userId == userId) {
        return;
    }
    m_userId = userId;
    emit userIdChanged(userId);
}

QString EditProfileState::phoneNumber() const
{
    return m_phoneNumber;
}

void EditProfileState::setPhoneNumber(const QString &phoneNumber)
{
    if (m_phoneNumber == phoneNumber) {
        return;
    }
    m_phoneNumber = phoneNumber;
    emit phoneNumberChanged(phoneNumber);
}

QString EditProfileState::email() const
{
    return m_email;
}

void EditProfileState::setEmail(const QString &email)
{
    if (m_email == email) {
        return;
    }
    m_email = email;
    emit emailChanged(email);
}

bool EditProfileState::isPhoneNumberConfirmed() const
{
    return m_isPhoneNumberConfirmed;
}

void EditProfileState::setIsPhoneNumberConfirmed(const bool &isPhoneNumberConfirmed)
{
    if (m_isPhoneNumberConfirmed == isPhoneNumberConfirmed) {
        return;
    }
    m_isPhoneNumberConfirmed = isPhoneNumberConfirmed;
    emit isPhoneNumberConfirmedChanged(isPhoneNumberConfirmed);
}

bool EditProfileState::isEmailConfirmed() const
{
    return m_isEmailConfirmed;
}

void EditProfileState::setIsEmailConfirmed(const bool &isEmailConfirmed)
{
    if (m_isEmailConfirmed == isEmailConfirmed) {
        return;
    }
    m_isEmailConfirmed = isEmailConfirmed;
    emit isEmailConfirmedChanged(isEmailConfirmed);
}

QUrl EditProfileState::avatarUrl() const
{
    return m_avatarUrl;
}

void EditProfileState::setAvatarUrl(const QUrl &avatarUrl)
{
    if (m_avatarUrl == avatarUrl) {
        return;
    }
    m_avatarUrl = avatarUrl;
    emit avatarUrlChanged(avatarUrl);
}

