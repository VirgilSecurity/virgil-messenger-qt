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

#include "operations/NetworkOperation.h"

#include "models/FileLoader.h"

using namespace vm;

NetworkOperation::NetworkOperation(QObject *parent, FileLoader *fileLoader)
    : Operation(QLatin1String("Network"), parent)
    , m_fileLoader(fileLoader)
    , m_isOnline(fileLoader->isServiceFound())
{
    connect(fileLoader, &FileLoader::serviceFound, this, &NetworkOperation::setIsOnline);
}

NetworkOperation::NetworkOperation(NetworkOperation *parent)
    : NetworkOperation(parent, parent->fileLoader())
{
}

FileLoader *NetworkOperation::fileLoader()
{
    return m_fileLoader;
}

bool NetworkOperation::isOnline() const
{
    return m_isOnline;
}

bool NetworkOperation::preRun()
{
    if (!m_isOnline && !hasChildren() && status() == Operation::Status::Started) {
        qCDebug(lcOperation) << "Operation is failed because network is offline";
        fail();
        return false;
    }
    return Operation::preRun();
}

void NetworkOperation::setIsOnline(bool isOnline)
{
    m_isOnline = isOnline;
    preRun();
}
