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

#ifndef VM_CORE_MESSENGER_CLOUD_FS_H
#define VM_CORE_MESSENGER_CLOUD_FS_H

#include "CloudFsFileDownloadInfo.h"
#include "CloudFsFileId.h"
#include "CloudFsFileInfo.h"
#include "CloudFsFolder.h"
#include "CloudFsFolderId.h"
#include "CloudFsFolderInfo.h"
#include "CloudFsNewFile.h"
#include "CloudFileMember.h"
#include "CoreMessengerStatus.h"
#include "User.h"

#include <QObject>
#include <QFuture>
#include <QFileInfo>
#include <QPointer>

#include <memory>
#include <variant>

//
//  Forward C struct declarations.
//
struct vssq_messenger_cloud_fs_t;
struct vscf_impl_t;

namespace vm {

class CoreMessenger;

//
//  Sync files and folders with the Cloud FS.
//
class CoreMessengerCloudFs
{
public:
    template<typename T>
    using Result = std::variant<CoreMessengerStatus, T>;

    template<typename T>
    using FutureResult = QFuture<Result<T>>;

    using FutureStatus = QFuture<CoreMessengerStatus>;

    using vssq_messenger_cloud_fs_ptr_t =
            std::unique_ptr<const vssq_messenger_cloud_fs_t, void (*)(const vssq_messenger_cloud_fs_t *)>;

    using vscf_impl_ptr_t = std::unique_ptr<vscf_impl_t, void (*)(const vscf_impl_t *)>;

    using vscf_impl_ptr_const_t = std::unique_ptr<const vscf_impl_t, void (*)(const vscf_impl_t *)>;

public:
    //
    //  Constructor handles C context from the CommKit.
    //
    CoreMessengerCloudFs(vssq_messenger_cloud_fs_ptr_t cloudFs, vscf_impl_ptr_t random,
                         const CoreMessenger *coreMessenger);

    //
    //  Encrypt given file and then request remote server for it's uploading URL.
    //  Note, file created in given folder.
    //
    FutureResult<CloudFsNewFile> createFile(const QString &sourceFilePath, const QString &destFilePath,
                                            const CloudFsFolderId &parentFolderId,
                                            const QByteArray &parentFolderPublicKey);

    //
    //  Delete file with a given identifier.
    //
    FutureStatus deleteFile(const CloudFsFileId &fileId);

    //
    //  Request a file download link and it's encrypted private key for decryption.
    //
    FutureResult<CloudFsFileDownloadInfo> getFileDownloadInfo(const CloudFsFileId &fileId) const;

    //
    //  Create a new folder in the given folder.
    //  Note, folder is shared if members are specified.
    //
    FutureResult<CloudFsFolder> createFolder(const QString &folderName, const CloudFileMembers &members,
                                             const CloudFsFolderId &parentFolderId,
                                             const QByteArray &parentFolderPublicKey);

    //
    //  Delete folder with a given identifier.
    //
    FutureStatus deleteFolder(const CloudFsFolderId &folderId);

    //
    //  Request a list of folder entries.
    //
    FutureResult<CloudFsFolder> listFolder(const CloudFsFolderId &folderId);

    //
    //  Decrypt given file owned by me.
    //
    CoreMessengerStatus decryptFile(const QString &sourceFilePath, const QString &destFilePath,
                                    const QByteArray &encryptedFileKey, const UserHandler &sender,
                                    const CloudFsFolder &parentFolder);

private:
    //
    //  Encrypt given file and returns a key for decryption.
    //
    Result<QByteArray> encryptFile(const QString &sourceFilePath, const QString &destFilePath);

    //
    //  Decrypt file/folder key by current user key and verify it's issuer signature.
    //
    Result<QByteArray> decryptKey(const QByteArray &encryptedKey, const UserHandler &issuer) const;

    //
    //  Decrypt file decryption key and check it's owners signature.
    //
    Result<QByteArray> decryptFileKey(const QByteArray &fileEncryptedKey, const UserHandler &sender,
                                      const CloudFsFolder &parentFolder) const;

    //
    //  Import given public key to the internal crypto representation.
    //
    Result<vscf_impl_ptr_t> importPublicKey(const QByteArray &publicKeyData) const;

    //
    //  Import given private key to the internal crypto representation.
    //
    Result<vscf_impl_ptr_t> importPrivateKey(const QByteArray &privateKeyData) const;

    //
    //  Generate a new private key and export it.
    //  If success return [publicKey, privateKey].
    //
    Result<std::tuple<QByteArray, QByteArray>> generateKeyPair() const;

private:
    vssq_messenger_cloud_fs_ptr_t m_cloudFs;
    vscf_impl_ptr_t m_random;
    QPointer<const CoreMessenger> m_coreMessenger;
};
} // namespace vm

#endif // VM_CORE_MESSENGER_CLOUD_FS_H
