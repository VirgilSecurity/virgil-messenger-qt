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

#include "IncomingMessageStage.h"

using namespace vm;

IncomingMessageStage vm::IncomingMessageStageFromString(const QString &stageString)
{
    if (stageString == QLatin1String("received")) {
        return IncomingMessageStage::Received;
    } else if (stageString == QLatin1String("decrypted")) {
        return IncomingMessageStage::Decrypted;
    } else if (stageString == QLatin1String("read")) {
        return IncomingMessageStage::Read;
    } else if (stageString == QLatin1String("broken")) {
        return IncomingMessageStage::Broken;
    } else {
        throw std::logic_error("Invalid IncomingMessageStage string: " + stageString.toStdString());
    }
}

QString vm::IncomingMessageStageToString(IncomingMessageStage stage)
{
    switch (stage) {
    case IncomingMessageStage::Received:
        return QLatin1String("received");

    case IncomingMessageStage::Decrypted:
        return QLatin1String("decrypted");

    case IncomingMessageStage::Read:
        return QLatin1String("read");

    case IncomingMessageStage::Broken:
        return QLatin1String("broken");

    default:
        throw std::logic_error("Invalid IncomingMessageStage");
    }
}

MessageStatus vm::IncomingMessageStageToMessageStatus(IncomingMessageStage stage)
{
    switch (stage) {
    case IncomingMessageStage::Received:
        return MessageStatus::New;

    case IncomingMessageStage::Decrypted:
    case IncomingMessageStage::Read:
        return MessageStatus::Succeed;

    case IncomingMessageStage::Broken:
        return MessageStatus::Broken;

    default:
        throw std::logic_error("Invalid IncomingMessageStage");
    }
}
