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

#include "operations/Operation.h"

Q_LOGGING_CATEGORY(lcOperation, "operation")

using namespace vm;

Operation::Operation(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{}

Operation::~Operation()
{
    cleanupOnce();
}

void Operation::start()
{
    if (!setStatus(Status::Started)) {
        return;
    }

    qCDebug(lcOperation) << "Starting operation:" << fullName();
    m_cleanedUp = false;
    if (hasChildren() || populateChildren()) {
        m_children.front()->start();
    }
    else {
        run();
    }
}

void Operation::drop()
{
    qCDebug(lcOperation) << "Drop operation:" << fullName();
    dropChildren();
    cleanupOnce();
    deleteLater();
}

void Operation::dropChildren()
{
    while (!m_children.empty()) {
        m_children.front()->drop();
        m_children.pop_front();
    }
}

QString Operation::name() const
{
    return m_name;
}

void Operation::setName(const QString &name)
{
    m_name = name;
}

QString Operation::fullName() const
{
    if (auto parentOp = dynamic_cast<Operation *>(parent())) {
        return parentOp->fullName() + QChar('/') + name();
    }
    return name();
}

Operation::Status Operation::status() const
{
    return m_status;
}

void Operation::appendChild(Operation *child)
{
    connectChild(child);
    m_children.push_back(child);
}

void Operation::prependChild(Operation *child)
{
    connectChild(child);
    m_children.push_front(child);
}

bool Operation::hasChildren() const
{
    return !m_children.empty();
}

Operation *Operation::firstChild()
{
    return hasChildren() ? m_children.front() : nullptr;
}

Operation *Operation::lastChild()
{
    return hasChildren() ? m_children.back() : nullptr;
}

void Operation::setRepeatable(bool repeatable)
{
    m_repeatable = repeatable;
}

void Operation::fail()
{
    if (setStatus(Status::Failed)) {
        cleanupOnce();
    }
}

void Operation::invalidate()
{
    setStatus(Status::Invalid);
    drop();
}

void Operation::finish()
{
    setStatus(Status::Finished);
}

void Operation::cleanupOnce()
{
    if (hasChildren()) {
        return;
    }
    if (m_cleanedUp) {
        return;
    }
    m_cleanedUp = true;
    cleanup();
}

void Operation::run()
{
    qCDebug(lcOperation) << "Empty operation runs. Skipped";
    finish();
}

void Operation::cleanup()
{
    qCDebug(lcOperation) << "Cleanup operation:" << fullName();
}

bool Operation::populateChildren()
{
    return hasChildren();
}

void Operation::connectChild(Operation *child)
{
    child->setParent(this);
    connect(child, &Operation::failed, this, &Operation::fail);
    connect(child, &Operation::invalidated, this, &Operation::onChildInvalidated);
    connect(child, &Operation::finished, this, &Operation::startNextChild);
    connect(child, &Operation::notificationCreated, this, &Operation::notificationCreated);
}

bool Operation::setStatus(const Operation::Status &status)
{
    if (m_status == status) {
        return false;
    }

    switch (status)
    {
    case Status::Created:
        return false;
    case Status::Started:
        if (m_status != Status::Created && m_status != Status::Failed && !(m_repeatable && m_status == Status::Finished)) {
            qCWarning(lcOperation) << "Unable to start operation";
            return false;
        }
        m_status = status;
        emit started();
        return true;
    case Status::Failed:
        if (m_status != Status::Started) {
            qCWarning(lcOperation) << "Unable to fail operation";
            return false;
        }
        m_status = status;
        emit failed();
        return true;
    case Status::Invalid:
        if (m_status != Status::Started) {
            qCWarning(lcOperation) << "Unable to invalidate operation";
            return false;
        }
        m_status = status;
        emit invalidated();
        return true;
    case Status::Finished:
        if (m_status != Status::Started) {
            qCWarning(lcOperation) << "Unable to finish operation";
            return false;
        }
        m_status = status;
        emit finished();
        return true;
    default:
        return false;
    }
}

void Operation::startNextChild()
{
    // Drop used children
    while (!m_children.empty()) {
        auto child = m_children.front();
        if (child->status() == Operation::Status::Finished || child->status() == Operation::Status::Invalid) {
            child->drop();
            m_children.pop_front();
        }
        else {
            break;
        }
    }
    // Find not started children
    for (auto &child : m_children) {
        if (child->status() != Operation::Status::Started) {
            child->start();
            return;
        }
    }
    //
    setStatus(Operation::Status::Finished);
}

void Operation::onChildInvalidated()
{
    if (m_repeatable) {
        startNextChild();
    }
    else {
        invalidate();
    }
}
