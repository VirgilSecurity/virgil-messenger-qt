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

#include "SetMembersCloudFileOperation.h"

#include "CloudFileOperation.h"
#include "CloudFileSystem.h"
#include "CloudFilesUpdate.h"

using namespace vm;
using Self = SetMembersCloudFileOperation;

Self::SetMembersCloudFileOperation(CloudFileOperation *parent, const CloudFileMembers &members,
                                   const CloudFileHandler &file, const CloudFileHandler &parentFolder)
    : Operation(QLatin1String("ShareCloudFiles"), parent),
      m_parent(parent),
      m_members(members),
      m_file(file),
      m_parentFolder(parentFolder),
      m_requestId(0)
{
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::membersSet, this, &Self::onSet);
    connect(m_parent->cloudFileSystem(), &CloudFileSystem::setMembersErrorOccurred, this, &Self::onSetErrorOccured);
}

void Self::run()
{
    m_requestId = m_parent->cloudFileSystem()->setMembers(m_file, m_members);
}

void Self::onSet(CloudFileRequestId requestId, const CloudFileHandler &file, const CloudFileMembers &members)
{
    if (m_requestId == requestId) {
        ListMembersCloudFileUpdate update;
        update.parentFolder = m_parentFolder;
        update.file = file;
        update.members = members;
        m_parent->cloudFilesUpdate(update);

        finish();
    }
}

void Self::onSetErrorOccured(CloudFileRequestId requestId, const QString &errorText)
{
    if (m_requestId == requestId) {
        failAndNotify(errorText);
    }
}
