isEmpty(VS_CUSTOMER) {
    # Available customers: Virgil, Area52
    VS_CUSTOMER=Virgil
}

message("Customer:" $$VS_CUSTOMER)

android: {
    ANDROID_DIR = $$PWD/$$VS_CUSTOMER/platforms/android

    OTHER_FILES += \
        $$ANDROID_DIR/AndroidManifest.xml \
        $$ANDROID_DIR/res/drawable/splash.xml \
        $$ANDROID_DIR/src/org/virgil/notification/NotificationClient.java
}

mkpath(../generated)
system($$QMAKE_COPY_DIR ../platforms ../generated/)
system($$QMAKE_COPY_DIR common/* ../generated/)
system($$QMAKE_COPY_DIR $$VS_CUSTOMER/* ../generated/)
