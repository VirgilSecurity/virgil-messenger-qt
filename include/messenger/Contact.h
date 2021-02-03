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

#ifndef VM_CONTACT_H
#define VM_CONTACT_H

#include "UserId.h"

#include <QString>

#include <vector>

namespace vm
{
class Contact
{
public:
    //
    //  Initialize all string members with empty string.
    //
    Contact();

    UserId userId() const;

    void setUserId(UserId userId);

    QString username() const;

    void setUsername(QString username);

    QString name() const;

    void setName(QString name);

    QString phone() const;

    void setPhone(QString phone);

    QString email() const;

    void setEmail(QString email);

    QString platformId() const;

    void setPlatformId(QString platformId);

    QString avatarLocalPath() const;

    void setAvatarLocalPath(QString avatarLocalPath);

    bool isBanned() const;

    void setIsBanned(bool isBanned);

private:
    UserId m_userId;
    QString m_username;
    QString m_name;
    QString m_phone;
    QString m_email;
    QString m_platformId;
    QString m_avatarLocalPath;
    bool m_isBanned;
};

using Contacts = std::vector<Contact>;
}

#endif // VM_CONTACT_H
