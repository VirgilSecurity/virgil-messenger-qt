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

#ifndef VM_DECRYPTFILEOPERATION_H
#define VM_DECRYPTFILEOPERATION_H

#include "Messenger.h"
#include "Operation.h"
#include "UserId.h"

#include <QFileInfo>

namespace vm
{
class MessageOperation;

class DecryptFileOperation : public Operation
{
    Q_OBJECT

public:
    explicit DecryptFileOperation(QObject *parent, Messenger *messenger, const QString &sourcePath, const QString &destPath,
                                  const QByteArray& decryptionKey, const QByteArray& signature, const UserId &senderId);

    void run() override;

signals:
    void decrypted(const QFileInfo &file);

private:
    QPointer<Messenger> m_messenger;
    const QString m_sourcePath;
    const QString m_destPath;
    const QByteArray m_decryptionKey;
    const QByteArray m_signature;
    const UserId m_senderId;
};
}

#endif // VM_DECRYPTFILEOPERATION_H
