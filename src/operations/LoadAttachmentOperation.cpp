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

#include "operations/LoadAttachmentOperation.h"

#include "operations/MessageOperation.h"
#include "MessageUpdate.h"
#include "Settings.h"

using namespace vm;

LoadAttachmentOperation::LoadAttachmentOperation(MessageOperation *parent, const Settings *settings)
    : NetworkOperation(parent), m_settings(settings)
{
    setName(QLatin1String("LoadAttachment"));

    if (settings->timeProfilerEnabled()) {
        setTimeProfiler(&m_loadTimeProfiler);
        m_loadTimeProfiler.start();
    }

    connect(this, &LoadAttachmentOperation::totalProgressChanged, [parent](quint64 bytesLoaded, quint64 bytesTotal) {
        Q_UNUSED(bytesTotal)
        MessageAttachmentProcessedSizeUpdate update;
        update.messageId = parent->message()->id();
        update.attachmentId = parent->message()->contentAsAttachment()->id();
        update.processedSize = bytesLoaded;
        parent->updateMessage(update);
    });
}

void LoadAttachmentOperation::startLoadOperation(quint64 bytesTotal)
{
    m_previousBytesTotal = m_bytesTotal;
    m_bytesTotal += bytesTotal;
    updateTotalProgress();
}

void LoadAttachmentOperation::setLoadOperationProgress(quint64 bytesLoaded)
{
    m_currentBytesLoaded = bytesLoaded;
    updateTotalProgress();
}

void LoadAttachmentOperation::cleanup()
{
    Operation::cleanup();
    m_bytesTotal = 0;
    m_previousBytesTotal = 0;
    m_currentBytesLoaded = 0;
}

void LoadAttachmentOperation::updateTotalProgress()
{
    emit totalProgressChanged(m_previousBytesTotal + m_currentBytesLoaded, m_bytesTotal);
}
