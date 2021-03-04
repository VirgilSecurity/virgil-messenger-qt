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

#ifndef VM_CLOUD_FILE_OBJECT_H
#define VM_CLOUD_FILE_OBJECT_H

#include <QObject>
#include <QPointer>

#include "CloudFile.h"
#include "CloudFileMembersModel.h"
#include "CloudFilePropertiesModel.h"
#include "CloudFilesUpdate.h"

namespace vm {
class Messenger;

class CloudFileObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool isFolder READ isFolder NOTIFY isFolderChanged)
    Q_PROPERTY(bool isShared READ isShared NOTIFY isSharedChanged)
    Q_PROPERTY(CloudFilePropertiesModel *properties MEMBER m_propertiesModel CONSTANT)
    Q_PROPERTY(CloudFileMembersModel *members MEMBER m_membersModel CONSTANT)
    Q_PROPERTY(bool userIsOwner MEMBER m_userIsOwner NOTIFY userIsOwnerChanged)

public:
    CloudFileObject(Messenger *messenger, QObject *parent);

    void setCloudFile(const CloudFileHandler &cloudFile);
    CloudFileHandler cloudFile() const;

    QString name() const;
    bool isFolder() const;
    bool isShared() const;

    void setMembers(const CloudFileMembers &members);
    CloudFileMembers members() const;
    CloudFileMembers selectedMembers() const;
    CloudFileMemberHandler findMemberById(const UserId &userId) const;

    void updateCloudFiles(const CloudFilesUpdate &update);

signals:
    void nameChanged(const QString &name);
    void isFolderChanged(bool isFolder);
    void isSharedChanged(bool isShared);

    void userIsOwnerChanged(bool isOwner);

private:
    QPointer<Messenger> m_messenger;
    CloudFileHandler m_cloudFile;
    CloudFilePropertiesModel *m_propertiesModel;
    CloudFileMembersModel *m_membersModel;
    bool m_userIsOwner = false;
};

} // namespace vm

#endif // VM_CLOUD_FILE_OBJECT_H
