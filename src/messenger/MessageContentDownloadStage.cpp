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

#include "MessageContentDownloadStage.h"

using namespace vm;

MessageContentDownloadStage vm::MessageContentDownloadStageFromString(const QString &stageString)
{
    if (stageString == QLatin1String("initial")) {
        return MessageContentDownloadStage::Initial;
    } else if (stageString == QLatin1String("preloading")) {
        return MessageContentDownloadStage::Preloading;
    } else if (stageString == QLatin1String("preloaded")) {
        return MessageContentDownloadStage::Preloaded;
    } else if (stageString == QLatin1String("downloading")) {
        return MessageContentDownloadStage::Downloading;
    } else if (stageString == QLatin1String("downloaded")) {
        return MessageContentDownloadStage::Downloaded;
    } else if (stageString == QLatin1String("decrypted")) {
        return MessageContentDownloadStage::Decrypted;
    } else {
        throw std::logic_error("Invalid MessageContentDownloadStage string");
    }
}

QString vm::MessageContentDownloadStageToString(MessageContentDownloadStage stage)
{
    switch (stage) {
    case MessageContentDownloadStage::Initial:
        return QLatin1String("initial");

    case MessageContentDownloadStage::Preloading:
        return QLatin1String("preloading");

    case MessageContentDownloadStage::Preloaded:
        return QLatin1String("preloaded");

    case MessageContentDownloadStage::Downloading:
        return QLatin1String("downloading");

    case MessageContentDownloadStage::Downloaded:
        return QLatin1String("downloaded");

    case MessageContentDownloadStage::Decrypted:
        return QLatin1String("decrypted");

    default:
        throw std::logic_error("Invalid MessageContentDownloadStage");
    }
}
