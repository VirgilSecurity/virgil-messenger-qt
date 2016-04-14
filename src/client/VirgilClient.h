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

#include <string>
#include "../VirgilCryptoHelper.h"

class VirgilClient {
public:
    VirgilClient(VirgilCryptoHelper * crypto);
    virtual ~VirgilClient();
    
    /**
     * @brief Start client work.
     *
     * @param[in] serverAddr        server address.
     * @param[in] serverPort        server port.
     *
     * @return "true" if client has been started successfully.
     */
    bool start(const std::string & serverAddr, uint16_t serverPort);
    
private:
    VirgilCryptoHelper * m_crypto;  /**< Pointer to external crypto helper. TODO: Use shared pointer */
    std::string m_serverIdentity;   /**< Identity of server */
    int m_serverFd;                 /**< servers's socket descriptor */
    
    /**
     * @brief Prepare request to server. Create encrypted data, signature and packet.
     *
     * @param[in] data              Data for send.
     *
     * @return string with prepared request.
     */
    std::string prepareRequest(const std::string & data);
    
    /**
     * @brief Request server's identity.
     *
     * @return string with server's identity or empty in case of error.
     */
    std::string requestServerIdentity() const;
    
    /**
     * @brief Read data from socket with timeout.
     *
     * @param[in] timeoutMs         Timeout in milliseconds.
     *
     * @return string with received data.
     */
    std::string readWithTimeoutMs(size_t timeoutMs = 5000) const;
    
    /**
     * @brief Parse received data from client. Check signature, decode and send answer.
     *
     * @param[in] response          Data received from server.
     *
     * @return "true" if received data is correct.
     */
    bool parseResponse(const std::string & response) const;
};