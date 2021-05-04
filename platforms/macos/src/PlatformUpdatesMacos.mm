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

#include "PlatformUpdatesMacos.h"

#include <SUUpdater.h>
#include <Sparkle.h>

using namespace vm;
using namespace platform;

using Self = PlatformUpdatesMacos;

PlatformUpdates& PlatformUpdates::instance()
{
    static Self impl;
    return impl;
}

static void checkUpdatesBackground()
{
    if (NO == [SUUpdater sharedUpdater].updateInProgress) {
        [[SUUpdater sharedUpdater] checkForUpdatesInBackground];
    }
}

Self::PlatformUpdatesMacos()
{
    //
    //  Configure updates feed URL.
    //
    NSString* urlStr = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"SUFeedURL"];
    NSURL* appcastURL = [NSURL URLWithString:urlStr];
    [[SUUpdater sharedUpdater] setFeedURL:appcastURL];

    //
    //  Configure updates timer.
    //
    m_updateTimer.setSingleShot(false);
    m_updateTimer.setInterval(Self::kUpdateCheckMinutes * 60 * 1000);
    QObject::connect(&m_updateTimer, &QTimer::timeout, checkUpdatesBackground);
}

bool Self::isSupported() const noexcept { return true; }

void Self::startChecking()
{
    m_updateTimer.start();

    checkUpdatesBackground();
}

void Self::checkNow() const
{
    if (NO == [SUUpdater sharedUpdater].updateInProgress) {
        [[SUUpdater sharedUpdater] checkForUpdates:nil];
    }
}
