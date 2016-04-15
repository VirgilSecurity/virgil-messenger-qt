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

#include <signal.h>
#include <iostream>
#include "VirgilServer.h"
#include "../VirgilCryptoHelper.h"

void instalSignalHandler();

VirgilCryptoHelper * cryptoHelper(0);
VirgilServer * server(0);

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        std::cout << "Usage : virgil_server <port> <email> <private key directory>" << std::endl;
        return 0;
    }
    
    const uint16_t _portNum(atoi(argv[1]));
    
    if(_portNum < 2000 || _portNum > 65535) {
        std::cout << "Please enter a port number between 2000 - 65535" << std::endl;
        return 0;
    }
    
    const std::string _keysIdentity(argv[2]);
    const std::string _privateKeyDir(argv[3]);
    
    instalSignalHandler();

    std::cout << "Virgil Server Start ..." << std::endl;
    
    // Prepare Crypto Helper
    VirgilCryptoHelper * cryptoHelper(new VirgilCryptoHelper);
    if (cryptoHelper->init(_keysIdentity, _privateKeyDir)) {
        
        // Start server work
        server = new VirgilServer(cryptoHelper);
        server->listen(_portNum);
    } else {
        std::cout << "terminate server" << std::endl;
    }

    delete server;
    delete cryptoHelper;
    
    std::cerr << std::endl << "Virgil Server Stopped" << std::endl;

    return 0;
}

/**
 * @brief Handler for Ctrl-C signal.
 * Correct stop of the server on Ctrl-C.
 */
void ctrlC(int s) {
    if (server) {
        server->stop();
    }
}

/**
 * @brief Install handler for Ctrl-C event.
 */
void instalSignalHandler() {
    struct sigaction sigIntHandler;
    
    sigIntHandler.sa_handler = ctrlC;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    
    sigaction(SIGINT, &sigIntHandler, 0);
}
