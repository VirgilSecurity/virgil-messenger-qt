#ifndef DEMO_IOTKIT_QT_FAKE_VS_INFO_GENERAL_T_H
#define DEMO_IOTKIT_QT_FAKE_VS_INFO_GENERAL_T_H
#include <cstdint>

#define VS_DEVICE_MANUFACTURE_ID_SIZE (16)
#define VS_DEVICE_TYPE_SIZE (4)
#define ETH_ADDR_LEN (6)

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint32_t build;
    uint32_t timestamp;
} vs_file_version_unpacked_t;

typedef struct {
    uint8_t manufacture_id[VS_DEVICE_MANUFACTURE_ID_SIZE]; /**< Manufacture ID*/
    uint8_t device_type[VS_DEVICE_TYPE_SIZE];              /**< Device type */
    uint8_t default_netif_mac[ETH_ADDR_LEN];               /**< Default network interface MAC address*/
    uint32_t device_roles;                                 /**< Mask based on #vs_snap_device_role_e elements */
    vs_file_version_unpacked_t fw_ver;                     /**< Firmware version */
    vs_file_version_unpacked_t tl_ver;                     /**< Trust List version */
} vs_info_general_t;


#endif // DEMO_IOTKIT_QT_FAKE_VS_INFO_GENERAL_T_H
