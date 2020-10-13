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

#include "database/core/TransactionScope.h"

#include "database/core/Database.h"

using namespace VSQ;

TransactionScope::TransactionScope(Database *database)
    : m_database(database)
{
    m_database->startTransaction();
}

TransactionScope::~TransactionScope()
{
    if (!m_finished) {
        qCWarning(lcDatabase) << "Transaction result wasn't used. Use result(), addAndFinish() to check result";
    }
    finish();
}

bool TransactionScope::add(bool result)
{
    if (m_finished) {
        qCCritical(lcDatabase) << "Transaction is already finished";
        m_result = false;
    }
    else if (m_result && !result) {
        m_result = false;
    }
    return m_result;
}

bool TransactionScope::addAndFinish(bool result)
{
    if (!add(result)) {
        return false;
    }
    return this->result();
}

bool TransactionScope::result()
{
    finish();
    return m_result;
}

void TransactionScope::finish()
{
    if (m_finished) {
        return;
    }
    if (m_result) {
        m_result = m_database->commitTransaction();
    }
    else {
        m_database->rollbackTransaction();
    }
    m_finished = true;
}
