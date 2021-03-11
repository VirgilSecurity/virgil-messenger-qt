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

#include "CoreMessengerCloudFs.h"

#include "CommKitBridge.h"
#include "FileUtils.h"
#include "UserImpl.h"
#include "CoreMessenger.h"

#include <virgil/crypto/foundation/vscf_recipient_cipher.h>
#include <virgil/crypto/foundation/vscf_error_message.h>
#include <virgil/crypto/foundation/vscf_key_provider.h>
#include <virgil/crypto/foundation/vscf_private_key.h>

#include <virgil/sdk/comm-kit/vssq_error_message.h>
#include <virgil/sdk/comm-kit/vssq_messenger_cloud_fs.h>
#include <virgil/sdk/comm-kit/vssq_messenger_cloud_fs_cipher.h>

#include <QLoggingCategory>
#include <QtConcurrent>
#include <QMimeDatabase>

using namespace vm;
using Self = vm::CoreMessengerCloudFs;

Q_LOGGING_CATEGORY(lcCoreMessengerCloudFs, "core-cloud-fs");

// --------------------------------------------------------------------------
//  C Helpers.
// --------------------------------------------------------------------------
using vssq_messenger_cloud_fs_created_file_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_created_file_t>;

static vssq_messenger_cloud_fs_created_file_ptr_t
vssq_messenger_cloud_fs_created_file_wrap_ptr(vssq_messenger_cloud_fs_created_file_t *ptr)
{

    return vssq_messenger_cloud_fs_created_file_ptr_t { ptr, vssq_messenger_cloud_fs_created_file_delete };
}

using vssq_messenger_cloud_fs_cipher_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_cipher_t>;

static vssq_messenger_cloud_fs_cipher_ptr_t
vssq_messenger_cloud_fs_cipher_wrap_ptr(vssq_messenger_cloud_fs_cipher_t *ptr)
{
    return vssq_messenger_cloud_fs_cipher_ptr_t { ptr, vssq_messenger_cloud_fs_cipher_delete };
}

using vssq_messenger_cloud_fs_file_download_info_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_file_download_info_t>;

static vssq_messenger_cloud_fs_file_download_info_ptr_t
vssq_messenger_cloud_fs_file_download_info_wrap_ptr(vssq_messenger_cloud_fs_file_download_info_t *ptr)
{
    return vssq_messenger_cloud_fs_file_download_info_ptr_t { ptr, vssq_messenger_cloud_fs_file_download_info_delete };
}

using vssq_messenger_cloud_fs_folder_info_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_folder_info_t>;

static vssq_messenger_cloud_fs_folder_info_ptr_t
vssq_messenger_cloud_fs_folder_info_wrap_ptr(vssq_messenger_cloud_fs_folder_info_t *ptr)
{

    return vssq_messenger_cloud_fs_folder_info_ptr_t { ptr, vssq_messenger_cloud_fs_folder_info_delete };
}

using vssq_messenger_cloud_fs_folder_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_folder_t>;

static vssq_messenger_cloud_fs_folder_ptr_t
vssq_messenger_cloud_fs_folder_wrap_ptr(vssq_messenger_cloud_fs_folder_t *ptr)
{

    return vssq_messenger_cloud_fs_folder_ptr_t { ptr, vssq_messenger_cloud_fs_folder_delete };
}

using vscf_recipient_cipher_ptr_t = vsc_unique_ptr<vscf_recipient_cipher_t>;

static vscf_recipient_cipher_ptr_t vscf_recipient_cipher_wrap_ptr(vscf_recipient_cipher_t *ptr)
{
    return vscf_recipient_cipher_ptr_t { ptr, vscf_recipient_cipher_delete };
}

using vscf_key_provider_ptr_t = vsc_unique_ptr<vscf_key_provider_t>;

static vscf_key_provider_ptr_t vscf_key_provider_wrap_ptr(vscf_key_provider_t *ptr)
{
    return vscf_key_provider_ptr_t { ptr, vscf_key_provider_delete };
}

using vssq_messenger_cloud_fs_access_list_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_access_list_t>;

static vssq_messenger_cloud_fs_access_list_ptr_t
vssq_messenger_cloud_fs_access_list_wrap_ptr(vssq_messenger_cloud_fs_access_list_t *ptr)
{
    return vssq_messenger_cloud_fs_access_list_ptr_t { ptr, vssq_messenger_cloud_fs_access_list_delete };
}

static CloudFsFileInfo cloudFsFileInfoFromC(const vssq_messenger_cloud_fs_file_info_t *fileInfoC)
{

    CloudFsFileInfo info;
    info.id = CloudFsFileId(vsc_str_to_qstring(vssq_messenger_cloud_fs_file_info_id(fileInfoC)));
    info.name = vsc_str_to_qstring(vssq_messenger_cloud_fs_file_info_name(fileInfoC));
    info.type = vsc_str_to_qstring(vssq_messenger_cloud_fs_file_info_type(fileInfoC));
    info.size = vssq_messenger_cloud_fs_file_info_size(fileInfoC);
    info.createdAt = QDateTime::fromTime_t(vssq_messenger_cloud_fs_file_info_created_at(fileInfoC));
    info.updatedAt = QDateTime::fromTime_t(vssq_messenger_cloud_fs_file_info_updated_at(fileInfoC));
    info.updatedBy = UserId(vsc_str_to_qstring(vssq_messenger_cloud_fs_file_info_updated_by(fileInfoC)));

    return info;
}

static CloudFsFolderInfo cloudFsFolderInfoFromC(const vssq_messenger_cloud_fs_folder_info_t *folderInfoC)
{

    CloudFsFolderInfo info;
    info.id = CloudFsFolderId(vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_id(folderInfoC)));
    info.name = vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_name(folderInfoC));
    info.createdAt = QDateTime::fromTime_t(vssq_messenger_cloud_fs_folder_info_created_at(folderInfoC));
    info.updatedAt = QDateTime::fromTime_t(vssq_messenger_cloud_fs_folder_info_updated_at(folderInfoC));
    info.updatedBy = UserId(vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_updated_by(folderInfoC)));
    info.sharedGroupId =
            CloudFsSharedGroupId(vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_shared_group_id(folderInfoC)));

    return info;
}

vssq_messenger_cloud_fs_permission_t memberTypeToPermission(const CloudFileMember::Type &memberType)
{
    switch (memberType) {
    case CloudFileMember::Type::Member:
        return vssq_messenger_cloud_fs_permission_USER;
    case CloudFileMember::Type::Owner:
        return vssq_messenger_cloud_fs_permission_ADMIN;
    default:
        throw std::logic_error("Invalid CloudFileMember type");
    }
}

CloudFileMember::Type permissionToMemberType(const vssq_messenger_cloud_fs_permission_t permission)
{
    switch (permission) {
    case vssq_messenger_cloud_fs_permission_USER:
        return CloudFileMember::Type::Member;
    case vssq_messenger_cloud_fs_permission_ADMIN:
        return CloudFileMember::Type::Owner;
    default:
        throw std::logic_error("Invalid cloud fs permission");
    }
}

// --------------------------------------------------------------------------
//  Implementation.
// --------------------------------------------------------------------------
Self::CoreMessengerCloudFs(const CoreMessenger *coreMessenger, vscf_impl_ptr_t random)
    : m_coreMessenger(coreMessenger), m_random(std::move(random))
{
}

Self::FutureResult<CloudFsNewFile> Self::createFile(const QString &sourceFilePath, const QString &destFilePath,
                                                    const CloudFsFolderId &parentFolderId,
                                                    const QByteArray &parentFolderPublicKey)
{

    return QtConcurrent::run([this, sourceFilePath, destFilePath, parentFolderId,
                              parentFolderPublicKey]() -> Result<CloudFsNewFile> {
        qCDebug(lcCoreMessengerCloudFs) << "Trying to create file" << FileUtils::fileName(sourceFilePath);

        //
        //  Encrypt file.
        //
        const auto encryptFileResult = encryptFile(sourceFilePath, destFilePath);
        if (auto status = std::get_if<CoreMessengerStatus>(&encryptFileResult)) {
            return *status;
        }

        auto fileKey = std::move(*std::get_if<QByteArray>(&encryptFileResult));

        //
        //  Create file on the CLoudFS.
        //
        QFileInfo destFileInfo(destFilePath);

        auto sourceFileMimeType = FileUtils::fileMimeType(sourceFilePath).toStdString();

        vssq_error_t error;
        vssq_error_reset(&error);

        auto fileName = FileUtils::attachmentFileName(FileUtils::localFileToUrl(sourceFilePath), false).toStdString();
        auto fileNameC = vsc_str_from(fileName);
        auto fileKeyC = vsc_data_from(fileKey);
        auto sourceFileMimeTypeC = vsc_str_from(sourceFileMimeType);
        auto parentFolderIdStd = QString(parentFolderId).toStdString();

        auto createdFile = vssq_messenger_cloud_fs_created_file_wrap_ptr(vssq_messenger_cloud_fs_create_file(
                m_coreMessenger->cloudFsC(), fileNameC, sourceFileMimeTypeC, destFileInfo.size(), fileKeyC,
                vsc_str_from(parentFolderIdStd), vsc_data_from(parentFolderPublicKey), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not create file: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        //
        //  Return a new file.
        //
        CloudFsNewFile newFile;
        newFile.info = cloudFsFileInfoFromC(vssq_messenger_cloud_fs_created_file_info(createdFile.get()));
        newFile.uploadLink = vsc_str_to_qstring(vssq_messenger_cloud_fs_created_file_upload_link(createdFile.get()));

        return newFile;
    });
}

Self::FutureStatus Self::deleteFile(const CloudFsFileId &fileId)
{

    return QtConcurrent::run([this, fileId]() -> CoreMessengerStatus {
        auto fileIdStd = QString(fileId).toStdString();

        const auto status = vssq_messenger_cloud_fs_delete_file(m_coreMessenger->cloudFsC(), vsc_str_from(fileIdStd));

        if (status != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not delete file: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        return CoreMessengerStatus::Success;
    });
}

Self::FutureResult<CloudFsFileDownloadInfo> Self::getFileDownloadInfo(const CloudFsFileId &fileId) const
{

    return QtConcurrent::run([this, fileId]() -> Result<CloudFsFileDownloadInfo> {
        vssq_error_t error;
        vssq_error_reset(&error);

        auto fileIdStd = QString(fileId).toStdString();

        auto fileDownloadInfoC =
                vssq_messenger_cloud_fs_file_download_info_wrap_ptr(vssq_messenger_cloud_fs_get_download_link(
                        m_coreMessenger->cloudFsC(), vsc_str_from(fileIdStd), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not get file download info: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        CloudFsFileDownloadInfo fileDownloadInfo;
        fileDownloadInfo.downloadLink =
                vsc_str_to_qstring(vssq_messenger_cloud_fs_file_download_info_link(fileDownloadInfoC.get()));

        fileDownloadInfo.fileEncryptedKey = vsc_data_to_qbytearray(
                vssq_messenger_cloud_fs_file_download_info_file_encrypted_key(fileDownloadInfoC.get()));

        return fileDownloadInfo;
    });
}

Self::FutureResult<CloudFsFolder> Self::createFolder(const QString &folderName, const CloudFileMembers &members,
                                                     const CloudFsFolderId &parentFolderId,
                                                     const QByteArray &parentFolderPublicKey)
{
    return QtConcurrent::run([this, folderName, members, parentFolderId,
                              parentFolderPublicKey]() -> Result<CloudFsFolder> {
        //
        //  Request folder creation.
        //
        auto folderNameStd = folderName.toStdString();
        auto parentFolderIdStd = QString(parentFolderId).toStdString();

        vssq_error_t error;
        vssq_error_reset(&error);

        vssq_messenger_cloud_fs_folder_info_ptr_t fileInfo = vssq_messenger_cloud_fs_folder_info_wrap_ptr(nullptr);
        if (members.empty()) {
            fileInfo = vssq_messenger_cloud_fs_folder_info_wrap_ptr(vssq_messenger_cloud_fs_create_folder(
                    m_coreMessenger->cloudFsC(), vsc_str_from(folderNameStd), vsc_str_from(parentFolderIdStd),
                    vsc_data_from(parentFolderPublicKey), &error));
        } else {
            auto usersAccess = vssq_messenger_cloud_fs_access_list_wrap_ptr(vssq_messenger_cloud_fs_access_list_new());
            for (const auto &member : members) {
                const auto user = m_coreMessenger->findUserByUsername(member->contact()->username());
                if (user) {
                    //
                    //  Exclude Self.
                    //
                    if (user->id() != m_coreMessenger->currentUser()->id()) {
                        vssq_messenger_cloud_fs_access_list_add_user(usersAccess.get(), user->impl()->user.get(),
                                                                     memberTypeToPermission(member->type()));
                    }
                } else {
                    return CoreMessengerStatus::Error_UserNotFound;
                }
            }

            fileInfo = vssq_messenger_cloud_fs_folder_info_wrap_ptr(vssq_messenger_cloud_fs_create_shared_folder(
                    m_coreMessenger->cloudFsC(), vsc_str_from(folderNameStd), vsc_str_from(parentFolderIdStd),
                    vsc_data_from(parentFolderPublicKey), usersAccess.get(), &error));
        }

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Cannot create folder: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        CloudFsFolder folder;
        folder.info = cloudFsFolderInfoFromC(fileInfo.get());
        return folder;
    });
}

Self::FutureStatus Self::deleteFolder(const CloudFsFolderId &folderId)
{

    return QtConcurrent::run([this, folderId]() -> CoreMessengerStatus {
        auto folderIdStd = QString(folderId).toStdString();

        const auto status =
                vssq_messenger_cloud_fs_delete_folder(m_coreMessenger->cloudFsC(), vsc_str_from(folderIdStd));

        if (status != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not delete folder: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        return CoreMessengerStatus::Success;
    });
}

Self::FutureResult<CloudFsFolder> Self::listFolder(const CloudFsFolderId &folderId)
{

    return QtConcurrent::run([this, folderId]() -> Result<CloudFsFolder> {
        vssq_error_t error;
        vssq_error_reset(&error);

        auto folderIdStd = QString(folderId).toStdString();

        const auto folderC = vssq_messenger_cloud_fs_folder_wrap_ptr(
                vssq_messenger_cloud_fs_list_folder(m_coreMessenger->cloudFsC(), vsc_str_from(folderIdStd), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not list folder: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        CloudFsFolder folder;
        folder.info = cloudFsFolderInfoFromC(vssq_messenger_cloud_fs_folder_info(folderC.get()));

        folder.folderPublicKey = vsc_data_to_qbytearray(vssq_messenger_cloud_fs_folder_public_key(folderC.get()));

        folder.folderEncryptedKey = vsc_data_to_qbytearray(vssq_messenger_cloud_fs_folder_encrypted_key(folderC.get()));

        const auto filesC = vssq_messenger_cloud_fs_folder_files(folderC.get());
        for (auto fileIt = filesC; (fileIt != nullptr) && vssq_messenger_cloud_fs_file_info_list_has_item(fileIt);
             fileIt = vssq_messenger_cloud_fs_file_info_list_next(fileIt)) {

            auto fileInfoC = vssq_messenger_cloud_fs_file_info_list_item(fileIt);
            folder.files.emplace_back(cloudFsFileInfoFromC(fileInfoC));
        }

        const auto foldersC = vssq_messenger_cloud_fs_folder_folders(folderC.get());
        for (auto folderIt = foldersC;
             (folderIt != nullptr) && vssq_messenger_cloud_fs_folder_info_list_has_item(folderIt);
             folderIt = vssq_messenger_cloud_fs_folder_info_list_next(folderIt)) {

            auto folderInfoC = vssq_messenger_cloud_fs_folder_info_list_item(folderIt);
            folder.folders.emplace_back(cloudFsFolderInfoFromC(folderInfoC));
        }

        return folder;
    });
}

Self::Result<QByteArray> Self::encryptFile(const QString &sourceFilePath, const QString &destFilePath)
{
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs)
                << "Can not encrypt file: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

        return CoreMessengerStatus::Error_FileEncryptionCryptoFailed;
    };

    auto fileError = [](CoreMessengerStatus error) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs) << "Can not encrypt file - read/write failed.";

        return error;
    };

    //
    //  Open File Streams.
    //
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.exists()) {
        qCWarning(lcCoreMessengerCloudFs) << "Can not encrypt file - source file not exists.";
        return fileError(CoreMessengerStatus::Error_FileDecryptionReadFailed);
    }

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionReadFailed);
    }

    QFile destFile(destFilePath);
    if (!destFile.open(QIODevice::WriteOnly)) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionReadFailed);
    }

    const auto fileSize = sourceFile.size();

    //
    //  Encrypt - Step 1 - Initialize cipher.
    //
    auto fileCipher = vssq_messenger_cloud_fs_cipher_wrap_ptr(vssq_messenger_cloud_fs_cipher_new());
    vssq_messenger_cloud_fs_cipher_use_random(fileCipher.get(), m_random.get());

    //
    //  Encrypt - Step 2 - Get and store decryption key.
    //
    const auto decryptionKeyBufLen = vssq_messenger_cloud_fs_cipher_init_encryption_out_key_len(fileCipher.get());
    auto [decryptionKey, decryptionKeyBuf] = makeMappedBuffer(decryptionKeyBufLen);

    const auto userPrivateKey = vssq_messenger_cloud_fs_user_private_key(m_coreMessenger->cloudFsC());

    vssq_status_t encryptionStatus = vssq_messenger_cloud_fs_cipher_init_encryption(fileCipher.get(), userPrivateKey,
                                                                                    fileSize, decryptionKeyBuf.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    adjustMappedBuffer(decryptionKeyBuf, decryptionKey);

    //
    //  Encrypt - Step 3 - Write head.
    //
    auto workingBuffer = vsc_buffer_wrap_ptr(vsc_buffer_new());

    const auto headLen = vssq_messenger_cloud_fs_cipher_start_encryption_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), headLen);

    encryptionStatus = vssq_messenger_cloud_fs_cipher_start_encryption(fileCipher.get(), workingBuffer.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionWriteFailed);
    }

    //
    //  Encrypt - Step 4 - Encrypt file.
    //
    qint64 processedFileSize = 0;
    while (!sourceFile.atEnd()) {
        //
        //  Read chunk.
        //
        char readBytes[2048];
        const auto readBytesSize = sourceFile.read(readBytes, sizeof(readBytes));

        if (readBytesSize == -1) {
            return fileError(CoreMessengerStatus::Error_FileEncryptionReadFailed);
        } else {
            processedFileSize += readBytesSize;
        }

        vsc_data_t readBytesData = vsc_data((byte *)readBytes, (size_t)readBytesSize);

        //
        //  Encrypt chunk.
        //
        const auto workingBufferLen =
                vssq_messenger_cloud_fs_cipher_process_encryption_out_len(fileCipher.get(), readBytesData.len);

        vsc_buffer_reset_with_capacity(workingBuffer.get(), workingBufferLen);

        encryptionStatus =
                vssq_messenger_cloud_fs_cipher_process_encryption(fileCipher.get(), readBytesData, workingBuffer.get());

        if (encryptionStatus != vssq_status_SUCCESS) {
            return cryptoError(encryptionStatus);
        }

        //
        //  Write chunk.
        //
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()),
                           (qint64)vsc_buffer_len(workingBuffer.get()))
            == -1) {
            return fileError(CoreMessengerStatus::Error_FileEncryptionWriteFailed);
        }
    }

    if (processedFileSize != fileSize) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionReadFailed);
    }

    //
    //  Encrypt - Step 5 - Write tail.
    //
    const auto tailLen = vssq_messenger_cloud_fs_cipher_finish_encryption_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), tailLen);

    encryptionStatus = vssq_messenger_cloud_fs_cipher_finish_encryption(fileCipher.get(), workingBuffer.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionWriteFailed);
    }

    //
    //  Encrypt - Step 6 - Write footer.
    //
    const auto footerLen = vssq_messenger_cloud_fs_cipher_finish_encryption_footer_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), footerLen);

    encryptionStatus = vssq_messenger_cloud_fs_cipher_finish_encryption_footer(fileCipher.get(), workingBuffer.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionWriteFailed);
    }

    //
    //  Return result.
    //
    return decryptionKey;
}

CoreMessengerStatus Self::decryptFile(const QString &sourceFilePath, const QString &destFilePath,
                                      const QByteArray &encryptedFileKey, const UserHandler &sender,
                                      const CloudFsFolder &parentFolder)
{
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> CoreMessengerStatus {
        qCWarning(lcCoreMessengerCloudFs)
                << "Can not decrypt file: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

        return CoreMessengerStatus::Error_FileDecryptionCryptoFailed;
    };

    auto fileError = [](CoreMessengerStatus errorStatus) -> CoreMessengerStatus {
        qCWarning(lcCoreMessengerCloudFs) << "Can not decrypt file - read/write failed.";

        return errorStatus;
    };

    //
    //  Open File Streams.
    //
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.exists()) {
        qCWarning(lcCoreMessengerCloudFs) << "Can not decrypt file - source file not exists.";
        return fileError(CoreMessengerStatus::Error_FileDecryptionReadFailed);
    }

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return fileError(CoreMessengerStatus::Error_FileDecryptionReadFailed);
    }

    QFile destFile(destFilePath);
    if (!destFile.open(QIODevice::WriteOnly)) {
        return fileError(CoreMessengerStatus::Error_FileDecryptionReadFailed);
    }

    const auto fileSize = sourceFile.size();

    //
    //  Decrypt - Step 1 - Initialize crypto.
    //
    auto fileCipher = vssq_messenger_cloud_fs_cipher_wrap_ptr(vssq_messenger_cloud_fs_cipher_new());
    vssq_messenger_cloud_fs_cipher_use_random(fileCipher.get(), m_random.get());

    //
    //  Decrypt - Step 2 - Decrypt a file decryption key.
    //
    auto decryptKeyResult = decryptFileKey(encryptedFileKey, sender, parentFolder);
    if (auto status = std::get_if<CoreMessengerStatus>(&decryptKeyResult)) {
        return *status;
    }

    auto decryptionKey = std::move(*std::get_if<QByteArray>(&decryptKeyResult));

    //
    //  Decrypt - Step 3 - Setup decryption key.
    //
    vssq_status_t decryptionStatus =
            vssq_messenger_cloud_fs_cipher_start_decryption(fileCipher.get(), vsc_data_from(decryptionKey));

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    //
    //  Decrypt - Step 4 - Decrypt file.
    //
    auto workingBuffer = vsc_buffer_wrap_ptr(vsc_buffer_new());

    qint64 processedFileSize = 0;
    while (!sourceFile.atEnd()) {
        //
        //  Read chunk.
        //
        char readBytes[2048];
        const auto readBytesSize = sourceFile.read(readBytes, sizeof(readBytes));

        if (readBytesSize == -1) {
            return fileError(CoreMessengerStatus::Error_FileDecryptionReadFailed);
        } else {
            processedFileSize += readBytesSize;
        }

        vsc_data_t readBytesData = vsc_data((byte *)readBytes, (size_t)readBytesSize);

        //
        //  Decrypt chunk.
        //
        const auto workingBufferLen =
                vssq_messenger_cloud_fs_cipher_process_decryption_out_len(fileCipher.get(), readBytesData.len);

        vsc_buffer_reset_with_capacity(workingBuffer.get(), workingBufferLen);

        decryptionStatus =
                vssq_messenger_cloud_fs_cipher_process_decryption(fileCipher.get(), readBytesData, workingBuffer.get());

        if (decryptionStatus != vssq_status_SUCCESS) {
            return cryptoError(decryptionStatus);
        }

        //
        //  Write chunk.
        //
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()),
                           (qint64)vsc_buffer_len(workingBuffer.get()))
            == -1) {
            return fileError(CoreMessengerStatus::Error_FileDecryptionWriteFailed);
        }
    }

    if (processedFileSize != fileSize) {
        return fileError(CoreMessengerStatus::Error_FileDecryptionReadFailed);
    }

    //
    //  Decrypt - Step 5 - Write tail and verify signature.
    //
    const auto tailLen = vssq_messenger_cloud_fs_cipher_finish_decryption_out_len(fileCipher.get());
    vsc_buffer_reset_with_capacity(workingBuffer.get(), tailLen);

    const auto senderPublicKey = vssq_messenger_user_public_key(sender->impl()->user.get());
    decryptionStatus =
            vssq_messenger_cloud_fs_cipher_finish_decryption(fileCipher.get(), senderPublicKey, workingBuffer.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get()))
        == -1) {
        return fileError(CoreMessengerStatus::Error_FileDecryptionWriteFailed);
    }

    return CoreMessengerStatus::Success;
}

Self::FutureResult<CloudFileMembers> Self::getSharedGroupUsers(const CloudFsSharedGroupId &sharedGroupId)
{
    return QtConcurrent::run([this, sharedGroupId]() -> Result<CloudFileMembers> {
        qCDebug(lcCoreMessengerCloudFs) << "Get group shared users:" << sharedGroupId;

        vssq_error_t error;
        vssq_error_reset(&error);

        const auto sharedGroupIdStd = QString(sharedGroupId).toStdString();

        auto accessList = vssq_messenger_cloud_fs_access_list_wrap_ptr(vssq_messenger_cloud_fs_get_shared_group_users(
                m_coreMessenger->cloudFsC(), vsc_str_from(sharedGroupIdStd), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not get shared group users: " << vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        CloudFileMembers cloudFileMembers;
        for (const auto *user_it = accessList.get();
             (user_it != NULL) && vssq_messenger_cloud_fs_access_list_has_item(user_it);
             user_it = vssq_messenger_cloud_fs_access_list_next(user_it)) {

            const auto user_permission = vssq_messenger_cloud_fs_access_list_item(user_it);
            const auto identity = vssq_messenger_cloud_fs_access_identity(user_permission);
            const auto permission = vssq_messenger_cloud_fs_access_permission(user_permission);

            auto contact = std::make_shared<Contact>();
            contact->setUserId(UserId(vsc_str_to_qstring(identity)));

            cloudFileMembers.emplace_back(
                    std::make_unique<CloudFileMember>(contact, permissionToMemberType(permission)));
        }

        return cloudFileMembers;
    });
}

Self::FutureStatus Self::setSharedGroupUsers(const CloudFsSharedGroupId &sharedGroupId, const QByteArray &encryptedKey,
                                             const UserHandler &keyIssuer, const CloudFileMembers &members)
{
    return QtConcurrent::run([this, sharedGroupId, encryptedKey, keyIssuer, members]() -> CoreMessengerStatus {
        qCDebug(lcCoreMessengerCloudFs) << "Set group shared users:" << sharedGroupId;

        auto usersAccess = vssq_messenger_cloud_fs_access_list_wrap_ptr(vssq_messenger_cloud_fs_access_list_new());
        for (const auto &member : members) {
            const auto user = m_coreMessenger->findUserByUsername(member->contact()->username());
            if (user) {
                //
                //  Exclude Self.
                //
                if (user->id() != m_coreMessenger->currentUser()->id()) {
                    vssq_messenger_cloud_fs_access_list_add_user(usersAccess.get(), user->impl()->user.get(),
                                                                 memberTypeToPermission(member->type()));
                }
            } else {
                return CoreMessengerStatus::Error_UserNotFound;
            }
        }

        const auto sharedGroupIdStd = QString(sharedGroupId).toStdString();

        const auto status = vssq_messenger_cloud_fs_set_shared_group_users(
                m_coreMessenger->cloudFsC(), vsc_str_from(sharedGroupIdStd), vsc_data_from(encryptedKey),
                keyIssuer->impl()->user.get(), usersAccess.get());

        if (status != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessengerCloudFs)
                    << "Can not get shared group users: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        return CoreMessengerStatus::Success;
    });
}

Self::Result<QByteArray> Self::decryptKey(const QByteArray &encryptedKey, const UserHandler &issuer) const
{

    //
    //  Create helpers for error handling.
    //
    auto handleError = [](vssq_status_t status) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs)
                << "Can not decrypt file key: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

        return CoreMessengerStatus::Error_FileDecryptionCryptoFailed;
    };

    const auto encryptedKeyC = vsc_data_from(encryptedKey);
    const size_t decryptedKeyLen =
            vssq_messenger_cloud_fs_decrypted_key_len(m_coreMessenger->cloudFsC(), encryptedKeyC);
    auto [decryptedKey, decryptedKeyBuf] = makeMappedBuffer(decryptedKeyLen);

    const vssq_status_t status = vssq_messenger_cloud_fs_decrypt_key(
            m_coreMessenger->cloudFsC(), encryptedKeyC, issuer.get()->impl()->user.get(), decryptedKeyBuf.get());

    if (status != vssq_status_SUCCESS) {
        return handleError(status);
    }

    adjustMappedBuffer(decryptedKeyBuf, decryptedKey);

    return decryptedKey;
}

Self::Result<QByteArray> Self::decryptFileKey(const QByteArray &fileEncryptedKey, const UserHandler &sender,
                                              const CloudFsFolder &parentFolder) const
{

    //
    //  Create helpers for error handling.
    //
    auto handleError = [](vssq_status_t status) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs)
                << "Can not decrypt file key: " << vsc_str_to_qstring(vssq_error_message_from_status(status));

        return CoreMessengerStatus::Error_FileDecryptionCryptoFailed;
    };

    const auto fileEncryptedKeyC = vsc_data_from(fileEncryptedKey);
    const size_t decryptedKeyLen =
            vssq_messenger_cloud_fs_decrypted_key_len(m_coreMessenger->cloudFsC(), fileEncryptedKeyC);
    auto [decryptedKey, decryptedKeyBuf] = makeMappedBuffer(decryptedKeyLen);

    vssq_status_t status = vssq_status_SUCCESS;

    if (parentFolder.isShared()) {
        //
        //  Decrypt root folder key first.
        //
        const auto parentFolderIssuer = m_coreMessenger->findUserById(parentFolder.info.updatedBy);
        if (!parentFolderIssuer) {
            return CoreMessengerStatus::Error_UserNotFound;
        }

        const auto parentFolderIdStd = QString(parentFolder.info.id).toStdString();
        const auto parentFolderIdStdC = vsc_str_from(parentFolderIdStd);

        auto folderKeyResult = decryptKey(parentFolder.folderEncryptedKey, parentFolderIssuer);
        if (auto decryptFolderKeyStatus = std::get_if<CoreMessengerStatus>(&folderKeyResult)) {
            return *decryptFolderKeyStatus;
        }

        auto folderKey = std::move(*std::get_if<QByteArray>(&folderKeyResult));

        status = vssq_messenger_cloud_fs_decrypt_key_with_parent_folder_key(
                m_coreMessenger->cloudFsC(), fileEncryptedKeyC, sender->impl()->user.get(), parentFolderIdStdC,
                vsc_data_from(folderKey), decryptedKeyBuf.get());

    } else {
        status = vssq_messenger_cloud_fs_decrypt_key(m_coreMessenger->cloudFsC(), fileEncryptedKeyC,
                                                     sender->impl()->user.get(), decryptedKeyBuf.get());
    }

    if (status != vssq_status_SUCCESS) {
        return handleError(status);
    }

    adjustMappedBuffer(decryptedKeyBuf, decryptedKey);

    return decryptedKey;
}

Self::Result<vscf_impl_ptr_t> Self::importPublicKey(const QByteArray &publicKeyData) const
{

    auto keyProvider = vscf_key_provider_wrap_ptr(vscf_key_provider_new());

    vscf_key_provider_use_random(keyProvider.get(), m_random.get());

    vscf_error_t foundationError;
    vscf_error_reset(&foundationError);

    auto publicKey = vscf_impl_wrap_ptr(
            vscf_key_provider_import_public_key(keyProvider.get(), vsc_data_from(publicKeyData), &foundationError));

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs)
                << "Can not import public key: " << vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoImportPublicKeyFailed;
    }

    return publicKey;
}

Self::Result<vscf_impl_ptr_t> Self::importPrivateKey(const QByteArray &privateKeyData) const
{

    auto keyProvider = vscf_key_provider_wrap_ptr(vscf_key_provider_new());

    vscf_key_provider_use_random(keyProvider.get(), m_random.get());

    vscf_error_t foundationError;
    vscf_error_reset(&foundationError);

    auto privateKey = vscf_impl_wrap_ptr(
            vscf_key_provider_import_private_key(keyProvider.get(), vsc_data_from(privateKeyData), &foundationError));

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs) << "Can not import private key: "
                                          << vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoImportPrivateKeyFailed;
    }

    return privateKey;
}

Self::Result<std::tuple<QByteArray, QByteArray>> Self::generateKeyPair() const
{

    //
    //  Init crypto.
    //
    auto keyProvider = vscf_key_provider_wrap_ptr(vscf_key_provider_new());

    vscf_key_provider_use_random(keyProvider.get(), m_random.get());

    vscf_error_t foundationError;
    vscf_error_reset(&foundationError);

    //
    //  Generate a private key.
    //
    auto privateKey = vscf_impl_wrap_ptr(
            vscf_key_provider_generate_private_key(keyProvider.get(), vscf_alg_id_ED25519, &foundationError));

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs) << "Con not generate a private key: "
                                          << vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoGenerateKeyFailed;
    }

    //
    //  Export the private key.
    //
    const auto exportedPrivateKeyLen = vscf_key_provider_exported_private_key_len(keyProvider.get(), privateKey.get());

    auto [exportedPrivateKey, exportedPrivateKeyBuf] = makeMappedBuffer(exportedPrivateKeyLen);

    foundationError.status =
            vscf_key_provider_export_private_key(keyProvider.get(), privateKey.get(), exportedPrivateKeyBuf.get());

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs) << "Con not export a private key: "
                                          << vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoExportPrivateKeyFailed;
    }

    adjustMappedBuffer(exportedPrivateKeyBuf, exportedPrivateKey);

    //
    //  Get a public key.
    //
    auto publicKey = vscf_impl_wrap_ptr(vscf_private_key_extract_public_key(privateKey.get()));

    //
    //  Export the private key.
    //
    const auto exportedPublicKeyLen = vscf_key_provider_exported_private_key_len(keyProvider.get(), privateKey.get());

    auto [exportedPublicKey, exportedPublicKeyBuf] = makeMappedBuffer(exportedPublicKeyLen);

    foundationError.status =
            vscf_key_provider_export_public_key(keyProvider.get(), publicKey.get(), exportedPublicKeyBuf.get());

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs) << "Con not export a public key: "
                                          << vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoExportPublicKeyFailed;
    }

    adjustMappedBuffer(exportedPublicKeyBuf, exportedPublicKey);

    return std::make_tuple(std::move(exportedPublicKey), std::move(exportedPrivateKey));
}
