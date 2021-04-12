message(STATUS "[Begin] customer_common.cmake")

# Set project target name and variables before define project in main Cmake file
if(NOT PROJECT_NAME)
    # Android API level
    set(ANDROID_MIN_SDK_VERSION "26")
    set(ANDROID_TARGET_SDK_VERSION "29")
    set(ANDROID_SDK_PLATFORM "${ANDROID_TARGET_SDK_VERSION}")
    set(ANDROID_NATIVE_API_LEVEL "${ANDROID_MIN_SDK_VERSION}")
    message(STATUS "Android API level: [${ANDROID_NATIVE_API_LEVEL}]")
else()
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
        # Templates
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
        # Version
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "${VS_BUNDLE_PREFIX}.${PROJECT_NAME}")
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/macos/release-notes.html.in=${CMAKE_CURRENT_BINARY_DIR}/update/release-notes.html")
        list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/macos/dmg.json.in=${CMAKE_CURRENT_BINARY_DIR}/dmg.json")
    # ----------
    # IOS
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "ios")
        # Version
        set(CMAKE_OSX_DEPLOYMENT_TARGET "9.0")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
        set(MACOSX_BUNDLE_BUNDLE_NAME "${VS_BUNDLE_PREFIX}.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "${VS_BUNDLE_PREFIX}.${PROJECT_NAME}")
    # ----------
    # Windows
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "windows")
       list(APPEND VS_TEMPLATES "${PROJECT_SOURCE_DIR}/platforms/windows/messenger.nsi.in=${CMAKE_BINARY_DIR}/messenger.nsi")
    endif()
endif()

message(STATUS "[End] customer_common.cmake")
