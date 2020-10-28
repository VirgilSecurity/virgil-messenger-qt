message(STATUS "[Begin] customer.cmake")

# Set project target name and variables before define project in main Cmake file
if(NOT PROJECT_NAME)
    set(VS_TARGET_NAME "Area52Messenger")
    message(STATUS "Set target name: [${VS_TARGET_NAME}]")
    # Android API level
    set(ANDROID_NATIVE_API_LEVEL "21")
    message(STATUS "Android API level: [${ANDROID_NATIVE_API_LEVEL}]")
else()

    # -- Application variables
    set(VS_ORGANIZATION_NAME "Area52")
    set(VS_ORGANIZATION_DISPLAYNAME "Area52")
    
    set(VS_APPLICATION_NAME "Area52Messenger")
    set(VS_APPLICATION_DISPLAY_NAME "Area52 Messenger")
    
    set(VS_ORGANIATION_DOMAIN "virgil.net")
    set(VS_MESSANGER_URL_TEMPLATE "https://messenger%1.area52.virgil.net")
    set(VS_XMPP_URL_TEMPLATE "xmpp%1.area52.virgil.net")
    
    # -- Sources and includes
    # Custom customer sources and includes
    #set(VS_CUSTOMER_INCLUDE_DIR "${VS_CUSTOMER_DIR}/includes")
    #set(VS_CUSTOMER_SOURCES "${VS_CUSTOMER_DIR}/src/Example.cpp")
    
    # ----------    
    # Templates
    # ----------    
    # ***********************************************************************************
    list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/include/VSQCustomer.h.in=${PROJECT_SOURCE_DIR}/include/VSQCustomer.h")

    # ----------
    # Android
    # ----------    
    # ***********************************************************************************    
    if(VS_PLATFORM STREQUAL "android")
        # Firebase dir for gradle
        set(VS_FIREBASE_DIR "${PREBUILT_DIR}/firebase_cpp_sdk")
        set(VS_ANDROID_PACKAGE_NAME "com.area52.area52messenger")        
        set(VS_PUSH_PROXY "push-notifications-proxy")
        set(VS_PUSH_TOPIC "com.virgil.Area52Messenger")
        # Templates
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/src/android/java/org/virgil/notification/NotificationClient.java.in=${PROJECT_SOURCE_DIR}/src/android/java/org/virgil/notification/NotificationClient.java")
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/android/AndroidManifest.xml.in=${PROJECT_SOURCE_DIR}/platforms/android/AndroidManifest.xml")
        list(APPEND VS_TEMPLATES "${VS_CUSTOMER_DIR}/platforms/android/google-services.json.in=${PROJECT_SOURCE_DIR}/platforms/android/google-services.json")
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/android/gradle.properties.in=${PROJECT_SOURCE_DIR}/platforms/android/gradle.properties")
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
    # ----------
    # MacOS
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "macos")
        # Sparkle data
        set(SU_FEED_URL "https://virgilsecurity.bintray.com/messenger/macos/nightly/appcast.xml")
        set(SU_PUBLIC_ED_KEY "44RVxRhV4h4Hlw+VOeXvCj78Z5NUhJ2Qi5N+kpu8KxI=")
        # Version
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

        set(MACOSX_BUNDLE_ICON_FILE "MyIcon.icns")
        set(MACOSX_BUNDLE_BACKGROUND_FILE "Background.png")
        set(MACOSX_DEPLOYMENT_TARGET 10.14)
        set(VS_BUNDLE_PREFIX "com.virgilsecurity")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "${VS_BUNDLE_PREFIX}.${PROJECT_NAME}")
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/macos/release-notes.html.in=${CMAKE_CURRENT_BINARY_DIR}/update/release-notes.html")
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/macos/dmg.json.in=${CMAKE_CURRENT_BINARY_DIR}/dmg.json")
    # ----------
    # IOS
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "ios" OR VS_PLATFORM STREQUAL "iossim")
        set(VS_BUNDLE_PREFIX "com.virgil")
        set(MACOSX_BUNDLE_BUNDLE_NAME "${VS_BUNDLE_PREFIX}.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "${VS_BUNDLE_PREFIX}.${PROJECT_NAME}")
    endif()
endif()

message(STATUS "[End] customer.cmake")
































































