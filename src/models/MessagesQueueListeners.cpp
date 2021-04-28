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

#include "MessagesQueueListeners.h"

#include "MessageOperationSource.h"

Q_LOGGING_CATEGORY(lcMessagesQueueListener, "messages-listener");

using namespace vm;

bool UniqueMessageDownloadOperationFilter::preRun(OperationSourcePtr source)
{
    const auto messageSource = dynamic_cast<MessageOperationSource *>(source.get());
    if (messageSource->download()) {
        const auto id = messageSource->message()->id();
        QMutexLocker locker(&m_mutex);
        if (!m_ids.contains(id)) {
            m_ids.push_back(id);
            return true;
        }
        qCDebug(lcMessagesQueueListener) << "Skipped duplicated download for message" << id;
        return false;
    }
    return true;
}

void UniqueMessageDownloadOperationFilter::postRun(OperationSourcePtr source)
{
    const auto messageSource = dynamic_cast<MessageOperationSource *>(source.get());
    if (messageSource->download()) {
        const auto id = messageSource->message()->id();
        QMutexLocker locker(&m_mutex);
        m_ids.removeAll(id);
    }
}
