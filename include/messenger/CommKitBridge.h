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

#ifndef VM_COMM_KIT_BRIDGE_H
#define VM_COMM_KIT_BRIDGE_H

#include <virgil/crypto/common/vsc_str.h>
#include <virgil/crypto/common/vsc_data.h>
#include <virgil/crypto/common/vsc_buffer.h>

#include <virgil/crypto/foundation/vscf_impl.h>

#include <virgil/sdk/comm-kit/vssq_messenger_cloud_fs.h>

#include <QString>
#include <QByteArray>

#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

template<typename CType>
using vsc_unique_ptr = std::unique_ptr<CType, void (*)(const std::remove_const_t<CType> *)>;

using vsc_buffer_ptr_t = vsc_unique_ptr<vsc_buffer_t>;

using vssq_messenger_cloud_fs_ptr_t = vsc_unique_ptr<const vssq_messenger_cloud_fs_t>;

using vscf_impl_ptr_t = vsc_unique_ptr<vscf_impl_t>;

using vscf_impl_ptr_const_t = vsc_unique_ptr<const vscf_impl_t>;

vsc_str_t vsc_str_from(const std::string &str);

QString vsc_str_to_qstring(vsc_str_t str);

vsc_data_t vsc_data_from(const QByteArray &data);

QByteArray vsc_data_to_qbytearray(vsc_data_t data);

vsc_buffer_ptr_t vsc_buffer_wrap_ptr(vsc_buffer_t *ptr);

vscf_impl_ptr_t vscf_impl_wrap_ptr(vscf_impl_t *ptr);

vscf_impl_ptr_const_t vscf_impl_wrap_ptr(const vscf_impl_t *ptr);

std::tuple<QByteArray, vsc_buffer_ptr_t> makeMappedBuffer(size_t size);

void ensureMappedBuffer(QByteArray &bytes, vsc_buffer_ptr_t &buffer, size_t capacity);

void adjustMappedBuffer(const vsc_buffer_ptr_t &buffer, QByteArray &bytes);

#endif // VM_COMM_KIT_BRIDGE_H
