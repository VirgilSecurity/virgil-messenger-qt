message(STATUS "[Begin] customer.cmake")

# Set project target name and variables before define project in main Cmake file
if(NOT PROJECT_NAME)
    set(VS_TARGET_NAME "Area52Messenger")
    message(STATUS "Set target name: [${VS_TARGET_NAME}]")
else()

    # -- Application variables
    set(VS_ORGANIZATION_NAME "Area52")
    set(VS_ORGANIZATION_DISPLAYNAME "Area52")

    set(VS_APPLICATION_NAME "Area52Messenger")
    set(VS_APPLICATION_DISPLAY_NAME "Area52 Messenger")

    set(VS_ORGANIATION_DOMAIN "virgil.net")
    set(VS_MESSANGER_URL_TEMPLATE "https://messenger%1.area52.virgil.net")
    set(VS_XMPP_URL_TEMPLATE "xmpp%1.area52.virgil.net")
    set(CONTACT_DISCOVERY_URL_TEMPLATE "https://disco%1.virgilsecurity.com")
    set(VS_XMPP_DOMAIN_TEMPLATE "xmpp%1.area52.virgil.net")
    set(VS_PUSH_PROXY "push-notifications-proxy")
    set(VS_PUSH_TOPIC "com.virgil.Area52Messenger")

    set(SECURITY_APPLICATION_GROUP_IDENTIFIER "group.com.virgil.Area52Messenger")

    # -- Sources and includes
    # Custom customer sources and includes
    #set(VS_CUSTOMER_INCLUDE_DIR "${VS_CUSTOMER_DIR}/includes")
    #set(VS_CUSTOMER_SOURCES "${VS_CUSTOMER_DIR}/src/Example.cpp")

    # ----------
    # Android
    # ----------    
    # ***********************************************************************************
    if(VS_PLATFORM STREQUAL "android")
        # Firebase dir for gradle
        set(VS_ANDROID_PACKAGE_NAME "com.area52.area52messenger")

    # ----------
    # Windows
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "windows")
        # Windows sparkle feed URL
        set(WIN_SU_URL "https://virgilsecurity.bintray.com/messenger/windows/area52/nightly")
        set(WIN_SU_FILE "${PROJECT_NAME}-installer-${VS_TARGET_VERSION}.exe")
        set(WIN_SU_FILE_URL "${WIN_SU_URL}/${WIN_SU_FILE}")
        set(WIN_SU_FEED_URL "${WIN_SU_URL}/appcast.xml")

    # ----------
    # MacOS
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "macos")
        # Sparkle data
        set(SU_FEED_URL "https://virgilsecurity.bintray.com/messenger/macos/nightly/appcast.xml")
        set(SU_PUBLIC_ED_KEY "44RVxRhV4h4Hlw+VOeXvCj78Z5NUhJ2Qi5N+kpu8KxI=")
        set(MACOSX_BUNDLE_ICON_FILE "MyIcon.icns")
        set(MACOSX_BUNDLE_BACKGROUND_FILE "Background.png")
        set(VS_BUNDLE_PREFIX "com.virgilsecurity")
    # ----------
    # IOS
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "ios")
        set(VS_BUNDLE_PREFIX "com.virgil")
    endif()
endif()

message(STATUS "[End] customer.cmake")
































































