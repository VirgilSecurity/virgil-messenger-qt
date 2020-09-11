isEmpty(VS_CUSTOMER) {
    # Available customers: Virgil, Area52
    VS_CUSTOMER=Virgil
}

message("Customer:" $$VS_CUSTOMER)

android: {
    OTHER_FILES += \
        $$PWD/$$VS_CUSTOMER/platforms/android/AndroidManifest.xml \
        $$PWD/$$VS_CUSTOMER/platforms/android/res/drawable/splash.xml
}

mkpath(../generated)
system($$QMAKE_COPY_DIR ../platforms ../generated/)
system($$QMAKE_COPY_DIR common/* ../generated/)
system($$QMAKE_COPY_DIR $$VS_CUSTOMER/* ../generated/)
