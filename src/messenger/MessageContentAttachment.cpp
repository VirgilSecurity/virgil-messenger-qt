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


#include "MessageContentAttachment.h"


#include "FileUtils.h"
#include "Utils.h"
#include "android/VSQAndroid.h"


using namespace vm;
using Self = MessageContentAttachment;


bool Self::applyUpdate(const MessageUpdate& messageUpdate) {
    if (auto update = std::get_if<MessageAttachmentUploadStageUpdate>(&messageUpdate)) {
        setUploadStage(update->uploadStage);
    }
    else if (auto update = std::get_if<MessageAttachmentDownloadStageUpdate>(&messageUpdate)) {
        setDownloadStage(update->downloadStage);
    }
    else if (auto update = std::get_if<MessageAttachmentFingerprintUpdate>(&messageUpdate)) {
        setFingerprint(update->fingerprint);
    }
    else if (auto update = std::get_if<MessageAttachmentRemoteUrlUpdate>(&messageUpdate)) {
        setRemoteUrl(update->remoteUrl);
    }
    else if (auto update = std::get_if<MessageAttachmentEncryptionUpdate>(&messageUpdate)) {
        setEncryptedSize(update->encryptedSize);
        setDecryptionKey(update->decryptionKey);
        setSignature(update->signature);
    }
    else if (auto update = std::get_if<MessageAttachmentLocalPathUpdate>(&messageUpdate)) {
        setLocalPath(update->localPath);
    }
    else if (auto update = std::get_if<MessageAttachmentProcessedSizeUpdate>(&messageUpdate)) {
        setProcessedSize(update->processedSize);
    }
    else {
        return false;
    }
    return true;
}


AttachmentId Self::id() const {
    return m_id;
}


void Self::setId(AttachmentId id) {
    m_id = id;
}


QString Self::fileName() const {
    return m_fileName;
}


void Self::setFileName(QString fileName) {
    m_fileName = fileName;
}


QString Self::fingerprint() const {
    return m_fingerprint;
}


void Self::setFingerprint(QString fingerprint) {
    m_fingerprint = fingerprint;
}


QByteArray Self::decryptionKey() const {
    return m_decryptionKey;
}


void Self::setDecryptionKey(QByteArray decryptionKey) {
    m_decryptionKey = std::move(decryptionKey);
}


QByteArray Self::signature() const {
    return m_signature;
}


void Self::setSignature(QByteArray signature) {
    m_signature = std::move(signature);
}


qint64 Self::size() const {
    return m_size;
}


void Self::setSize(qint64 size) {
    m_size = size;
}


QUrl Self::remoteUrl() const {
    return m_remoteUrl;
}


void Self::setRemoteUrl(QUrl remoteUrl) {
    m_remoteUrl = remoteUrl;
}


QString Self::localPath() const {
    return m_localPath;
}


void Self::setLocalPath(QString localPath) {
    m_localPath = localPath;
}


qint64 Self::encryptedSize() const {
    return m_encryptedSize;
}


void Self::setEncryptedSize(qint64 encryptedSize) {
    m_encryptedSize = encryptedSize;
}


qint64 Self::processedSize() const {
    return m_processedSize;
}


void Self::setProcessedSize(qint64 processedSize) {
    m_processedSize = processedSize;
}


Self::UploadStage Self::uploadStage() const noexcept {
    return m_uploadStage;
}


void Self::setUploadStage(UploadStage uploadStage) {
    m_uploadStage = uploadStage;
}


Self::DownloadStage Self::downloadStage() const noexcept {
    return m_downloadStage;
}


void Self::setDownloadStage(DownloadStage downloadStage) {
    m_downloadStage = downloadStage;
}


QString Self::extrasToJson(const bool writeLocalPaths) const {
    Q_UNUSED(writeLocalPaths)
    return QString();
}

bool Self::readLocalFile(const QUrl &localUrl, QString &errorString) {
    if (!FileUtils::isValidUrl(localUrl)) {
        errorString = QObject::tr("File doesn't exist");
        return false;
    }

    const auto localFilePath = FileUtils::urlToLocalFile(localUrl);
    QFileInfo localInfo(localFilePath);
    if (!localInfo.exists()) {
        errorString = QObject::tr("File doesn't exist");
        return false;
    }
    if (localInfo.size() == 0) {
        errorString = QObject::tr("File is empty");
        return false;
    }

#ifdef VS_ANDROID
    const quint64 fileSize = VSQAndroid::getFileSize(localUrl);
#else
    const quint64 fileSize = localInfo.size();
#endif
    setId(AttachmentId(Utils::createUuid()));
    setSize(fileSize);
    setLocalPath(localInfo.absoluteFilePath());
    return true;
}
