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


#ifndef VM_MESSAGE_CONTENT_JSON_UTILS_H
#define VM_MESSAGE_CONTENT_JSON_UTILS_H

#include "MessageContent.h"

#include <QByteArray>
#include <QString>
#include <QJsonObject>

namespace vm {

//
//  Utility class to convert MessageContent to/from JSON.
//
class MessageContentJsonUtils {

public:
    //
    //  Convert message content to the JSON object.
    //
    static QJsonObject to(const MessageContent& messageContent, const bool writeLocalPaths);

    //
    //  Convert message content to the JSON string.
    //
    static QString toString(const MessageContent& messageContent, const bool writeLocalPaths);

    //
    //  Convert message content to the JSON byte array.
    //
    static QByteArray toBytes(const MessageContent& messageContent, const bool writeLocalPaths);

    //
    //  Convert any JSON object to JSON byte array.
    //
    static QByteArray toBytes(const QJsonObject& jsonObject);

    //
    //  Get message content from the JSON object.
    //
    static MessageContent from(const QJsonObject& messageJsonObject, QString& errorString);

    //
    //  Get message content from the JSON string.
    //
    static MessageContent fromString(const QString& messageJsonString, QString& errorString);

    //
    //  Get message content from the JSON byte array.
    //
    static MessageContent fromBytes(const QByteArray& messageJsonBytes, QString& errorString);

    //
    //  Parse extra JSON fields related to a picture content.
    //
    static bool readExtras(const QJsonObject& json, MessageContentPicture& picture);

    //
    //  Parse extra JSON fields related to a picture content.
    //
    static bool readExtras(const QString& jsonString, MessageContentPicture& picture);

    //
    //  Parse write JSON fields related to a picture content.
    //
    static bool writeExtras(const MessageContentPicture& picture, const bool writeLocalPaths, QJsonObject& json);

private:
    MessageContentJsonUtils() {};

    //
    //  Write attachment fields except extras.
    //
    static void writeAttachment(const MessageContentAttachment& attachment, QJsonObject& jsonObject);

    //
    //  Parse attachment fields except extras.
    //
    static void readAttachment(const QJsonObject& jsonObject, MessageContentAttachment& attachment);
};
} // namespace vm

#endif // VM_MESSAGE_CONTENT_JSON_UTILS_H
