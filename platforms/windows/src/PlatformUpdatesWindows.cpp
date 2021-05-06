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

#include "PlatformUpdatesWindows.h"

#include <VSQCustomer.h>

#include <winsparkle.h>

#include <QResource>

using namespace vm;
using namespace platform;

using Self = PlatformUpdatesWindows;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#if defined(VERSION)
static constexpr const auto kVersion = TOSTRING(VERSION) "-alpha";
#else
static constexpr const auto kVersion = "unknown";
#endif

PlatformUpdates& PlatformUpdates::instance()
{
    static Self impl;
    return impl;
}

static void checkUpdatesBackground() {
    win_sparkle_check_update_without_ui();
}

Self::PlatformUpdatesWindows()
{
    //
    //  Configure updates feed URL.
    //
    win_sparkle_set_appcast_url(Customer::kWinSparklURL.toUtf8().constData());
    win_sparkle_set_dsa_pub_pem(
            reinterpret_cast<const char *>(QResource(":qml/resources/windows/dsa_pub.pem").data()));
    win_sparkle_set_app_details(Customer::ApplicationName.toStdWString().c_str(),
                                Customer::OrganizationDisplayName.toStdWString().c_str(),
                                QString(kVersion).toStdWString().c_str());

    //
    //  Configure updates timer.
    //
    m_updateTimer.setSingleShot(false);
    m_updateTimer.setInterval(Self::kUpdateCheckMinutes * 60 * 1000);
    QObject::connect(&m_updateTimer, &QTimer::timeout, checkUpdatesBackground);
}

Self::~PlatformUpdatesWindows() noexcept
{
    win_sparkle_cleanup();
}

bool Self::isSupported() const noexcept
{
    return true;
}

void Self::startChecking()
{
    m_updateTimer.start();

    checkUpdatesBackground();
}

void Self::checkNow() const
{
    win_sparkle_check_update_with_ui();
}
