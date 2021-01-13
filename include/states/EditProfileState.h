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

#ifndef VM_EDITPROFILESTATE_H
#define VM_EDITPROFILESTATE_H

#include "ConfirmationCodeType.h"
#include "OperationState.h"

#include <QUrl>

namespace vm
{
class UsersController;

class EditProfileState : public OperationState
{
    Q_OBJECT
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(bool isPhoneNumberConfirmed READ isPhoneNumberConfirmed WRITE setIsPhoneNumberConfirmed NOTIFY isPhoneNumberConfirmedChanged)
    Q_PROPERTY(bool isEmailConfirmed READ isEmailConfirmed WRITE setIsEmailConfirmed NOTIFY isEmailConfirmedChanged)
    Q_PROPERTY(QUrl avatarUrl READ avatarUrl WRITE setAvatarUrl NOTIFY avatarUrlChanged)

public:
    EditProfileState(UsersController *usersController, QState *parent);

    void processVerificationResponse(const ConfirmationCodeType &codeType, const bool isVerified);

signals:
    void verify(const ConfirmationCodeType &codeType);

    void phoneNumberChanged(const QString &phoneNumber);
    void emailChanged(const QString &email);
    void isPhoneNumberConfirmedChanged(const bool confirmed);
    void isEmailConfirmedChanged(const bool confirmed);
    void avatarUrlChanged(const QUrl &url);

private:
    QString phoneNumber() const;
    void setPhoneNumber(const QString &phoneNumber);
    QString email() const;
    void setEmail(const QString &email);
    bool isPhoneNumberConfirmed() const;
    void setIsPhoneNumberConfirmed(const bool confirmed);
    bool isEmailConfirmed() const;
    void setIsEmailConfirmed(const bool confirmed);
    QUrl avatarUrl() const;
    void setAvatarUrl(const QUrl &avatarUrl);

    UsersController *m_usersController;
    QString m_phoneNumber;
    QString m_email;
    bool m_isPhoneNumberConfirmed;
    bool m_isEmailConfirmed;
    QUrl m_avatarUrl;
};
}

#endif // VM_EDITPROFILESTATE_H
