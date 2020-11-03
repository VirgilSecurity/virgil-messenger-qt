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

#include "Core.h"

#include <virgil/iot/messenger/messenger.h>

#include "Utils.h"

Q_LOGGING_CATEGORY(lcCore, "core")

using namespace vm;
using namespace VirgilIoTKit;

bool Core::findContact(const Contact::Id &contactId)
{
    return VS_CODE_OK == vs_messenger_virgil_search(contactId.toStdString().c_str());
}

int Core::bufferSizeForEncryption(const int rawSize)
{
    // TODO(fpohtmeh): use function rawSize * 3 + 2180
    return 5 * rawSize + 5000;
}

int Core::bufferSizeForDecryption(const int encryptedSize)
{
    return encryptedSize;
}

Optional<Message> Core::decryptMessage(const Contact::Id &sender, const QString &message)
{
    const int decryptedMsgSzMax = bufferSizeForDecryption(message.size());
    QByteArray decryptedMessage = QByteArray(decryptedMsgSzMax + 1, 0x00);
    size_t decryptedMessageSz = 0;

    qCDebug(lcCore) << "Sender:" << sender;
    qCDebug(lcCore) << "Encrypted message:" << message.length() << "bytes";

    // Decrypt message
    if (VS_CODE_OK != vs_messenger_virgil_decrypt_msg(
                sender.toStdString().c_str(),
                message.toStdString().c_str(),
                (uint8_t *)decryptedMessage.data(), decryptedMsgSzMax,
                &decryptedMessageSz)) {
        qCWarning(lcCore) << "Received message cannot be decrypted";
        return NullOptional;
    }

    // Adjust buffer with a decrypted data
    decryptedMessage.resize((int)decryptedMessageSz);

    Message msg = Utils::messageFromJson(decryptedMessage);
    msg.authorId = sender;
    return msg;
}

Optional<QString> Core::encryptMessage(const Message &message, const Contact::Id &recipient)
{
    // Create JSON-formatted message to be sent
    const QString internalJson = Utils::messageToJson(message);
    qCDebug(lcCore) << "Recipient:" << recipient;
    qCDebug(lcCore) << "Json for encryption:" << internalJson;

    // Encrypt message
    const auto plaintext = internalJson.toStdString();
    const size_t _encryptedMsgSzMax = Core::bufferSizeForEncryption(plaintext.size());
    uint8_t encryptedMessage[_encryptedMsgSzMax];
    size_t encryptedMessageSz = 0;

    if (VS_CODE_OK != vs_messenger_virgil_encrypt_msg(
                     recipient.toStdString().c_str(),
                     reinterpret_cast<const uint8_t*>(plaintext.c_str()),
                     plaintext.length(),
                     encryptedMessage,
                     _encryptedMsgSzMax,
                     &encryptedMessageSz)) {
        qCWarning(lcCore) << "Cannot encrypt message to be sent";
        return NullOptional;
    }
    return QString::fromLatin1(reinterpret_cast<char*>(encryptedMessage), encryptedMessageSz);
}

bool Core::decryptFile(const QString &encPath, const QString &path, const Contact::Id &recipientId)
{
    // Read
    QFile encFile(encPath);
    if (!encFile.exists()) {
        qCCritical(lcCore) << "Source file doesn't exist";
        return false;
    }
    if (!encFile.open(QFile::ReadOnly)) {
        qCCritical(lcCore) << "Source file can't be opened";
        return false;
    }
    const auto encBytes = encFile.readAll();
    qCDebug(lcCore) << "Read" << encBytes.size() << "bytes";
    encFile.close();
    if (encBytes.size() == 0) {
        qCDebug(lcCore) << "Empty file was skipped";
        return false;
    }

    // Decrypt
    std::vector<char> bytes(Core::bufferSizeForDecryption(encBytes.size()));
    size_t bytesSize = 0;
    const auto code = vs_messenger_virgil_decrypt_msg(
                recipientId.toStdString().c_str(),
                encBytes.data(),
                reinterpret_cast<uint8_t*>(bytes.data()), bytes.size(), &bytesSize);
    if (VS_CODE_OK != code)
    {
        qCCritical(lcCore) << "Cannot decrypt file:" << encPath << "code:" << code;
        return false;
    }
    bytes.resize(bytesSize);

    // Write
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        qCCritical(lcCore) << "Destination file can't be opened";
        return false;
    }
    file.write(bytes.data(), bytesSize);
    file.close();
    qCDebug(lcCore) << "File decrypted:" << path << "size:" << QFileInfo(file).size();
    return true;
}

bool Core::encryptFile(const QString &path, const QString &encPath, const Contact::Id &recipientId)
{
    // Read
    QFile file(path);
    if (!file.exists()) {
        qCCritical(lcCore) << "Source file doesn't exist";
        return false;
    }
    if (!file.open(QFile::ReadOnly)) {
        qCCritical(lcCore) << "Source file can't be opened";
        return false;
    }
    const auto bytes = file.readAll();
    qCDebug(lcCore) << "Read" << bytes.size() << "bytes";
    if (bytes.size() == 0) {
        qCDebug(lcCore) << "Empty file was skipped";
        return false;
    }
    file.close();

    // Encrypt
    std::vector<char> encBytes(Core::bufferSizeForEncryption(bytes.size()));
    size_t encBytesSize = 0;
    const auto code = vs_messenger_virgil_encrypt_msg(
                recipientId.toStdString().c_str(),
                reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size(),
                reinterpret_cast<uint8_t*>(encBytes.data()), encBytes.size(), &encBytesSize);
    if (VS_CODE_OK != code)
    {
        qCCritical(lcCore) << "Cannot encrypt file:" << path << "code:" << code;
        return false;
    }
    encBytes.resize(encBytesSize);
    encBytes.push_back(0);

    // Write
    QFile encFile(encPath);
    if (!encFile.open(QFile::WriteOnly)) {
        qCCritical(lcCore) << "Destination file can't be opened";
        return false;
    }
    encFile.write(encBytes.data(), encBytes.size());
    encFile.close();
    qCDebug(lcCore) << "File encrypted:" << encPath << "size:" << QFileInfo(encFile).size();
    return true;
}
