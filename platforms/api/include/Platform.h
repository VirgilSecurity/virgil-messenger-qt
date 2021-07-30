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

#ifndef VM_PLATFORM_PLATFORM
#define VM_PLATFORM_PLATFORM

#include <QString>
#include <QDir>

namespace vm {
namespace platform {

//
//  Provides General Platform Initialization.
//
class Platform
{
public:
    //
    //  Initialize system resources.
    //  Note, Qt Resources should be available at this point.
    //
    virtual bool init() const = 0;

    //
    //  Initialize resources after UI initialization.
    //
    virtual bool uiInit() const = 0;

    //
    //  Return writable location for the application.
    //
    virtual QDir appDataLocation() const = 0;

    //
    //  Return path to a file with custom CA certificates, or empty string.
    //
    virtual QString caBundlePath() const = 0;

    virtual ~Platform() noexcept = default;

    //
    //  This method should be implemented within derived class.
    //
    static Platform &instance();
};

} // namespace platform
} // namespace vm

#endif // VM_PLATFORM_PLATFORM
