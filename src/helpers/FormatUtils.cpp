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

#include "FormatUtils.h"

#include <QObject>

using namespace vm;
using Self = vm::FormatUtils;

QString Self::formattedSize(quint64 fileSize)
{
    return QLocale::system().formattedDataSize(fileSize);
}

QString Self::formattedDataSizeProgress(quint64 loaded, quint64 total)
{
    if (loaded <= 0) {
        return QLatin1String("...");
    }
    // Get power for total. Based on QLocale::formattedDataSize
    int power = !total ? 0 : int((63 - qCountLeadingZeroBits(quint64(qAbs(total)))) / 10);
    // Get number for loaded
    const int base = 1024;
    const int precision = 2;
    const auto locale = QLocale::system();
    const auto formattedLoaded = power
            ? locale.toString(loaded / std::pow(double(base), power), 'f', qMin(precision, 3 * power))
            : locale.toString(loaded);
    return formattedLoaded + QLatin1String(" / ") + formattedSize(total);
}

QString Self::formattedElapsedSeconds(std::chrono::seconds seconds, std::chrono::seconds nowInterval)
{
    if (seconds < nowInterval) {
        return QObject::tr("now");
    }
    if (seconds <= 3 * nowInterval) {
        return QObject::tr("few seconds ago");
    }
    const std::chrono::seconds minute(60);
    if (seconds < minute) {
        return QObject::tr("recently");
    }
    if (seconds < 2 * minute) {
        return QObject::tr("a minute ago");
    }
    if (seconds < 50 * minute) {
        return QObject::tr("%1 minutes ago").arg(seconds / minute);
    }
    const std::chrono::seconds hour(60 * minute);
    if (seconds < 2 * hour) {
        return QObject::tr("an hour ago");
    }
    const std::chrono::seconds day(24 * hour);
    if (seconds < day) {
        return QObject::tr("%1 hours ago").arg(seconds / hour);
    }
    if (seconds < 2 * day) {
        return QObject::tr("yesterday");
    }
    const std::chrono::seconds month(30 * day);
    if (seconds < month) {
        return QObject::tr("%1 days ago").arg(seconds / day);
    }
    const std::chrono::seconds year(12 * month);
    if (seconds < year) {
        return QObject::tr("%1 months ago").arg(seconds / month);
    }
    if (seconds < 2 * year) {
        return QObject::tr("year ago");
    }
    return QObject::tr("%1 years ago").arg(seconds / year);
}

QString Self::formattedLastSeenActivity(std::chrono::seconds seconds, std::chrono::seconds updateInterval)
{
    if (seconds < updateInterval) {
        return QObject::tr("Online");
    }

    return QObject::tr("Last seen %1").arg(formattedElapsedSeconds(seconds, updateInterval));
}

QString Self::formattedLastSeenNoActivity()
{
    return QObject::tr("Offline");
}
