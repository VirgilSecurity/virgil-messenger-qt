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

#ifndef VM_CLOUD_FILES_UPDATE_H
#define VM_CLOUD_FILES_UPDATE_H

#include "CloudFile.h"
#include "CloudFileUpdateSource.h"

#include <variant>

namespace vm
{
struct CloudFilesUpdateBase
{
    CloudFileHandler parentFolder;
};

struct ListCloudFolderUpdate : public CloudFilesUpdateBase {
    ModifiableCloudFiles files;
};

struct MergeCloudFolderUpdate : public CloudFilesUpdateBase {
    ModifiableCloudFiles added;
    CloudFiles deleted;
    CloudFiles updated;
};

struct CreateCloudFilesUpdate : public CloudFilesUpdateBase {
    ModifiableCloudFiles files;
};

struct DeleteCloudFilesUpdate : public CloudFilesUpdateBase {
    CloudFiles files;
};

struct SetProgressCloudFileUpdate : public CloudFilesUpdateBase {
    CloudFileHandler file;
    quint64 bytesLoaded = 0;
    quint64 bytesTotal = 0;
};

struct DownloadCloudFileUpdate : public CloudFilesUpdateBase {
    CloudFileHandler file;
    QString fingerprint;
};

using CloudFilesUpdate = std::variant<
    ListCloudFolderUpdate,
    MergeCloudFolderUpdate,
    CreateCloudFilesUpdate,
    DeleteCloudFilesUpdate,
    SetProgressCloudFileUpdate,
    DownloadCloudFileUpdate
    >;

}

#endif // VM_CLOUD_FILES_UPDATE_H
