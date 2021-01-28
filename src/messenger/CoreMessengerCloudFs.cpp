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
#include "UserImpl.h"

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

static vssq_messenger_cloud_fs_created_file_ptr_t vssq_messenger_cloud_fs_created_file_wrap_ptr(
        vssq_messenger_cloud_fs_created_file_t *ptr) {

        return vssq_messenger_cloud_fs_created_file_ptr_t{ptr, vssq_messenger_cloud_fs_created_file_delete};
}


using vssq_messenger_cloud_fs_cipher_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_cipher_t>;

static vssq_messenger_cloud_fs_cipher_ptr_t vssq_messenger_cloud_fs_cipher_wrap_ptr(vssq_messenger_cloud_fs_cipher_t *ptr) {
    return vssq_messenger_cloud_fs_cipher_ptr_t{ptr, vssq_messenger_cloud_fs_cipher_delete};
}


using vssq_messenger_cloud_fs_file_download_info_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_file_download_info_t>;

static vssq_messenger_cloud_fs_file_download_info_ptr_t vssq_messenger_cloud_fs_file_download_info_wrap_ptr(vssq_messenger_cloud_fs_file_download_info_t *ptr) {
    return vssq_messenger_cloud_fs_file_download_info_ptr_t{ptr, vssq_messenger_cloud_fs_file_download_info_delete};
}


using vssq_messenger_cloud_fs_folder_info_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_folder_info_t>;

static vssq_messenger_cloud_fs_folder_info_ptr_t vssq_messenger_cloud_fs_folder_info_wrap_ptr(
        vssq_messenger_cloud_fs_folder_info_t *ptr) {

        return vssq_messenger_cloud_fs_folder_info_ptr_t{ptr, vssq_messenger_cloud_fs_folder_info_delete};
}


using vssq_messenger_cloud_fs_folder_ptr_t = vsc_unique_ptr<vssq_messenger_cloud_fs_folder_t>;

static vssq_messenger_cloud_fs_folder_ptr_t vssq_messenger_cloud_fs_folder_wrap_ptr(
        vssq_messenger_cloud_fs_folder_t *ptr) {

        return vssq_messenger_cloud_fs_folder_ptr_t{ptr, vssq_messenger_cloud_fs_folder_delete};
}


using vscf_recipient_cipher_ptr_t = vsc_unique_ptr<vscf_recipient_cipher_t>;

static vscf_recipient_cipher_ptr_t vscf_recipient_cipher_wrap_ptr(vscf_recipient_cipher_t *ptr) {
    return vscf_recipient_cipher_ptr_t{ptr, vscf_recipient_cipher_delete};
}


using vscf_key_provider_ptr_t = vsc_unique_ptr<vscf_key_provider_t>;

static vscf_key_provider_ptr_t vscf_key_provider_wrap_ptr(vscf_key_provider_t *ptr) {
    return vscf_key_provider_ptr_t{ptr, vscf_key_provider_delete};
}


static CloudFsFileInfo cloudFsFileInfoFromC(const vssq_messenger_cloud_fs_file_info_t *fileInfoC) {

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


static CloudFsFolderInfo cloudFsFolderInfoFromC(const vssq_messenger_cloud_fs_folder_info_t *folderInfoC) {

    CloudFsFolderInfo info;
    info.id = CloudFsFolderId(vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_id(folderInfoC)));
    info.name = vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_name(folderInfoC));
    info.createdAt = QDateTime::fromTime_t(vssq_messenger_cloud_fs_folder_info_created_at(folderInfoC));
    info.updatedAt = QDateTime::fromTime_t(vssq_messenger_cloud_fs_folder_info_updated_at(folderInfoC));
    info.updatedBy = UserId(vsc_str_to_qstring(vssq_messenger_cloud_fs_folder_info_updated_by(folderInfoC)));

    return info;
}

// --------------------------------------------------------------------------
//  Implementation.
// --------------------------------------------------------------------------
Self::CoreMessengerCloudFs(vssq_messenger_cloud_fs_ptr_t cloudFs, vscf_impl_ptr_t random)
    : m_cloudFs(std::move(cloudFs)), m_random(std::move(random)) {
}


Self::FutureResult<CloudFsNewFile> Self::createFile(const QString &sourceFilePath, const QString &destFilePath) {

    return createFile(sourceFilePath, destFilePath, CloudFsFolderId(), QByteArray());
}


Self::FutureResult<CloudFsNewFile> Self::createFile(const QString &sourceFilePath, const QString &destFilePath,
        const CloudFsFolderId& parentFolderId, const QByteArray& parentFolderPublicKey) {

    return QtConcurrent::run([this, sourceFilePath, destFilePath, parentFolderId, parentFolderPublicKey]()
            -> Result<CloudFsNewFile> {

        qCInfo(lcCoreMessengerCloudFs) << "Trying to create file";

        //
        //  Encrypt file.
        //
        const auto encryptFileResult = encryptFile(sourceFilePath, destFilePath);
        if (auto status = std::get_if<CoreMessengerStatus>(&encryptFileResult)) {
            return *status;
        }

        auto fileKey = std::move(*std::get_if<QByteArray>(&encryptFileResult));

        //
        //  Encrypt file key.
        //
        auto encryptKeyResult = encryptKey(fileKey, parentFolderId, parentFolderPublicKey);
        if (auto status = std::get_if<CoreMessengerStatus>(&encryptKeyResult)) {
            return *status;
        }

        auto fileEncryptedKey = std::move(*std::get_if<QByteArray>(&encryptKeyResult));

        //
        //  Create file on the CLoudFS.
        //
        QFileInfo sourceFileInfo(sourceFilePath);
        QFileInfo destFileInfo(destFilePath);

        auto sourceFileMimeType = QMimeDatabase().mimeTypeForFile(sourceFileInfo).name().toStdString();

        vssq_error_t error;
        vssq_error_reset(&error);

        auto fileName = sourceFileInfo.fileName().toStdString();
        auto fileNameC = vsc_str_from(fileName);
        auto fileEncryptedKeyC = vsc_data_from(fileEncryptedKey);
        auto sourceFileMimeTypeC = vsc_str_from(sourceFileMimeType);
        auto parentFolderIdStd = QString(parentFolderId).toStdString();

        auto createdFile = vssq_messenger_cloud_fs_created_file_wrap_ptr(
                vssq_messenger_cloud_fs_create_file(
                        m_cloudFs.get(), fileNameC, sourceFileMimeTypeC, destFileInfo.size(),
                        vsc_str_from(parentFolderIdStd), fileEncryptedKeyC, &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs) << "Can not create file: " <<
                    vsc_str_to_qstring(vssq_error_message_from_error(&error));

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


Self::FutureStatus Self::deleteFile(const CloudFsFileId& fileId) {

    return QtConcurrent::run([this, fileId]() -> CoreMessengerStatus {

        auto fileIdStd = QString(fileId).toStdString();

        const auto status = vssq_messenger_cloud_fs_delete_file(m_cloudFs.get(), vsc_str_from(fileIdStd));

        if (status != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessengerCloudFs) << "Can not delete file: " <<
                    vsc_str_to_qstring(vssq_error_message_from_status(status));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        return CoreMessengerStatus::Success;
    });
}


Self::FutureResult<CloudFsFileDownloadInfo> Self::getFileDownloadInfo(const CloudFsFileId& fileId) const {

    return QtConcurrent::run([this, fileId]() -> Result<CloudFsFileDownloadInfo> {

        vssq_error_t error;
        vssq_error_reset(&error);

        auto fileIdStd = QString(fileId).toStdString();

        auto fileDownloadInfoC = vssq_messenger_cloud_fs_file_download_info_wrap_ptr(
                vssq_messenger_cloud_fs_get_download_link(m_cloudFs.get(), vsc_str_from(fileIdStd), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs) << "Can not get file download info: " <<
                    vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        CloudFsFileDownloadInfo fileDownloadInfo;
        fileDownloadInfo.downloadLink = vsc_str_to_qstring(
                vssq_messenger_cloud_fs_file_download_info_link(fileDownloadInfoC.get()));

        fileDownloadInfo.fileEncryptedKey  = vsc_data_to_qbytearray(
                vssq_messenger_cloud_fs_file_download_info_file_encrypted_key(fileDownloadInfoC.get()));

        return fileDownloadInfo;
    });
}


Self::FutureResult<CloudFsFolder> Self::createFolder(const QString& folderName) {

    return createFolder(folderName, CloudFsFolderId(), QByteArray());
}


Self::FutureResult<CloudFsFolder> Self::createFolder(const QString& folderName, const CloudFsFolderId& parentFolderId,
        const QByteArray& parentFolderPublicKey) {

    return QtConcurrent::run([this, folderName, parentFolderId, parentFolderPublicKey]() -> Result<CloudFsFolder> {

        //
        //  Generate folder private key.
        //
        auto folderPrivateKeyResult = generateKeyPair();

        if (auto status = std::get_if<CoreMessengerStatus>(&folderPrivateKeyResult)) {
            return *status;
        }

        auto [folderPublicKey, folderPrivateKey] =
                std::move(*std::get_if<std::tuple<QByteArray, QByteArray>>(&folderPrivateKeyResult));

        //
        //  Encrypt folder private key.
        //
        auto folderEncryptedKeyResult = encryptKey(folderPrivateKey, parentFolderId, parentFolderPublicKey);

        if (auto status = std::get_if<CoreMessengerStatus>(&folderEncryptedKeyResult)) {
            return *status;
        }

        auto folderEncryptedKey = std::move(*std::get_if<QByteArray>(&folderEncryptedKeyResult));

        //
        //  Request folder creation.
        //
        vssq_error_t error;
        vssq_error_reset(&error);

        auto folderNameStd = folderName.toStdString();
        auto parentFolderIdStd = QString(parentFolderId).toStdString();

        auto fileInfoC = vssq_messenger_cloud_fs_folder_info_wrap_ptr(
                vssq_messenger_cloud_fs_create_folder(m_cloudFs.get(), vsc_str_from(folderNameStd),
                          vsc_data_from(folderEncryptedKey), vsc_data_from(folderPublicKey),
                          vsc_str_from(parentFolderIdStd), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs) << "Can create folder: " <<
                    vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        CloudFsFolder folder;
        folder.info = cloudFsFolderInfoFromC(fileInfoC.get());
        folder.folderPublicKey = std::move(folderPublicKey);
        folder.folderEncryptedKey = std::move(folderEncryptedKey);

        return folder;
    });
}


Self::FutureStatus Self::deleteFolder(const CloudFsFolderId& folderId) {

    return QtConcurrent::run([this, folderId]() -> CoreMessengerStatus {

        auto folderIdStd = QString(folderId).toStdString();

        const auto status = vssq_messenger_cloud_fs_delete_folder(m_cloudFs.get(), vsc_str_from(folderIdStd));

        if (status != vssq_status_SUCCESS) {
            qCWarning(lcCoreMessengerCloudFs) << "Can not delete folder: " <<
                    vsc_str_to_qstring(vssq_error_message_from_status(status));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }

        return CoreMessengerStatus::Success;
    });
}


Self::FutureResult<CloudFsFolder> Self::listFolder(const CloudFsFolderId& folderId) {

    return QtConcurrent::run([this, folderId]() -> Result<CloudFsFolder> {

        vssq_error_t error;
        vssq_error_reset(&error);

        auto folderIdStd = QString(folderId).toStdString();

        const auto folderC = vssq_messenger_cloud_fs_folder_wrap_ptr(
                vssq_messenger_cloud_fs_list_folder(m_cloudFs.get(), vsc_str_from(folderIdStd), &error));

        if (vssq_error_has_error(&error)) {
            qCWarning(lcCoreMessengerCloudFs) << "Can not list folder: "
                    << vsc_str_to_qstring(vssq_error_message_from_error(&error));

            return CoreMessengerStatus::Error_CloudFsRequestFailed;
        }


        CloudFsFolder folder;
        folder.info = cloudFsFolderInfoFromC(vssq_messenger_cloud_fs_folder_info(folderC.get()));

        folder.folderPublicKey =
                vsc_data_to_qbytearray(vssq_messenger_cloud_fs_folder_public_key(folderC.get()));

        folder.folderEncryptedKey =
                vsc_data_to_qbytearray(vssq_messenger_cloud_fs_folder_encrypted_key(folderC.get()));

        const auto filesC = vssq_messenger_cloud_fs_folder_files(folderC.get());
        for (auto fileIt = filesC;
                (fileIt != nullptr) && vssq_messenger_cloud_fs_file_info_list_has_item(fileIt);
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


Self::Result<QByteArray> Self::encryptFile(const QString &sourceFilePath, const QString &destFilePath) {
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs) << "Can not encrypt file: " <<
                vsc_str_to_qstring(vssq_error_message_from_status(status));

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
    vssq_status_t encryptionStatus = vssq_messenger_cloud_fs_cipher_setup_defaults(fileCipher.get());

    if (encryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(encryptionStatus);
    }

    //
    //  Encrypt - Step 2 - Get and store decryption key.
    //
    const auto decryptionKeyBufLen = vssq_messenger_cloud_fs_cipher_init_encryption_out_key_len(fileCipher.get());
    auto [decryptionKey, decryptionKeyBuf] = makeMappedBuffer(decryptionKeyBufLen);

    const auto userPrivateKey = vssq_messenger_cloud_fs_user_private_key(m_cloudFs.get());

    encryptionStatus = vssq_messenger_cloud_fs_cipher_init_encryption(
                fileCipher.get(), userPrivateKey, fileSize, decryptionKeyBuf.get());

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

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionWriteFailed);
    }


    //
    //  Encrypt - Step 4 - Encrypt file.
    //
    qint64 processedFileSize = 0;
    while(!sourceFile.atEnd()) {
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
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), (qint64)vsc_buffer_len(workingBuffer.get())) == -1) {
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

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
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

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
        return fileError(CoreMessengerStatus::Error_FileEncryptionWriteFailed);
    }

    //
    //  Return result.
    //
    return decryptionKey;
}


CoreMessengerStatus Self::decryptFile(const QString &sourceFilePath, const QString &destFilePath,
        const QByteArray& encryptedFileKey, const UserHandler& sender) {
    //
    //  Create helpers for error handling.
    //
    auto cryptoError = [](vssq_status_t status) -> CoreMessengerStatus {
        qCWarning(lcCoreMessengerCloudFs) << "Can not decrypt file: " <<
                vsc_str_to_qstring(vssq_error_message_from_status(status));

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
    vssq_status_t decryptionStatus = vssq_messenger_cloud_fs_cipher_setup_defaults(fileCipher.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    //
    //  Decrypt - Step 2 - Decrypt a file decryption key.
    //
    auto decryptKeyResult = decryptKey(encryptedFileKey, sender);
    if (auto status = std::get_if<CoreMessengerStatus>(&decryptKeyResult)) {
        return *status;
    }

    auto decryptionKey = std::move(*std::get_if<QByteArray>(&decryptKeyResult));

    //
    //  Decrypt - Step 3 - Setup decryption key.
    //
    decryptionStatus = vssq_messenger_cloud_fs_cipher_start_decryption(fileCipher.get(), vsc_data_from(decryptionKey));

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    //
    //  Decrypt - Step 4 - Decrypt file.
    //
    auto workingBuffer = vsc_buffer_wrap_ptr(vsc_buffer_new());

    qint64 processedFileSize = 0;
    while(!sourceFile.atEnd()) {
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
        if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), (qint64)vsc_buffer_len(workingBuffer.get())) == -1) {
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
    decryptionStatus = vssq_messenger_cloud_fs_cipher_finish_decryption(fileCipher.get(), senderPublicKey, workingBuffer.get());

    if (decryptionStatus != vssq_status_SUCCESS) {
        return cryptoError(decryptionStatus);
    }

    if (destFile.write((const char *)vsc_buffer_bytes(workingBuffer.get()), vsc_buffer_len(workingBuffer.get())) == -1) {
        return fileError(CoreMessengerStatus::Error_FileDecryptionWriteFailed);
    }

    return CoreMessengerStatus::Success;
}


Self::Result<QByteArray> Self::encryptKey(const QByteArray &fileKey, const CloudFsFolderId& parentFolderId,
            const QByteArray& parentFolderPublicKey) const {
    //
    //  Create helpers for error handling.
    //
    auto foundationError = [](vscf_status_t status) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs) << "Can not encrypt file key: " <<
                vsc_str_to_qstring(vscf_error_message_from_status(status));

        return CoreMessengerStatus::Error_FileEncryptionCryptoFailed;
    };

    //
    //  Init recipient cipher.
    //
    auto cipher = vscf_recipient_cipher_wrap_ptr(vscf_recipient_cipher_new());

    vscf_recipient_cipher_use_random(cipher.get(), m_random.get());

    //
    //  Encrypt file key for myself.
    //
    const auto currentUser = vssq_messenger_cloud_fs_user(m_cloudFs.get());
    const auto currentUserPrivateKey = vssq_messenger_cloud_fs_user_private_key(m_cloudFs.get());
    const auto currentUserPublicKey = vssq_messenger_user_public_key(currentUser);
    const auto currentUserIdentity = vssq_messenger_user_identity(currentUser);

    vscf_recipient_cipher_add_key_recipient(cipher.get(), vsc_str_as_data(currentUserIdentity), currentUserPublicKey);

    if(parentFolderId.isValid()) {
        auto publicKeyResult = importPublicKey(parentFolderPublicKey);

        if (auto status = std::get_if<CoreMessengerStatus>(&publicKeyResult)) {
            return *status;
        }

        auto parentFolderIdStd = QString(parentFolderId).toStdString();
        auto parentFolderIdC = vsc_str_from(parentFolderIdStd);
        auto folderPublicKeyC = std::move(*std::get_if<vscf_impl_ptr_t>(&publicKeyResult));

        vscf_recipient_cipher_add_key_recipient(cipher.get(), vsc_str_as_data(parentFolderIdC), folderPublicKeyC.get());
    }

    vscf_status_t foundationStatus =
            vscf_recipient_cipher_add_signer(cipher.get(), vsc_str_as_data(currentUserIdentity), currentUserPrivateKey);

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    foundationStatus = vscf_recipient_cipher_start_signed_encryption(cipher.get(), fileKey.size());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    const auto messageInfoLen = vscf_recipient_cipher_message_info_len(cipher.get());
    const auto encMsgDataLen =
            vscf_recipient_cipher_encryption_out_len(cipher.get(), fileKey.size()) +
            vscf_recipient_cipher_encryption_out_len(cipher.get(), 0);

    auto [fileEncryptedKey, fileEncryptedKeyBuf] = makeMappedBuffer(messageInfoLen + encMsgDataLen);

    vscf_recipient_cipher_pack_message_info(cipher.get(), fileEncryptedKeyBuf.get());

    foundationStatus = vscf_recipient_cipher_process_encryption(
            cipher.get(), vsc_data_from(fileKey), fileEncryptedKeyBuf.get());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    foundationStatus = vscf_recipient_cipher_finish_encryption(cipher.get(), fileEncryptedKeyBuf.get());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    const auto encMsgInfoFooterLen = vscf_recipient_cipher_message_info_footer_len(cipher.get());
    ensureMappedBuffer(fileEncryptedKey, fileEncryptedKeyBuf, encMsgInfoFooterLen);

    foundationStatus = vscf_recipient_cipher_pack_message_info_footer(cipher.get(), fileEncryptedKeyBuf.get());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    adjustMappedBuffer(fileEncryptedKeyBuf, fileEncryptedKey);

    return fileEncryptedKey;
}


Self::Result<QByteArray> Self::decryptKey(const QByteArray &fileEncryptedKey, const UserHandler& sender) const {

    //
    //  Create helpers for error handling.
    //
    auto foundationError = [](vscf_status_t status) -> Self::Result<QByteArray> {
        qCWarning(lcCoreMessengerCloudFs) << "Can not decrypt file key: " <<
                vsc_str_to_qstring(vscf_error_message_from_status(status));

        return CoreMessengerStatus::Error_FileDecryptionCryptoFailed;
    };

    //
    //  Init recipient cipher.
    //
    auto cipher = vscf_recipient_cipher_wrap_ptr(vscf_recipient_cipher_new());

    vscf_recipient_cipher_use_random(cipher.get(), m_random.get());

    //
    //  Decrypt.
    //
    const auto currentUser = vssq_messenger_cloud_fs_user(m_cloudFs.get());
    const auto currentUserPrivateKey = vssq_messenger_cloud_fs_user_private_key(m_cloudFs.get());
    const auto currentUserIdentity = vssq_messenger_user_identity(currentUser);

    vscf_status_t foundationStatus = vscf_recipient_cipher_start_decryption_with_key(cipher.get(),
            vsc_str_as_data(currentUserIdentity), currentUserPrivateKey, vsc_data_empty());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    const auto fileKeyBufLen = vscf_recipient_cipher_decryption_out_len(cipher.get(), fileEncryptedKey.size()) +
                               vscf_recipient_cipher_decryption_out_len(cipher.get(), 0);

    auto [fileKey, fileKeyBuf] = makeMappedBuffer(fileKeyBufLen);

    foundationStatus =
            vscf_recipient_cipher_process_decryption(cipher.get(), vsc_data_from(fileEncryptedKey), fileKeyBuf.get());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    foundationStatus = vscf_recipient_cipher_finish_decryption(cipher.get(), fileKeyBuf.get());

    if (foundationStatus != vscf_status_SUCCESS) {
        return foundationError(foundationStatus);
    }

    //
    //  Verify.
    //
    if(!vscf_recipient_cipher_is_data_signed(cipher.get())) {
        return CoreMessengerStatus::Error_FileDecryptionCryptoSignatureFailed;
    }

    const auto signerInfos = vscf_recipient_cipher_signer_infos(cipher.get());

    if(!vscf_signer_info_list_has_item(signerInfos)) {
        return CoreMessengerStatus::Error_FileDecryptionCryptoSignatureFailed;
    }

    const auto signerInfo = vscf_signer_info_list_item(signerInfos);

    const auto senderPublicKey = vssq_messenger_user_public_key(sender->impl()->user.get());
    const bool verified = vscf_recipient_cipher_verify_signer_info(cipher.get(), signerInfo, senderPublicKey);
    if (!verified) {
        return CoreMessengerStatus::Error_FileDecryptionCryptoSignatureFailed;
    }

    adjustMappedBuffer(fileKeyBuf, fileKey);

    return fileKey;
}


Self::Result<vscf_impl_ptr_t> Self::importPublicKey(const QByteArray& publicKeyData) const {

    auto keyProvider = vscf_key_provider_wrap_ptr(vscf_key_provider_new());

    vscf_key_provider_use_random(keyProvider.get(), m_random.get());

    vscf_error_t foundationError;
    vscf_error_reset(&foundationError);

    auto publicKey = vscf_impl_wrap_ptr(
            vscf_key_provider_import_public_key(keyProvider.get(), vsc_data_from(publicKeyData), &foundationError));

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs) << "Can not import public key: " <<
                                          vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoImportPublicKeyFailed;
    }

    return publicKey;
}


Self::Result<vscf_impl_ptr_t> Self::importPrivateKey(const QByteArray& privateKeyData) const {

    auto keyProvider = vscf_key_provider_wrap_ptr(vscf_key_provider_new());

    vscf_key_provider_use_random(keyProvider.get(), m_random.get());

    vscf_error_t foundationError;
    vscf_error_reset(&foundationError);

    auto privateKey = vscf_impl_wrap_ptr(
            vscf_key_provider_import_private_key(keyProvider.get(), vsc_data_from(privateKeyData), &foundationError));

    if (vscf_error_has_error(&foundationError)) {
        qCWarning(lcCoreMessengerCloudFs) << "Can not import private key: " <<
                                          vsc_str_to_qstring(vscf_error_message_from_error(&foundationError));

        return CoreMessengerStatus::Error_CryptoImportPrivateKeyFailed;
    }

    return privateKey;
}


Self::Result<std::tuple<QByteArray, QByteArray>> Self::generateKeyPair() const {

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
