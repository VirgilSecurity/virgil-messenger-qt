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

#ifndef VM_JSON_UTILS_H
#define VM_JSON_UTILS_H

#include <QJsonObject>
#include <QJsonDocument>

#include <QByteArray>
#include <QString>

namespace vm {

//
//  Utility functions that simplifies JSON operations.
//
class JsonUtils
{
public:
    static QByteArray toBytes(const QJsonObject &json, const QString &underKey = {})
    {

        if (underKey.isEmpty()) {
            return QJsonDocument(json).toJson(QJsonDocument::Compact);

        } else {
            QJsonObject parent;
            parent.insert(underKey, json);
            return QJsonDocument(parent).toJson(QJsonDocument::Compact);
        }
    }

    template<typename T>
    static QByteArray toBytes(const T &obj, const QString &underKey = {})
    {

        QJsonObject json;
        obj.writeJson(json);

        return toBytes(json, underKey);
    }

    static QString toString(const QJsonObject &json, const QString &underKey = {})
    {
        return QString(toBytes(json, underKey));
    }

    template<typename T>
    static QString toString(const T &obj, const QString &underKey = {})
    {
        return QString(toBytes(obj, underKey));
    }

    template<typename T>
    static std::optional<T> toObject(const QJsonObject &json, const QString &underKey = {})
    {

        T obj;

        if (underKey.isEmpty()) {
            if (obj.readJson(json)) {
                return obj;
            }
        } else {
            auto underJsonValue = json[underKey];
            if (underJsonValue.isObject() && obj.readJson(underJsonValue.toObject())) {
                return obj;
            }
        }

        return std::nullopt;
    }

    template<typename T>
    static std::optional<T> toObject(const QByteArray &jsonBytes, const QString &underKey = {})
    {

        const auto jsonDoc = QJsonDocument::fromJson(jsonBytes);

        if (jsonDoc.isObject()) {
            return toObject<T>(jsonDoc.object(), underKey);
        }

        return std::nullopt;
    }

    template<typename T>
    static std::optional<T> toObject(const QString &jsonBytes, const QString &underKey = {})
    {
        return toObject<T>(jsonBytes.toUtf8(), underKey);
    }

private:
    JsonUtils() = default;
};
} // namespace vm

#endif // VM_JSON_UTILS_H
