#include <VSInfoGeneralModel.h>

VSInfoGeneralModel::VSInfoGeneralModel() {
}

VSInfoGeneralModel::VSInfoGeneralModel(vs_info_general_t general_info) {
    m_vs_info_general = general_info;
}

QString
VSInfoGeneralModel::manufacture_id() const {

    std::string str = (char *)m_vs_info_general.manufacture_id;
    return QString::fromUtf8((char *)m_vs_info_general.manufacture_id, VS_DEVICE_MANUFACTURE_ID_SIZE);
}

QString
VSInfoGeneralModel::deviceType() const {
    return QString::fromUtf8((char *)m_vs_info_general.deviceType, VS_DEVICE_TYPE_SIZE);
}

QString
VSInfoGeneralModel::default_netif_mac() const {
    return QString::fromUtf8((char *)m_vs_info_general.default_netif_mac, ETH_ADDR_LEN);
}

int
VSInfoGeneralModel::device_roles() const {
    return m_vs_info_general.device_roles;
}

static QString
to_version(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    QString str = "";
    return str.sprintf("%d.%d.%d.%d", a, b, c, d);
}

QString
VSInfoGeneralModel::fw_ver() const {

    return to_version(m_vs_info_general.fw_ver.build,
                      m_vs_info_general.fw_ver.major,
                      m_vs_info_general.fw_ver.minor,
                      m_vs_info_general.fw_ver.patch);
}
QString
VSInfoGeneralModel::tl_ver() const {
    return to_version(m_vs_info_general.tl_ver.build,
                      m_vs_info_general.tl_ver.major,
                      m_vs_info_general.tl_ver.minor,
                      m_vs_info_general.tl_ver.patch);
}
