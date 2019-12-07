#include <VSInfoGeneralController.h>
#include <fake_vs_info_general_t.h>

static void
copy_string_to_arr(uint8_t str[], uint8_t arr[], int size) {

    for (int i = 0; i < size; ++i) {
        arr[i] = str[i];
    }
}

VSInfoGeneralController::VSInfoGeneralController(QObject *parent)
    : QObject(parent), m_VSInfoGeneralList(), m_CurrentVSInfoGeneralModel(nullptr) {


    // Generating of fake general info
    for (size_t i = 0; i < 10; ++i) {

        vs_info_general_t general_info;

        uint8_t arr[] = {'0', '1', '2', '3', '4'};

        arr[0] = '0' + i;
        copy_string_to_arr(arr, general_info.manufacture_id, 5);
        copy_string_to_arr(arr, general_info.device_type, 5);
        copy_string_to_arr(arr, general_info.default_netif_mac, 5);
        general_info.device_roles = i;

        general_info.fw_ver.major = i;
        general_info.fw_ver.minor = i;
        general_info.fw_ver.patch = i;
        general_info.fw_ver.build = i;

        general_info.tl_ver.major = i;
        general_info.tl_ver.minor = i;
        general_info.tl_ver.patch = i;
        general_info.tl_ver.build = i;

        VSInfoGeneralModel *model1 = new VSInfoGeneralModel(general_info);
        m_VSInfoGeneralList.append(model1);
    }
}

VSInfoGeneralController::~VSInfoGeneralController() {
    for (auto model : m_VSInfoGeneralList) {
        delete model;
    }

    delete m_CurrentVSInfoGeneralModel;
}

QQmlListProperty<VSInfoGeneralModel>
VSInfoGeneralController::getVSInfoGeneralList() {
    return QQmlListProperty<VSInfoGeneralModel>(this, m_VSInfoGeneralList);
}

VSInfoGeneralModel *
VSInfoGeneralController::getCurrentVSInfoGeneral() {
    return m_CurrentVSInfoGeneralModel;
}
