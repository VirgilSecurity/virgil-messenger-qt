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

#include <QSharedPointer>

#include <virgil/iot/qt/protocols/snap/VSQNetifBase.h>

using namespace VirgilIoTKit;

extern "C" {
    struct VSQLowLevelNetif {
        static vs_status_e init(vs_netif_t* netif,
                                      const vs_netif_rx_cb_t rxCb,
                                      const vs_netif_process_cb_t processCb) {
            Q_ASSERT(netif);
            if (!netif) {
                return VS_CODE_ERR_NULLPTR_ARGUMENT;
            }
            auto qNetif = reinterpret_cast<VSQNetifBase*>(netif);

            // Save Callbacks
            qNetif->m_lowLevelRxCall = rxCb;
            qNetif->m_lowLevelPacketProcess = processCb;

            // Initialize Network interface
            return qNetif->init() ? VS_CODE_OK : VS_CODE_ERR_AMBIGUOUS_INIT_CALL;
        }

        static vs_status_e deinit(const vs_netif_t* netif) {
            return VS_CODE_ERR_NOT_IMPLEMENTED;
        }

        static vs_status_e tx(const vs_netif_t* netif, const uint8_t* data, const uint16_t dataSz) {
            return VS_CODE_ERR_NOT_IMPLEMENTED;
        }

        static vs_status_e mac(const vs_netif_t* netif, vs_mac_addr_t* macAddr) {
            return VS_CODE_ERR_NOT_IMPLEMENTED;
        }
    };
}

VSQNetifBase::VSQNetifBase() {
    // User data points to this object
    m_lowLevelNetif.user_data = this;

    // Prepare functionality implementations
    m_lowLevelNetif.init = VSQLowLevelNetif::init;
    m_lowLevelNetif.deinit = VSQLowLevelNetif::deinit;
    m_lowLevelNetif.tx = VSQLowLevelNetif::tx;
    m_lowLevelNetif.mac_addr = VSQLowLevelNetif::mac;

    // Prepare buffer to receive data
    m_lowLevelNetif.packet_buf_filled = 0;
}

bool VSQNetifBase::processData(const QByteArray &data) {
    return false;
}
