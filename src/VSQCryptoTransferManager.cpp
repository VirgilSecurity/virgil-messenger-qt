//  Copyright (C) 2015-2020 Virgil Security, Inc.
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

#include "VSQCryptoTransferManager.h"

#include <VSQDownload.h>
#include <VSQSettings.h>
#include <VSQTransfer.h>
#include <VSQUpload.h>
#include <VSQUtils.h>

#include <virgil/iot/messenger/messenger.h>

using namespace VirgilIoTKit;

VSQCryptoTransferManager::VSQCryptoTransferManager(QXmppClient *client, QNetworkAccessManager *networkAccessManager, VSQSettings *settings, QObject *parent)
    : VSQTransferManager(client, networkAccessManager, settings, parent)
{}

VSQCryptoTransferManager::~VSQCryptoTransferManager()
{}

VSQUpload *VSQCryptoTransferManager::startCryptoUpload(const QString id, const QString filePath, const QString recipient)
{
    auto encFilePath = getCacheNewFilePath();
    if (!ecnryptFile(filePath, encFilePath, recipient)) {
        return nullptr;
    }
    emit fileEncrypted(id, encFilePath);
    auto upload = startUpload(id, encFilePath);
    if (!upload) {
        QFile::remove(encFilePath);
        return nullptr;
    }
    connect(upload, &VSQUpload::ended, [=](bool failed) {
        if (failed) {
            qCWarning(lcTransferManager) << "Crypt upload was failed";
        }
        qCDebug(lcTransferManager) << "Removing of:" << encFilePath;
        // TODO(fpohtmeh): remove
        //QFile::remove(encFilePath);
    });
    return upload;
}

VSQDownload *VSQCryptoTransferManager::startCryptoDownload(const QString id, const QUrl url, const QString filePath, const QString recipient)
{
    auto decFilePath = getCacheNewFilePath();
    auto download = startDownload(id, url, decFilePath);
    connect(download, &VSQDownload::ended, [=](bool failed) {
        if (failed) {
            qCWarning(lcTransferManager) << "Crypt download was failed";
        }
        else if (decryptFile(decFilePath, filePath, recipient)) {
            emit fileDownloadedAndDecrypted(id, filePath);
        }
        qCDebug(lcTransferManager) << "Removing of:" << decFilePath;
        // TODO(fpohtmeh): remove
        //QFile::remove(decFilePath);
    });
    return download;
}

QString VSQCryptoTransferManager::getCacheNewFilePath()
{
    return settings()->attachmentCacheDir().filePath(VSQUtils::createUuid());
}

bool VSQCryptoTransferManager::ecnryptFile(const QString &path, const QString &encPath, const QString &recipient)
{
    qCDebug(lcTransferManager) << "File encryption:" << path << "=>" << encPath << "Recipient:" << recipient;
#ifdef VS_DEVMODE_BAD_DECRYPT
    qCWarning(lcTransferManager) << "ENCRYPTION IS DISABLED";
    return QFile::copy(path, encPath);
#endif
    // Read
    QFile file(path);
    if (!file.exists()) {
        qCCritical(lcTransferManager) << "Source file doesn't exist";
        return false;
    }
    if (!file.open(QFile::ReadOnly)) {
        qCCritical(lcTransferManager) << "Source file can't be opened";
        return false;
    }
    const auto bytes = file.readAll();
    qCDebug(lcTransferManager) << "Read" << bytes.size() << "bytes";
    if (bytes.size() == 0) {
        qCDebug(lcTransferManager) << "Empty file was skipped";
        return false;
    }
    file.close();

    // Encrypt
    std::vector<char> encBytes(5 * bytes.size() + 5000);
    size_t encBytesSize = 0;
    const auto code = vs_messenger_virgil_encrypt_msg(
                recipient.toStdString().c_str(),
                reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size(),
                reinterpret_cast<uint8_t*>(encBytes.data()), encBytes.size(), &encBytesSize);
    if (VS_CODE_OK != code)
    {
        qCCritical(lcTransferManager) << "Cannot encrypt file:" << path << "code:" << code;
        return false;
    }
    encBytes.resize(encBytesSize);

    encBytes.push_back(0);
    // Write
    QFile encFile(encPath);
    if (!encFile.open(QFile::WriteOnly)) {
        qCCritical(lcTransferManager) << "Destination file can't be opened";
        return false;
    }
    encFile.write(encBytes.data(), encBytes.size());
    encFile.close();
    qCDebug(lcTransferManager) << "File encrypted:" << encPath << "size:" << QFileInfo(encFile).size();
    return true;
}

bool VSQCryptoTransferManager::decryptFile(const QString &encPath, const QString &path, const QString &recipient)
{
    qCDebug(lcTransferManager) << "File decryption:" << encPath << "=>" << path << "Recipient:" << recipient;
#ifdef VS_DEVMODE_BAD_DECRYPT
    qCWarning(lcTransferManager) << "DECRYPTION IS DISABLED";
    return QFile::copy(encPath, path);
#endif

    // Read
    QFile encFile(encPath);
    if (!encFile.exists()) {
        qCCritical(lcTransferManager) << "Source file doesn't exist";
        return false;
    }
    if (!encFile.open(QFile::ReadOnly)) {
        qCCritical(lcTransferManager) << "Source file can't be opened";
        return false;
    }
    const auto encBytes = encFile.readAll();
    qCDebug(lcTransferManager) << "Read" << encBytes.size() << "bytes";
    encFile.close();
    if (encBytes.size() == 0) {
        qCDebug(lcTransferManager) << "Empty file was skipped";
        return false;
    }

    // Decrypt
    std::vector<char> bytes(encBytes.size());
    size_t bytesSize = 0;
    const auto code = vs_messenger_virgil_decrypt_msg(
                recipient.toStdString().c_str(),
                encBytes.data(),
                reinterpret_cast<uint8_t*>(bytes.data()), bytes.size(), &bytesSize);
    if (VS_CODE_OK != code)
    {
        qCCritical(lcTransferManager) << "Cannot decrypt file:" << encPath << "code:" << code;
        return false;
    }
    bytes.resize(bytesSize);

    // Write
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        qCCritical(lcTransferManager) << "Destination file can't be opened";
        return false;
    }
    file.write(bytes.data(), bytesSize);
    file.close();
    qCDebug(lcTransferManager) << "File decrypted:" << path << "size:" << QFileInfo(file).size();
    return true;
}
