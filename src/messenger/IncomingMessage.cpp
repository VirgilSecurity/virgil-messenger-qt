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

#include "IncomingMessage.h"

using namespace vm;
using Self = IncomingMessage;

Self::Stage Self::stageFromString(const QString &stageString)
{
    return IncomingMessageStageFromString(stageString);
}

QString Self::stageToString(Self::Stage stage)
{
    return IncomingMessageStageToString(stage);
}

bool Self::isIncoming() const noexcept
{
    return true;
}

Self::Status Self::status() const noexcept
{
    return IncomingMessageStageToMessageStatus(m_stage);
}

Self::Stage Self::stage() const noexcept
{
    return m_stage;
}

void Self::setStage(Self::Stage stage)
{
    m_stage = stage;
}

QString Self::stageString() const
{
    return Self::stageToString(m_stage);
}

void Self::setStageString(QString stageString)
{
    m_stage = Self::stageFromString(stageString);
}

bool Self::applyUpdate(const MessageUpdate &update)
{
    if (auto stageUpdate = std::get_if<IncomingMessageStageUpdate>(&update)) {
        m_stage = stageUpdate->stage;
        return true;
    }
    return Message::applyUpdate(update);
}
