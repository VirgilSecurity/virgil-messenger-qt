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

#ifndef VirgilCryptoHelper_h
#define VirgilCryptoHelper_h

#include <stdio.h>
#include <virgil/sdk/ServicesHub.h>
#include <virgil/sdk/models/CardModel.h>
#include <virgil/crypto/VirgilByteArray.h>

using namespace virgil::sdk;
using namespace virgil::sdk::models;
using namespace virgil::crypto;

class VirgilCryptoHelper {
public:
    VirgilCryptoHelper();
    
    ~VirgilCryptoHelper();
    
    /**
     * @brief Prepare crypto helper. Create or load keys.
     *
     * @param[in] identity              email as server identity (can be fictive).
     * @param[in] privateKeyDir         local directory with private key.
     *
     * @return "true" if initialization has been completed successfully.
     */
    bool init(const std::string & identity,
              const std::string & privateKeyDir);
    
    /**
     * @brief Is crypto helper can be used.
     *
     * @return "true" if crypto helper can be used.
     */
    bool isValid() const;
    
    /**
     * @brief Encrypt data for recipient with given identity.
     *
     * @param[in] recipientIdentity     email as recipient identity (can be fictive).
     * @param[in] textForEncrypt        text for encryption.
     * @param[out] encryptedBase64      encrypted data in base64 format.
     *
     * @return "true" if encryption has been done successfully.
     */
    bool encrypt(const std::string & recipientIdentity,
                 const std::string & textForEncrypt,
                 std::string & encryptedBase64);
    
    /**
     * @brief Decrypt data with own private key.
     *
     * @param[in] encryptedBase64       encrypted data in base64 format.
     * @param[out] decryptedText        text for encryption.
     *
     * @return "true" if decryption has been done successfully.
     */
    bool decrypt(const std::string & encryptedBase64,
                 std::string & decryptedText);
    
    /**
     * @brief Sign data with own private key.
     *
     * @param[in] textForSign           text data for signature.
     * @param[out] signature            created signature data in base64.
     *
     * @return "true" if signature has been done successfully.
     */
    bool sign(const std::string & textForSign,
              std::string & signature);
    
    /**
     * @brief Verify receiver signature.
     *
     * @param[in] recipientIdentity     email as recipient identity (can be fictive).
     * @param[in] textForSign           text data which has been used for signature.
     * @param[in] signature             current received signature.
     *
     * @return "true" if signature is correct.
     */
    bool verify(const std::string & recipientIdentity,
                const std::string & textForSign,
                const std::string & signature);
    
    /**
     * @brief Get own identity (email).
     *
     * @return own identity (email).
     */
    std::string ownIdentity() const;
    
private:
    /// Application token created at http://developer.virgilsecurity.com
    static const std::string _applicationToken;
    
    bool m_valid;                       /**< Is current crypto helper ready to use */
    ServicesHub m_servicesHub;          /**< Virgil services hub */
    CardModel m_currentCard;            /**< Virgil card of current application instance. (Stored in clod) */
    VirgilByteArray m_privateKey;       /**< Virgil private key of current application instance. (Stored localy) */
    std::string m_privateKeyFile;       /**< Name of local file with Virgil private key. */
    
    /**
     * @brief Save current private key to file.
     *
     * @return "true" if successfully saved.
     */
    bool savePrivateKey();
    
    /**
     * @brief Load own private key from file.
     *
     * @return "true" if successfully loaded.
     */
    bool loadPrivateKey();
    
    /**
     * @brief Request Virgil Card with given identity from cloud.
     *
     * @param[in] identity              email as identity (can be fictive).
     * @param[out] card                 received Virgil Card.
     *
     * @return "true" if successfully received.
     */
    bool getCard(const std::string & identity, CardModel & card);
    
    /**
     * @brief Create eliptic curve key pair, create card, push it to cloud.
     *
     * @param[in] identity              email as identity (can be fictive).
     * @param[out] card                 received Virgil Card.
     *
     * @return "true" if successfully received.
     */
    bool createCard(const std::string & identity,
                    const std::string & privateKeyDir);
    
};

#endif /* VirgilCryptoHelper_h */
