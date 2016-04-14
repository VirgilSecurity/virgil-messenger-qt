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

#include "VirgilClient.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>

/******************************************************************************/
VirgilClient::VirgilClient(VirgilCryptoHelper * crypto) : m_crypto(crypto) {
    
}

/******************************************************************************/
VirgilClient::~VirgilClient() {
    
}

/******************************************************************************/
std::string VirgilClient::prepareRequest(const std::string & data) {
    std::string _encryptedBase64;
    std::string _signature;
    
    if (m_crypto->encrypt(m_serverIdentity,
                          data,
                          _encryptedBase64)) {
        if (m_crypto->sign(_encryptedBase64, _signature)) {
            return m_crypto->ownIdentity() + " " +
            _encryptedBase64 + " " +
            _signature;
        }
    }
    
    return "";
}

/******************************************************************************/
bool VirgilClient::parseResponse(const std::string & response) const {
    // Parse received data
    std::vector<std::string> params;
    std::stringstream iss(response);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter(params));
    
    if (3 != params.size()) {
        std::cout << "Trere is no encrypted data." << std::endl;
        return false;
    }
    
    const std::string _senderIdentity(params[0]);
    const std::string _encryptedData(params[1]);
    const std::string _signatureData(params[2]);
    
    if (m_crypto->verify(_senderIdentity,
                         _encryptedData,
                         _signatureData)) {
        std::cout << std::endl << "Signature : CORRECT" << std::endl;
        
        std::string decryptedText;
        if (m_crypto->decrypt(_encryptedData, decryptedText)) {
            std::cout << "Received answer : '" << decryptedText << "'" << std::endl;
            return true;
        } else {
            std::cout << "Decryption : ERROR" << std::endl;
        }
    } else {
        std::cout << std::endl << "Signature : WRONG" << std::endl;
    }
    return false;
}

/******************************************************************************/
std::string VirgilClient::requestServerIdentity() const {
    const std::string _request("getIdentity");
    write(m_serverFd, _request.c_str(), _request.length());
    const std::string res(readWithTimeoutMs(5000));
    if (res.empty()) {
        std::cerr << "Can't get server's identity" << std::endl;
    } else {
        std::cout << "Server's identity : " << res << std::endl;
    }
    return res;
}

/******************************************************************************/
bool VirgilClient::start(const std::string & serverAddr, uint16_t serverPort) {
    // Open socket
    m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(m_serverFd < 0) {
        std::cerr << "Cannot open socket" << std::endl;
        return false;
    }
    
    struct hostent * _server(gethostbyname(serverAddr.c_str()));
    if(!_server) {
        std::cerr << "Host does not exist" << std::endl;
        return false;
    }
    
    struct sockaddr_in serverSockAddr;
    memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    serverSockAddr.sin_family = AF_INET;
    
    memcpy(&serverSockAddr.sin_addr.s_addr, _server->h_addr, _server->h_length);
    
    serverSockAddr.sin_port = htons(serverPort);
    
    if (0 > connect(m_serverFd, (struct sockaddr *) &serverSockAddr, sizeof(serverSockAddr))) {
        std::cerr << "Cannot connect!" << std::endl;
        return 0;
    }
    
    m_serverIdentity = requestServerIdentity();
    
    //send stuff to server
    while (true) {
        char inBuf[2048];
        
        std::cout << std::endl
        << "---------------------------"
        << std::endl
        << "Client input: ";
        
        std::cin.getline(inBuf, sizeof(inBuf));
        
        const std::string _request(prepareRequest(std::string(inBuf)));
        if (_request.size()) {
            write(m_serverFd, _request.c_str(), _request.length());
            const std::string _response(readWithTimeoutMs(5000));
            if (!_response.empty()) {
                parseResponse(_response);
            }
        } else {
            std::cerr << "Cannot prepare encrypted request!" << std::endl;
        }
    }
    return false;
}

/******************************************************************************/
std::string VirgilClient::readWithTimeoutMs(size_t timeoutMs) const {
    struct pollfd fd;
    fd.fd = m_serverFd;
    fd.events = POLLIN;
    
    if (0 <= poll(&fd, 1, static_cast <int> (timeoutMs))) {
        char buf[10 * 1024];
        memset(buf, 0, sizeof(buf));
        recv(m_serverFd, buf, sizeof(buf), 0);
        return std::string(buf);
    }
    return "";
}
