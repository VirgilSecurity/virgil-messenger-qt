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

#include "TimeProfilerSection.h"

#include "TimeProfiler.h"

using namespace vm;
using Self = TimeProfilerSection;

TimeProfilerSection::TimeProfilerSection(const QString &name, TimeProfiler *profiler)
    : m_preffix(QString("[%1] ").arg(name)), m_profiler(profiler), m_initialElapsed(profiler ? profiler->elapsed() : 0)
{
    if (profiler) {
        profiler->printMessageWithOptions(m_preffix + QLatin1String("Section started"), 0);
    }
}

TimeProfilerSection::~TimeProfilerSection()
{
    printMessage(QLatin1String("Section ended"));
}

void Self::printMessage(const QString &message)
{
    if (m_profiler) {
        m_profiler->printMessageWithOptions(m_preffix + message, elapsed());
    }
}

qint64 TimeProfilerSection::elapsed() const
{
    return m_profiler ? (m_profiler->elapsed() - m_initialElapsed) : 0;
}
