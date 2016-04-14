/**
 * Copyright (C) 2016 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdint>
#include <cstdlib>
#include <string>

#include "../VirgilCryptoHelper.h"

class VirgilServer {
public:
    VirgilServer(VirgilCryptoHelper * crypto);
    virtual ~VirgilServer();
    
    /**
     * @brief Start port listening.
     *
     * @param[in] serverPort        port number for listen.
     *
     * @return "true" if listening has been started successfully.
     */
    bool listen(uint16_t serverPort);
    
private:
    /**
     * @brief Communication task which processed in new thread.
     *
     * @param[in] param             pointer to parameter which passed when thread was started.
     */
    static void * communicationTask(void * param);
    
    /**
     * @brief Process received data from client. Check signature, decode and send answer.
     *
     * @param[in] receivedData      Data received from client.
     *
     * @return string with response or empty string in case of processing error.
     */
    std::string processCommand(const std::string & receivedData);
    
    /**
     * @brief Prepare answer to client. Create encrypted data, signature and result packet.
     *
     * @param[in] recipientIdentity     Recipient identity (was received from client).
     * @param[in] data                  Data for encrypt, sign and etc.
     *
     * @return "true" if answer has been prepared successfully.
     */
    std::string prepareAnswer(const std::string & recipientIdentity, const std::string & data);

    VirgilCryptoHelper * m_crypto;      /**< Pointer to external crypto helper. TODO: Use shared pointer */
    int m_clientFd;                     /**< client's socket descriptor */
};