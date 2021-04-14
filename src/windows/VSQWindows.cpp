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

#if VS_WINDOWS

#    include <QtCore>
#    include <winsparkle.h>
#    include "windows/VSQWindows.h"
#    include "VSQCustomer.h"
#    include "CustomerEnv.h"

/******************************************************************************/
VSQWindows::~VSQWindows()
{
    _deleteTimer();
    win_sparkle_cleanup();
}

/******************************************************************************/
void VSQWindows::_WinSparkle_init() const
{
    win_sparkle_set_appcast_url(Customer::kWinSparklURL.toUtf8().constData());
    win_sparkle_set_dsa_pub_pem(reinterpret_cast<const char *>(QResource("://qml/resources/windows/dsa_pub.pem").data()));
    win_sparkle_set_app_details(Customer::ApplicationName.toStdWString().c_str(),
                                Customer::OrganizationDisplayName.toStdWString().c_str(),
                                vm::CustomerEnv::version().toStdWString().c_str());
}

/******************************************************************************/
void VSQWindows::checkUpdates() const
{
    win_sparkle_check_update_with_ui();
}

/******************************************************************************/
void VSQWindows::checkUpdatesBackground() const
{
    win_sparkle_check_update_without_ui();
}

/******************************************************************************/
void VSQWindows::_deleteTimer()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
        m_updateTimer = nullptr;
    }
}

/******************************************************************************/
void VSQWindows::startUpdatesTimer()
{
    _WinSparkle_init();
    _deleteTimer();
    m_updateTimer = new QTimer();
    m_updateTimer->setSingleShot(false);
    m_updateTimer->setInterval(kUpdateCheckMinutes * 60 * 1000);
    connect(m_updateTimer, &QTimer::timeout, this, &VSQWindows::checkUpdatesBackground);
    m_updateTimer->start();
    checkUpdatesBackground();
}

/******************************************************************************/

#endif // VS_WINDOWS
