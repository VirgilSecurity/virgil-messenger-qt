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

#include "GroupAffiliation.h"

#include <stdexcept>

#include <QObject>

using namespace vm;

GroupAffiliation vm::GroupAffiliationFromString(const QString &affiliationString)
{
    if (affiliationString == QLatin1String("none")) {
        return GroupAffiliation::None;
    } else if (affiliationString == QLatin1String("outcast")) {
        return GroupAffiliation::Outcast;
    } else if (affiliationString == QLatin1String("member")) {
        return GroupAffiliation::Member;
    } else if (affiliationString == QLatin1String("admin")) {
        return GroupAffiliation::Admin;
    } else if (affiliationString == QLatin1String("owner")) {
        return GroupAffiliation::Owner;
    } else {
        throw std::logic_error("Invalid GroupAffiliation string");
    }
}

QString vm::GroupAffiliationToString(GroupAffiliation affiliation)
{
    switch (affiliation) {
    case GroupAffiliation::None:
        return QLatin1String("none");

    case GroupAffiliation::Outcast:
        return QLatin1String("outcast");

    case GroupAffiliation::Member:
        return QLatin1String("member");

    case GroupAffiliation::Admin:
        return QLatin1String("admin");

    case GroupAffiliation::Owner:
        return QLatin1String("owner");

    default:
        throw std::logic_error("Invalid GroupAffiliation");
    }
}

QString vm::GroupAffiliationToDisplayString(GroupAffiliation affiliation)
{
    switch (affiliation) {
    case GroupAffiliation::None:
        return QObject::tr("none");

    case GroupAffiliation::Outcast:
        return QObject::tr("outcast");

    case GroupAffiliation::Member:
        return QObject::tr("member");

    case GroupAffiliation::Admin:
        return QObject::tr("admin");

    case GroupAffiliation::Owner:
        return QObject::tr("owner");

    default:
        throw std::logic_error("Invalid GroupAffiliation");
    }
}
