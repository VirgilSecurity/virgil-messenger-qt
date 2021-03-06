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

#ifndef VM_UTILS_H
#define VM_UTILS_H

#include <QImage>
#include <QImageReader>

#include "Contact.h"
#include "Message.h"
#include "MessageContentAttachment.h"

namespace vm {
namespace Utils {
// String processing/format

QString elidedText(const QString &text, const int maxLength);

QString messageContentDisplayText(const MessageContent &messageContent);

QString printableLoadProgress(quint64 loaded, quint64 total);

QString printableContactsList(const Contacts &contacts);

// Debug

void printThreadId(const QString &message);

// Image functions

QSize applyOrientation(const QSize &size, const int orientation);

QImage applyOrientation(const QImage &image, const int orientation);

QSize calculateThumbnailSize(const QSize &size, const QSize &maxSize, const int orientation = 0);

bool readImage(QImageReader *reader, QImage *image);

// Contacts

Contacts getDeviceContacts(const Contacts &cachedContacts = Contacts());

QUrl getContactAvatarUrl(const ContactHandler contact);

QString displayUsername(const QString &username, const UserId &userId);
} // namespace Utils
} // namespace vm

#endif // VM_UTILS_H
