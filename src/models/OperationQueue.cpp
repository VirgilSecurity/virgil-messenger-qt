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

#include "OperationQueue.h"

#include <QtConcurrent>

#include "Operation.h"

using namespace vm;
using Self = OperationQueue;

Self::OperationQueue(const QLoggingCategory &category, QObject *parent)
    : QObject(parent)
    , m_category(category)
    , m_threadPool(new QThreadPool(this))
{
    qRegisterMetaType<vm::OperationSourcePtr>("OperationSourcePtr");
    qRegisterMetaType<vm::OperationQueue::PostFunction>("PostFunction");

    m_threadPool->setMaxThreadCount(5);

    connect(this, &OperationQueue::operationFailed, this, &OperationQueue::onOperationFailed);
}

Self::~OperationQueue()
{
    stop();
}

void Self::start()
{
    m_isStopped = false;
}

void Self::run()
{
    OperationSources sources;
    std::swap(sources, m_sources);
    for (auto &source : sources) {
        runSource(source);
    }
}

void Self::stop()
{
    qCDebug(m_category) << "stop";
    m_sources.clear();
    for (auto listener : m_listeners) {
        listener->clear();
    }
    m_isStopped = true;
    emit stopRequested(QPrivateSignal());
    m_threadPool->waitForDone();
}

void Self::addSource(OperationSourcePtr source)
{
    addSourceImpl(std::move(source), true);
}

void Self::addListener(OperationQueueListenerPtr listener)
{
    m_listeners.push_back(listener);
    connect(listener, &OperationQueueListener::notificationCreated, this, &Self::notificationCreated);
}

void Self::addSourceImpl(OperationSourcePtr source, const bool run)
{
    if (!source->isValid()) {
        return;
    }
    m_sources.push_back(std::move(source));
    if (run) {
        this->run();
    }
}

void Self::runSource(OperationSourcePtr source)
{
    auto threadPool = (source->priority() == OperationSource::Priority::Highest) ? QThreadPool::globalInstance() : &*m_threadPool;
    QtConcurrent::run(threadPool, [=, source = std::move(source)]() {
        // Skip if queue is stopped
        if (m_isStopped) {
            qCDebug(m_category) << "Operation was skipped because queue was stopped";
            return;
        }
        // Pre-run listeners
        for (auto listener : m_listeners) {
            if (!listener->preRun(source)) {
                return;
            }
        }
        // Perform operation
        auto op = createOperation(source);
        op->start();
        op->waitForDone();
        if (op->status() == Operation::Status::Failed) {
            emit operationFailed(source, QPrivateSignal());
        }
        else if (op->status() == Operation::Status::Invalid) {
            invalidateOperation(source);
        }
        op->drop(true);
        // Post-run listeners
        for (auto listener : m_listeners) {
            listener->postRun(source);
        }
    });
}

void Self::onOperationFailed(OperationSourcePtr source)
{
    if (source->attemptCount() < maxAttemptCount()) {
        qCDebug(m_category) << "Enqueued failed operation source:" << source->toString();
        source->incAttemptCount();
        addSourceImpl(std::move(source), false);
    }
    else if (source->attemptCount() == maxAttemptCount()) {
        qCDebug(m_category) << "Failed operation was invalidated:" << source->toString();
        invalidateOperation(source);
    }
}
