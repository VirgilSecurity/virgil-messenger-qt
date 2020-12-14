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

#ifndef VM_VALIDATOR_H
#define VM_VALIDATOR_H

#include <QObject>
#include <QRegularExpressionValidator>

#include "VSQCommon.h"

namespace vm
{
class Validator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRegularExpressionValidator *reUsername MEMBER m_reUsername CONSTANT)
    Q_PROPERTY(QRegularExpressionValidator *rePhone MEMBER m_rePhone CONSTANT)
    Q_PROPERTY(QRegularExpressionValidator *reEmail MEMBER m_reEmail CONSTANT)

public:
    explicit Validator(QObject *parent);
    ~Validator() override;

    // Returns username if it's valid or corrected version of username if it exists
    Optional<QString> validatedUsername(const QString &username, QString *errorText = 0) const;
    bool isValidUsername(const QString &username) const;

private:
    QRegularExpressionValidator *m_reUsername;
    QRegularExpressionValidator *m_rePhone;
    QRegularExpressionValidator *m_reEmail;
};
}

#endif // VM_VALIDATOR_H
