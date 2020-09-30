
message(STATUS "[Begin] customer.cmake")

# -- Application variables

set(VS_ORGANIZATION_NAME "VirgilSecurity")
set(VS_ORGANIZATION_DISPLAYNAME "Virgil")

set(VS_APPLICATION_NAME "VirgilMessenger")
set(VS_APPLICATION_DISPLAY_NAME "Virgil Secure messenger")

set(VS_ORGANIATION_DOMAIN "virgil.net")
set(VS_MESSANGER_URL_TEMPLATE "https://messenger%1.virgilsecurity.com")
set(VS_XMPP_URL_TEMPLATE "xmpp%1.virgilsecurity.com")
set(VS_ANDROID_PACKAGE_NAME "com.virgilsecurity.android.virgil")

# -- Sources and includes
# Custom customer sources and includes
#set(VS_CUSTOMER_INCLUDE_DIR "${VS_CUSTOMER_DIR}/includes")
#set(VS_CUSTOMER_INCLUDES "${VS_CUSTOMER_DIR}/${VS_CUSTOMER_INCLUDE_DIR}/Example.hpp")
#set(VS_CUSTOMER_SOURCES "${VS_CUSTOMER_DIR}/src/Example.cpp")

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

message(STATUS "[End] customer.cmake")
