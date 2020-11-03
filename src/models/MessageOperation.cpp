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

#include "models/MessageOperation.h"

#include "models/SendMessageOperation.h"

using namespace vm;

MessageOperation::MessageOperation(const GlobalMessage &message, QObject *parent)
    : Operation(QString("Message [%1]").arg(message.id), parent)
    , m_message(message)
{}

const GlobalMessage *MessageOperation::message() const
{
    return &m_message;
}

void MessageOperation::connectChild(Operation *child)
{
    Operation::connectChild(child);
    connect(child, &Operation::failed, this, std::bind(&MessageOperation::setStatus, this, Message::Status::Failed));
    connect(child, &Operation::invalidated, this, std::bind(&MessageOperation::setStatus, this, Message::Status::InvalidM));
    connect(child, &Operation::finished, this, std::bind(&MessageOperation::onChildFinished, this, child));
}

void MessageOperation::setStatus(const Message::Status &status)
{
    if (m_message.status == status) {
        return;
    }
    m_message.status = status;
    emit statusChanged(status);
}

void MessageOperation::onChildFinished(const Operation *child)
{
    if (dynamic_cast<const SendMessageOperation *>(child)) {
        setStatus(Message::Status::Sent);
    }
}
