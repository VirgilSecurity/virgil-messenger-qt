#ifndef __VSInfoGeneralModel__
#define __VSInfoGeneralModel__

#include "fake_vs_info_general_t.h"
#include <QObject>


class VSInfoGeneralModel: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString manufacture_id READ  manufacture_id  NOTIFY  manufacture_id_changed)
    Q_PROPERTY(QString device_type READ device_type NOTIFY device_type_changed)
    Q_PROPERTY(QString default_netif_mac READ  default_netif_mac  NOTIFY  default_netif_mac_changed)
    Q_PROPERTY(int device_roles READ device_roles NOTIFY device_roles_changed)
    Q_PROPERTY(QString fw_ver READ fw_ver NOTIFY fw_ver_changed)
    Q_PROPERTY(QString tl_ver READ tl_ver NOTIFY tl_ver_changed)

public:
    VSInfoGeneralModel();
    VSInfoGeneralModel(vs_info_general_t general_info);

    QString manufacture_id() const;
    QString device_type() const;
    QString default_netif_mac() const;
    int device_roles() const;

    QString fw_ver() const;
    QString tl_ver() const;

signals:
    void manufacture_id_changed();
    void device_type_changed();
    void default_netif_mac_changed();
    void device_roles_changed();
    void fw_ver_changed();
    void tl_ver_changed();

private:
    vs_info_general_t m_vs_info_general;
};

#endif // __VSInfoGeneralModel__
