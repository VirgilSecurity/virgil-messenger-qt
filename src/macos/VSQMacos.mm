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

#ifdef VS_MACOS

#include <QtCore>

#include <Sparkle.h>
#include <SUUpdater.h>

#include "macos/VSQMacos.h"

#include <virgil/iot/logger/logger.h>

/******************************************************************************/
VSQMacos::~VSQMacos() {
    _deleteTimer();
}

/******************************************************************************/
void VSQMacos::_setURL() const {
    NSString *urlStr = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"SUFeedURL"];
    NSURL *appcastURL = [NSURL URLWithString:[urlStr stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    [[SUUpdater sharedUpdater] setFeedURL:appcastURL];
}

/******************************************************************************/
void VSQMacos::checkUpdates() const {
    if (FALSE == [SUUpdater sharedUpdater].updateInProgress) {
        _setURL();
        [[SUUpdater sharedUpdater] checkForUpdates:nil];
    }
}

/******************************************************************************/
void VSQMacos::checkUpdatesBackground() const {
    if (FALSE == [SUUpdater sharedUpdater].updateInProgress) {
        _setURL();
        [[SUUpdater sharedUpdater] checkForUpdatesInBackground];
    }
}

/******************************************************************************/
void VSQMacos::_deleteTimer() {
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
        m_updateTimer = nullptr;
    }
}

/******************************************************************************/
void VSQMacos::startUpdatesTimer() {
    _deleteTimer();
    m_updateTimer = new QTimer();
    m_updateTimer->setSingleShot(false);
    m_updateTimer->setInterval(kUpdateCheckMinutes * 60 * 1000);
    connect(m_updateTimer, &QTimer::timeout, this, &VSQMacos::checkUpdatesBackground);
    m_updateTimer->start();
    checkUpdatesBackground();
}

/******************************************************************************/

#endif // VS_MACOS
