//  Copyright (C) 2015-2021 Virgil Security, Inc.
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

#ifndef VM_CORE_MESSENGER_STATUS_H
#define VM_CORE_MESSENGER_STATUS_H

namespace vm {

enum class CoreMessengerStatus {
    Success,
    Error_CryptoInit,
    Error_Offline,
    Error_NoCred,
    Error_Signin,
    Error_Signup,
    Error_MakeKeyBackup,
    Error_RestoreKeyBackup,
    Error_UserNotFound,
    Error_UserAlreadyExists,
    Error_ExportCredentials,
    Error_ImportCredentials,
    Error_ExportUser,
    Error_ImportUser,
    Error_InvalidCarbonMessage,
    Error_InvalidMessageFormat,
    Error_InvalidMessageVersion,
    Error_InvalidMessageCiphertext,
    Error_InvalidMessageRecipient,
    Error_FileEncryptionReadFailed,
    Error_FileEncryptionWriteFailed,
    Error_FileEncryptionCryptoFailed,
    Error_FileDecryptionReadFailed,
    Error_FileDecryptionWriteFailed,
    Error_FileDecryptionCryptoFailed,
    Error_FileDecryptionCryptoSignatureFailed,
    Error_CryptoImportPublicKeyFailed,
    Error_CryptoImportPrivateKeyFailed,
    Error_CryptoExportPublicKeyFailed,
    Error_CryptoExportPrivateKeyFailed,
    Error_CryptoGenerateKeyFailed,
    Error_CloudFsRequestFailed,
    Error_SendMessageFailed,
    Error_GroupIsAlreadyExists,
    Error_GroupNoParticipants,
    Error_UnexpectedCommKitError,
    Error_ModifyGroup_PermissionViolation,
    Error_AccessGroup_PermissionViolation,
    Error_GroupNotFound,
    Error_GroupNotLoaded,
    Error_GroupOwnerNotFound,
    Error_CreateGroup_CryptoFailed,
    Error_CreateGroup_XmppFailed,
    Error_CreateGroup_XmppConfigFailed,
    Error_ImportGroupEpoch_ParseFailed,
    Error_ProcessGroupMessage_SessionIDDoesntMatch,
    Error_ProcessGroupMessage_EpochNotFound,
    Error_ProcessGroupMessage_WrongKeyType,
    Error_ProcessGroupMessage_InvalidSignature,
    Error_ProcessGroupMessage_Ed25519Failed,
    Error_ProcessGroupMessage_DuplicateEpoch,
    Error_ProcessGroupMessage_PlainTextTooLong,
    Error_ProcessGroupMessage_CryptoFailed,
};

} // namespace vm

#endif // VM_CORE_MESSENGER_STATUS_H
