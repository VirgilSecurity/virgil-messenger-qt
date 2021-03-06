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

#include "CloudFileObject.h"

#include "Messenger.h"

#include <algorithm>

using namespace vm;
using Self = CloudFileObject;

Self::CloudFileObject(Messenger *messenger, QObject *parent)
    : QObject(parent),
      m_messenger(messenger),
      m_propertiesModel(new CloudFilePropertiesModel(this)),
      m_membersModel(new CloudFileMembersModel(messenger, this))
{
}

void Self::setCloudFile(const CloudFileHandler &cloudFile)
{
    const auto oldName = name();
    const auto wasFolder = isFolder();
    const auto wasShared = isShared();

    m_cloudFile = cloudFile;
    m_propertiesModel->setCloudFile(cloudFile);

    if (oldName != name()) {
        emit nameChanged(name());
    }
    if (wasFolder != isFolder()) {
        emit isFolderChanged(isFolder());
    }
    if (wasShared != isShared()) {
        emit isSharedChanged(isShared());
    }
}

CloudFileHandler Self::cloudFile() const
{
    return m_cloudFile;
}

QString Self::name() const
{
    return m_cloudFile ? m_cloudFile->name() : QString();
}

bool Self::isFolder() const
{
    return m_cloudFile && m_cloudFile->isFolder();
}

bool Self::isShared() const
{
    return m_cloudFile && m_cloudFile->isShared();
}

void Self::setMembers(const CloudFileMembers &members)
{
    m_membersModel->setMembers(members);

    const auto userMember = findMemberById(m_messenger->currentUser()->id());
    const bool userIsOwner = userMember && userMember->type() == CloudFileMember::Type::Owner;
    if (m_userIsOwner != userIsOwner) {
        m_userIsOwner = userIsOwner;
        emit userIsOwnerChanged(userIsOwner);
    }
}

CloudFileMembers Self::members() const
{
    return m_membersModel->members();
}

CloudFileMembers Self::selectedMembers() const
{
    return m_membersModel->selectedMembers();
}

CloudFileMemberHandler Self::findMemberById(const UserId &userId) const
{
    return m_membersModel->findMemberById(userId);
}

void Self::updateCloudFiles(const CloudFilesUpdate &update)
{
    if (std::holds_alternative<CachedListCloudFolderUpdate>(update)
        || std::holds_alternative<CloudListCloudFolderUpdate>(update)
        || std::holds_alternative<CreateCloudFilesUpdate>(update)
        || std::holds_alternative<DeleteCloudFilesUpdate>(update)
        || std::holds_alternative<TransferCloudFileUpdate>(update)
        || std::holds_alternative<DownloadCloudFileUpdate>(update)) {
        return;
    } else if (auto upd = std::get_if<ListMembersCloudFileUpdate>(&update)) {
        setMembers(upd->members);
    } else {
        throw std::logic_error("Invalid CloudFilesUpdate in CloudFileObject::updateCloudFiles");
    }
}
