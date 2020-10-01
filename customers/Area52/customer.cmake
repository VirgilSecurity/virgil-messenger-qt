
message(STATUS "[Begin] customer.cmake")

# -- Application variables

set(VS_ORGANIZATION_NAME "Area52")
set(VS_ORGANIZATION_DISPLAYNAME "Area52")

set(VS_APPLICATION_NAME "Area52Messenger")
set(VS_APPLICATION_DISPLAY_NAME "Area52 Messenger")

set(VS_ORGANIATION_DOMAIN "virgil.net")
set(VS_MESSANGER_URL_TEMPLATE "https://messenger%1.area52.virgil.net")
set(VS_XMPP_URL_TEMPLATE "xmpp%1.area52.virgil.net")
set(VS_ANDROID_PACKAGE_NAME "com.area52.area52messenger")


# -- Sources and includes
# Custom customer sources and includes
#set(VS_CUSTOMER_INCLUDE_DIR "${VS_CUSTOMER_DIR}/includes")
#set(VS_CUSTOMER_INCLUDES "${VS_CUSTOMER_DIR}/${VS_CUSTOMER_INCLUDE_DIR}/Example.hpp")
#set(VS_CUSTOMER_SOURCES "${VS_CUSTOMER_DIR}/src/Example.cpp")
set(VS_CUSTOMER_SOURCES "${VS_CUSTOMER_DIR}/src/customer.qrc")

# Firebase dir for gradle
set(VS_FIREBASE_DIR "${PREBUILT_DIR}/firebase_cpp_sdk")

# Templates
list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/include/VSQCustomer.h.in=${PROJECT_SOURCE_DIR}/include/VSQCustomer.h")
if(VS_PLATFORM STREQUAL "android")
    list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/src/android/java/org/virgil/notification/NotificationClient.java.in=${PROJECT_SOURCE_DIR}/src/android/java/org/virgil/notification/NotificationClient.java")
    list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/android/AndroidManifest.xml.in=${PROJECT_SOURCE_DIR}/platforms/android/AndroidManifest.xml")
    list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/android/google-services.json.in=${PROJECT_SOURCE_DIR}/platforms/android/google-services.json")
    list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/android/gradle.properties.in=${PROJECT_SOURCE_DIR}/platforms/android/gradle.properties")
elseif(VS_PLATFORM STREQUAL "macos")    
    list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/macos/release-notes.html.in=${PROJECT_SOURCE_DIR}/platforms/macos/release-notes.html")
endif()    

# Files
list(APPEND VS_FILES "")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xhdpi/icon_round.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xhdpi/icon_round.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xhdpi/icon.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xhdpi/icon.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xhdpi/splashscreen.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xhdpi/splashscreen.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-hdpi/icon_round.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-hdpi/icon_round.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-hdpi/icon.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-hdpi/icon.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-hdpi/splashscreen.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-hdpi/splashscreen.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xxhdpi/icon_round.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xxhdpi/icon_round.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xxhdpi/icon.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xxhdpi/icon.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xxhdpi/splashscreen.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xxhdpi/splashscreen.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xxxhdpi/icon_round.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xxxhdpi/icon_round.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xxxhdpi/icon.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xxxhdpi/icon.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-xxxhdpi/splashscreen.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-xxxhdpi/splashscreen.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-mdpi/icon_round.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-mdpi/icon_round.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-mdpi/icon.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-mdpi/icon.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-mdpi/splashscreen.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-mdpi/splashscreen.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-ldpi/icon_round.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-ldpi/icon_round.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-ldpi/icon.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-ldpi/icon.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable-ldpi/splashscreen.png=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable-ldpi/splashscreen.png")
list(APPEND VS_FILES "${VS_CUSTOMER_DIR}/platforms/android/res/drawable/splash.xml=${PROJECT_SOURCE_DIR}/platforms/android/res/drawable/splash.xml")


message(STATUS "[End] customer.cmake")
