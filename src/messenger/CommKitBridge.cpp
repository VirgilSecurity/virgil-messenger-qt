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

#include "CommKitBridge.h"


vsc_str_t vsc_str_from(const std::string& str) {
    return vsc_str(str.c_str(), str.size());
}


QString vsc_str_to_qstring(vsc_str_t str) {
    return QString::fromStdString({str.chars, str.len});
}


vsc_data_t vsc_data_from(const QByteArray& data) {
    return vsc_data((const byte *)data.data(), data.size());
}


QByteArray vsc_data_to_qbytearray(vsc_data_t data) {
    return QByteArray((const char *)data.bytes, data.len);
}


vsc_buffer_ptr_t vsc_buffer_wrap_ptr(vsc_buffer_t *ptr) {
    return vsc_buffer_ptr_t{ptr, vsc_buffer_delete};
}


vscf_impl_ptr_t vscf_impl_wrap_ptr(vscf_impl_t *ptr) {
    return vscf_impl_ptr_t{ptr, vscf_impl_delete};
}


vscf_impl_ptr_const_t vscf_impl_wrap_ptr(const vscf_impl_t *ptr) {
    return vscf_impl_ptr_const_t{ptr, vscf_impl_delete};
}


std::tuple<QByteArray, vsc_buffer_ptr_t> makeMappedBuffer(size_t size) {

    QByteArray byteArray(size, 0x00);
    auto buffer  = vsc_buffer_wrap_ptr(vsc_buffer_new());
    vsc_buffer_use(buffer.get(), (byte *)byteArray.data(), byteArray.size());

    return std::make_tuple(std::move(byteArray), std::move(buffer));
}


void ensureMappedBuffer(QByteArray& bytes, vsc_buffer_ptr_t& buffer, size_t capacity) {

    auto len = vsc_buffer_len(buffer.get());

    bytes.resize(len + capacity);

    vsc_buffer_release(buffer.get());
    vsc_buffer_use(buffer.get(), (byte *)bytes.data(), bytes.size());
    vsc_buffer_inc_used(buffer.get(), len);
}


void adjustMappedBuffer(const vsc_buffer_ptr_t& buffer, QByteArray& bytes) {

    bytes.resize(vsc_buffer_len(buffer.get()));
}
