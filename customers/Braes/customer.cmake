message(STATUS "[Begin] customer.cmake")

# Set project target name and variables before define project in main Cmake file
if(NOT PROJECT_NAME)
    set(VS_TARGET_NAME "BraesMessenger")
    message(STATUS "Set target name: [${VS_TARGET_NAME}]")
else()

    # -- Application variables
    set(VS_ORGANIZATION_NAME "Braes Capital")
    set(VS_ORGANIZATION_DISPLAYNAME "Braes")
    
    set(VS_APPLICATION_NAME "${VS_TARGET_NAME}")
    set(VS_APPLICATION_DISPLAY_NAME "Braes Secure Messenger")
    
    set(VS_ORGANIATION_DOMAIN "virgil.net")
    set(VS_MESSANGER_URL_TEMPLATE "https://messenger%1.virgilsecurity.com")
    set(VS_XMPP_URL_TEMPLATE "xmpp%1.virgilsecurity.com")
    set(VS_XMPP_DOMAIN_TEMPLATE "xmpp%1.virgilsecurity.com")
    set(CONTACT_DISCOVERY_URL_TEMPLATE "https://disco%1-va.virgilsecurity.com")
    
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
        set(VS_ANDROID_PACKAGE_NAME "com.virgil.BraesMessenger")        
        set(VS_PUSH_PROXY "braes-push-notifications-proxy")
        set(VS_PUSH_TOPIC "com.virgil.BraesMessenger")
    # ----------
    # MacOS
    # ----------
    # ***********************************************************************************
    elseif(VS_PLATFORM STREQUAL "macos")
        # Sparkle data
        set(SU_FEED_URL "https://virgilsecurity.bintray.com/braes/macos/nightly/appcast.xml")
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
































































