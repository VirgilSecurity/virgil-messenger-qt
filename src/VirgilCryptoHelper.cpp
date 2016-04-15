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

#include <iostream>
#include <fstream>
#include <virgil/sdk/models/IdentityModel.h>
#include <virgil/sdk/models/PublicKeyModel.h>
#include <virgil/sdk/dto/Identity.h>
#include <virgil/sdk/io/Marshaller.h>
#include <virgil/sdk/Credentials.h>
#include <virgil/sdk/ServiceUri.h>
#include <virgil/crypto/VirgilKeyPair.h>
#include <virgil/crypto/VirgilCipher.h>
#include <virgil/crypto/VirgilSigner.h>
#include <virgil/crypto/foundation/VirgilBase64.h>

using namespace virgil::sdk::io;
using namespace virgil::sdk::dto;
using namespace virgil::crypto;
using namespace virgil::crypto::foundation;

#include "VirgilCryptoHelper.h"

const std::string VirgilCryptoHelper::_applicationToken("eyJpZCI6ImExNjVlMDhiLWNiZjUtNDk4OC05NjIyLTY5Y2M3NDMzMzA4ZSIsImFwcGxpY2F0aW9uX2NhcmRfaWQiOiIzMmU5MWE0YS1mMmVjLTRhZWMtYmYzNy0xMTU3NmQxZTYwNjYiLCJ0dGwiOi0xLCJjdGwiOi0xLCJwcm9sb25nIjowfQ==.MFgwDQYJYIZIAWUDBAICBQAERzBFAiAji3nsTWqHVJPJCvZ9snroRXmz+QjmF/jCc+BvZc4JdwIhAIEJ0GqjaLOJ6A6sTFFoslbxuvOO5n5qYXS1UW1VGGF4");

/******************************************************************************/
VirgilCryptoHelper::VirgilCryptoHelper() :
m_valid(false), m_servicesHub(_applicationToken, ServiceUri("https://identity-stg.virgilsecurity.com",
                                                            "https://keys-stg.virgilsecurity.com",
                                                            "https://keys-private-stg.virgilsecurity.com")) {
}

/******************************************************************************/
VirgilCryptoHelper::~VirgilCryptoHelper() {
    
}

/******************************************************************************/
bool VirgilCryptoHelper::init(const std::string & identity,
                              const std::string & privateKeyDir) {
    // Prepare private key file name
    m_privateKeyFile = privateKeyDir + identity + ".key";
    m_privateKeyFile.replace(m_privateKeyFile.find("@"), 1, "_");
    
    // Search for exist virgil card
    if (getCard(identity, m_currentCard) && loadPrivateKey()) {
        std::cout << "Virgil keys were been loaded" << std::endl;
        m_valid = true;
    } else {
        if (createCard(identity, privateKeyDir)) {
            std::cout << "Virgil keys were been created" << std::endl;
            m_valid = true;
        } else {
            std::cout << "ERROR: Can't create keys" << std::endl;
        }
    }
    
    return m_valid;
}

/******************************************************************************/
bool VirgilCryptoHelper::isValid() const {
    return m_valid;
}

/******************************************************************************/
std::string VirgilCryptoHelper::ownIdentity() const {
    return m_currentCard.getCardIdentity().getValue();
}

/******************************************************************************/
bool VirgilCryptoHelper::getCard(const std::string & identity, CardModel & card) {
    try {
        // Get card
        Identity cardIdentity(identity, IdentityModel::Type::Email);
        const bool _includeUnconfirmed(true);
        std::vector<CardModel> _foundCards(m_servicesHub.card().search(cardIdentity, _includeUnconfirmed));
        if (_foundCards.empty()) return false;
        
        // Sort cards by time and get newest
        std::sort(_foundCards.begin(), _foundCards.end(),
                  [](const CardModel & a, const CardModel & b) -> bool {
                      return a.getCreatedAt() > b.getCreatedAt();
                  });
        card = _foundCards.front();
        
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::createCard(const std::string & identity,
                                    const std::string & privateKeyDir) {
    try {
        // Create Key Pair
        VirgilKeyPair keyPair(VirgilKeyPair::generate(VirgilKeyPair::Type_EC_M255));
        
        // Push Card to Virgil Public key Service
        Identity cardIdentity(identity, IdentityModel::Type::Email);
        Credentials credentials(keyPair.privateKey());
        
        m_currentCard = m_servicesHub
        .card()
        .create(cardIdentity, keyPair.publicKey(), credentials, {{"key", "value"}});
        
        const std::string _cardStr(Marshaller<CardModel>::toJson<4>(m_currentCard));
        std::cout << "Virgil Card:" << std::endl;
        std::cout << _cardStr << std::endl;
        
        // Save private key to local drive
        m_privateKey = keyPair.privateKey();
        savePrivateKey();
        
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::savePrivateKey() {
    try {
        std::ofstream file;
        file.open (m_privateKeyFile);
        file << virgil::crypto::bytes2str(m_privateKey);
        file.close();
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::loadPrivateKey() {
    try {
        std::ifstream file(m_privateKeyFile, std::ios::in | std::ios::binary);
        if (!file) {
            throw std::runtime_error("can not read file: " + m_privateKeyFile);
        }
        
        std::copy(std::istreambuf_iterator<char>(file),
                  std::istreambuf_iterator<char>(),
                  std::back_inserter(m_privateKey));
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::encrypt(const std::string & recipientIdentity,
                                 const std::string & textForEncrypt,
                                 std::string & encryptedBase64) {
    try {
        CardModel recipientCard;
        if (!getCard(recipientIdentity, recipientCard)) return false;

        VirgilCipher cipher;
        cipher.addKeyRecipient(str2bytes(recipientCard.getId()), recipientCard.getPublicKey().getKey());
        encryptedBase64 = VirgilBase64::encode(cipher.encrypt(str2bytes(textForEncrypt), true));
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::decrypt(const std::string & encryptedBase64,
                                 std::string & decryptedText) {
    try {
        VirgilByteArray decryptedData =
        VirgilCipher().decryptWithKey(VirgilBase64::decode(encryptedBase64),
                                      str2bytes(m_currentCard.getId()),
                                      m_privateKey);
        
        decryptedText = bytes2str(decryptedData);
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::sign(const std::string & textForSign,
                              std::string & signature) {
    try {
        const VirgilByteArray _signatureData(VirgilSigner().sign(str2bytes(textForSign), m_privateKey));
        signature = VirgilBase64::encode(_signatureData);
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    return true;
}

/******************************************************************************/
bool VirgilCryptoHelper::verify(const std::string & recipientIdentity,
                                const std::string & textForSign,
                                const std::string & signature) {
    try {
        CardModel recipientCard;
        if (!getCard(recipientIdentity, recipientCard)) return false;
        
        return VirgilSigner().verify(str2bytes(textForSign),
                                     VirgilBase64::decode(signature),
                                     recipientCard.getPublicKey().getKey());
    } catch (std::exception& exception) {
        std::cerr << exception.what() << std::endl;
    }
    
    return false;
}
