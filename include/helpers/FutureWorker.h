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

#ifndef VM_FUTUREWORKER_H
#define VM_FUTUREWORKER_H

#include <QFutureWatcher>

namespace vm {

class FutureWorker
{
public:
    template<typename FutureType, typename ResultType = decltype(typename FutureType::result())>
    static void run(FutureType future, std::function<void(ResultType)> resultHandler)
    {
        using FutureWatcher = QFutureWatcher<ResultType>;

        auto futureWatcher = new FutureWatcher();
        QObject::connect(futureWatcher, &FutureWatcher::finished,
                         [futureWatcher, future = std::move(future), resultHandler = std::move(resultHandler)]() {
                             futureWatcher->deleteLater();
                             resultHandler(future.result());
                         });
        futureWatcher->setFuture(future);
    }

    template<typename FutureType, typename ResultHandler>
    static void run(FutureType future, ResultHandler resultHandler)
    {
        using ResultType = decltype(future.result());

        std::function<void(ResultType)> f = std::move(resultHandler);

        run(std::move(future), std::move(f));
    }
};
} // namespace vm

#endif // VM_FUTUREWORKER_H
