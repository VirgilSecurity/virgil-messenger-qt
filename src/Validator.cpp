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

#include "Validator.h"

using namespace vm;

Validator::Validator(QObject *parent)
    : QObject(parent)
    , m_reUsername(new QRegularExpressionValidator(QRegularExpression(R"(^(?!_)\w{1,20}(?<!_)$)"), this))
    , m_rePhone(new QRegularExpressionValidator(QRegularExpression(R"(^\+?[1-9]\d{3,14}$)"), this))
    , m_reEmail(new QRegularExpressionValidator(QRegularExpression(R"(^\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*$)"), this))
{
}

Validator::~Validator()
{
}

std::optional<QString> Validator::validatedUsername(const QString &username, QString *errorText) const
{
    if (username.isEmpty()) {
        if (errorText) {
            *errorText = QObject::tr("Username can't be empty");
        }
        return {};
    }
    if (!isValidUsername(username)) {
        if (errorText) {
            *errorText = QObject::tr("Username is not valid");
        }
        return {};
    }
    return username.toLower();
}

bool Validator::isValidUsername(const QString &username) const
{
    const auto match = m_reUsername->regularExpression().match(username);
    return match.hasMatch() && !match.hasPartialMatch();
}
