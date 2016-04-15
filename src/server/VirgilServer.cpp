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

#include "VirgilServer.h"

#include <iterator>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <vector>

/******************************************************************************/
VirgilServer::VirgilServer(VirgilCryptoHelper * crypto) :
m_crypto(crypto),
m_clientFd(-1),
m_serverFd(-1),
m_serverThread(0) {
}

/******************************************************************************/
VirgilServer::~VirgilServer() {
    
}

/******************************************************************************/
std::string VirgilServer::processCommand(const std::string & receivedData) {
    // Parse received data
    std::vector<std::string> params;
    std::stringstream iss(receivedData);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter(params));
    
    if (3 != params.size()) {
        std::cout << "Trere is no encrypted data." << std::endl;
        return "";
    }
    
    const std::string _senderIdentity(params[0]);
    const std::string _encryptedData(params[1]);
    const std::string _signatureData(params[2]);
    
    std::cout << "Sender : " << _senderIdentity << std::endl;
    std::cout << "Encrypted data : " << _encryptedData << std::endl;
    std::cout << "Signature : " << _signatureData << std::endl;
    
    // Verify data
    if (m_crypto->verify(_senderIdentity,
                         _encryptedData,
                         _signatureData)) {
        std::cout << "Signature : CORRECT" << std::endl;
        
        // Decrypt data
        std::string decryptedText;
        if (m_crypto->decrypt(_encryptedData, decryptedText)) {
            std::cout << std::endl << "Decrypted data : " << decryptedText << std::endl;
            
            // Received data is correct, so let's send encrypted answer to client
            return prepareAnswer(_senderIdentity, "<Data received by server : '" + decryptedText + "'>");
        } else {
            std::cout << "Decryption : ERROR" << std::endl;
        }
    } else {
        std::cout << "Signature : WRONG" << std::endl;
    }
    return "";
}

/******************************************************************************/
std::string VirgilServer::prepareAnswer(const std::string & recipientIdentity,
                                        const std::string & data) {
    std::string _encryptedBase64;
    std::string _signature;
    
    if (m_crypto->encrypt(recipientIdentity,
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
void * VirgilServer::serverTask(void * param) {
    std::cout << std::endl << "Thread No: " << pthread_self() << std::endl;
    
    VirgilServer * obj(reinterpret_cast<VirgilServer *>(param));
    
    struct sockaddr_in clntAdd;
    socklen_t len(sizeof(clntAdd));
    
    while (true) {
        std::cout << "Listening" << std::endl;
        
        const int _newFd(accept(obj->m_serverFd, (struct sockaddr *)&clntAdd, &len));
        
        if (_newFd < 0) {
            std::cerr << "Cannot accept connection" << std::endl;
            break;
        } else {
            std::cout << "Connection successfull" << std::endl;
        }
        
        if (-1 == obj->m_clientFd) {
            obj->m_clientFd = _newFd;
            obj->communicationTask();
        } else {
            std::cout << "Close new connection" << std::endl;
            close(_newFd);
        }
    }
    
    return 0;
}

/******************************************************************************/
void VirgilServer::communicationTask() {
    char receiveBuf[2048];
    
    while (true) {
        memset(receiveBuf, 0, sizeof(receiveBuf));
        
        // Receive data
        if (0 >= read(m_clientFd, receiveBuf, sizeof(receiveBuf) - 1)) {
            std::cout << "Read socket error";
            m_clientFd = -1;
            break;
        } else {
            const std::string _stdReceive(receiveBuf);
            std::cout << std::endl << "Server received: " << _stdReceive << std::endl;
            
            if (_stdReceive == "getIdentity") {
                // Send of identity
                send(m_clientFd,
                     m_crypto->ownIdentity().c_str(),
                     m_crypto->ownIdentity().length(),
                     0);
                continue;
            }
            
            // Process data
            const std::string _answer(processCommand(_stdReceive));
            if (_answer.empty()) continue;
            
            // Send answer
            std::cout << "Server answer has been sent." << std::endl;
            send(m_clientFd, _answer.c_str(), _answer.length(), 0);
        }
    }
    close(m_clientFd);
    m_clientFd = -1;
    return;
}

/******************************************************************************/
bool VirgilServer::listen(uint16_t serverPort) {
    //create socket
    m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(m_serverFd < 0) {
        std::cerr << "Cannot open socket" << std::endl;
        return false;
    }
    
    struct sockaddr_in svrAdd;
    memset(&svrAdd, 0, sizeof(svrAdd));
    
    svrAdd.sin_family = AF_INET;
    svrAdd.sin_addr.s_addr = INADDR_ANY;
    svrAdd.sin_port = htons(serverPort);
    
    //bind socket
    if (::bind(m_serverFd, (struct sockaddr *) &svrAdd, sizeof(svrAdd)) < 0) {
        std::cerr << "Cannot bind" << std::endl;
        return false;
    }
    
    ::listen(m_serverFd, 5);
    
    // Start server's thread
    pthread_create(&m_serverThread, 0, VirgilServer::serverTask, this);
    
    // Wait for thread finish
    pthread_join(m_serverThread, 0);
    
    // Close socket
    close(m_serverFd);
    
    return true;
}

/******************************************************************************/
void VirgilServer::stop() {
    if (m_serverThread) {
        pthread_cancel(m_serverThread);
    }
}
