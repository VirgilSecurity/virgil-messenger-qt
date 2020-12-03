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

#ifndef VM_OPERATION_H
#define VM_OPERATION_H

#include <deque>

#include <QObject>

#include "VSQCommon.h"

Q_DECLARE_LOGGING_CATEGORY(lcOperation)

namespace vm
{
class Operation : public QObject
{
    Q_OBJECT

public:
    enum class Status
    {
        Created,
        Started,
        Failed,
        Invalid,
        Finished
    };

    Operation(const QString &name, QObject *parent);
    ~Operation() override;

    void start();
    void stop();
    void waitForDone();

    // Cleanup and deleteLater operation with children
    void drop();
    void dropChildren();

    QString name() const;
    void setName(const QString &name);
    QString fullName() const;
    Status status() const;

    void appendChild(Operation *child);
    bool hasChildren() const;

signals:
    void started();
    void failed();
    void invalidated();
    void finished();

    void notificationCreated(const QString &notification, const bool error);

protected:
    void fail();
    void invalidate();
    void invalidate(const QString &notification);
    void finish();

    void cleanupOnce();

    virtual bool preRun();
    virtual void run();
    virtual void cleanup();
    virtual bool populateChildren();
    virtual void connectChild(Operation *child);

private:
    bool setStatus(const Status &status);

    void startNextChild();

    QString m_name;
    Status m_status = Status::Created;

    std::deque<Operation *> m_children;
    bool m_cleanedUp = false;
};
}

#endif // VM_OPERATION_H
