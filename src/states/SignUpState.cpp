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

#include "states/SignUpState.h"

#include "Validator.h"
#include "VSQMessenger.h"

using namespace VSQ;

SignUpState::SignUpState(VSQMessenger *messenger, Validator *validator, QState *parent)
    : OperationState(parent), m_messenger(messenger), m_validator(validator) {
    connect(m_messenger, &VSQMessenger::signedUp, this, &SignUpState::operationFinished);
    connect(m_messenger, &VSQMessenger::signUpErrorOccured, this, &SignUpState::operationErrorOccurred);
    connect(m_messenger, &VSQMessenger::signedUp, this, &SignUpState::signedUp);
    connect(this, &SignUpState::signUp, this, &SignUpState::processSignUp);
}

void
SignUpState::processSignUp(const QString &username) {
    QString errorText;
    const auto validUsername = m_validator->validatedUsername(username);

    if (!validUsername) {
        emit operationStarted();
        emit operationErrorOccurred(errorText);
    } else {
        if (m_userId != *validUsername) {
            m_userId = *validUsername;
            emit userIdChanged(m_userId);
        }
        emit operationStarted();
        m_messenger->signUp(m_userId);
    }
}