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

#ifndef VM_OPERATIONQUEUE_H
#define VM_OPERATIONQUEUE_H

#include <QLoggingCategory>
#include <QPointer>

#include "OperationSource.h"

class QThreadPool;

namespace vm
{
class Operation;

class OperationQueue : public QObject
{
    Q_OBJECT

public:
    using PostFunction = OperationSource::PostFunction;

    explicit OperationQueue(const QLoggingCategory &category, QObject *parent);
    ~OperationQueue() override;

    void start();
    void run();
    void stop();

    void addSource(OperationSourcePtr source);

signals:
    void stopRequested(QPrivateSignal);
    void operationFailed(OperationSourcePtr source, QPrivateSignal);
    void notificationCreated(const QString &notification, const bool error);

protected:
    virtual Operation *createOperation(OperationSourcePtr source) = 0;
    virtual void invalidateOperation(OperationSourcePtr source) = 0;

private:
    void addSourceImpl(OperationSourcePtr source, const bool run);
    void runSource(OperationSourcePtr source);

    void onOperationFailed(OperationSourcePtr source);

    const QLoggingCategory &m_category;

    QPointer<QThreadPool> m_threadPool;
    std::atomic_bool m_isStopped = false;
    std::vector<OperationSourcePtr> m_sources;
};
}

Q_DECLARE_METATYPE(vm::OperationSourcePtr);
Q_DECLARE_METATYPE(vm::OperationQueue::PostFunction);

#endif // VM_OPERATIONQUEUE_H
