
#ifndef VIRGIL_IOT_INFO_CLIENT_APPLICATION_HEADERS_H_
#define VIRGIL_IOT_INFO_CLIENT_APPLICATION_HEADERS_H_

#include <QtCore>
#include <QtNetwork>
#include <QtWidgets>

#include <array>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <kit/snap_protocol.h>
#include <kit/logger.h>

using namespace std::string_literals;

struct CAppSettings {
    VirgilIoTKit::CMac _ownMac;
};

extern CAppSettings _appSettings;

#endif //VIRGIL_IOT_INFO_CLIENT_APPLICATION_HEADERS_H_
