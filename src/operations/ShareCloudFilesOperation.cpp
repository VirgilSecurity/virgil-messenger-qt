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

#include "ShareCloudFilesOperation.h"

#include "CloudFileOperation.h"

using namespace vm;
using Self = ShareCloudFilesOperation;

Self::ShareCloudFilesOperation(CloudFileOperation *parent, const CloudFiles &files)
    : Operation(QLatin1String("ShareCloudFiles"), parent), m_parent(parent), m_files(files)
{
}

void Self::addMembers(const CloudFileMembers &members)
{
    m_addedMembers.insert(m_addedMembers.end(), members.begin(), members.end());
}

void Self::removeMembers(const CloudFileMembers &members)
{
    m_removedMembers.insert(m_removedMembers.end(), members.begin(), members.end());
}

void Self::run()
{
    if (m_addedMembers.empty() && m_removedMembers.empty()) {
        throw std::logic_error("ShareCloudFileOperation members are empty");
    }

    if (!m_addedMembers.empty()) {
        failAndNotify(tr("Add cloud file members is under development"));
    }
    if (!m_removedMembers.empty()) {
        failAndNotify(tr("Remove cloud file members is under development"));
    }
}
