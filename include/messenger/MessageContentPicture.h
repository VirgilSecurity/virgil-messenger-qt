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



#ifndef VM_MESSAGE_CONTENT_PICTURE_H
#define VM_MESSAGE_CONTENT_PICTURE_H

#include "MessageContentFile.h"


namespace vm {

//
//  Class that handles picture as message content.
//
class MessageContentPicture : public MessageContentAttachment  {
public:
    //
    //  Apply picture specific update.
    //
    bool applyUpdate(const MessageUpdate& update) override;

    //
    //  Return path to the resized picture suitable for a preview.
    //
    QString previewPath() const;

    //
    //  Return preview picture path if exists or thumbnail path otherwise.
    //
    QString previewOrThumbnailPath() const;

    //
    //  Return image thumbnail size.
    //
    QSize thumbnailSize() const;

    //
    //  Return picture thumbnail.
    //
    MessageContentFile thumbnail() const;

    //
    //  Create picture message content from the given path.
    //
    static MessageContentPicture createFromLocalFile(const QUrl& localUrl);

private:
    QString m_previewFilePath;
    MessageContentFile m_thumbnail;
    qsizetype m_thumbnailHeight;
    qsizetype m_thumbnailWidth;
};
} // namespace vm

#endif // VM_MESSAGE_CONTENT_PICTURE_H
