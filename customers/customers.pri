isEmpty(VS_CUSTOMER) {
    # Available customers: Virgil, Area52
    VS_CUSTOMER=Virgil
}

message("Customer:" $$VS_CUSTOMER)

mkpath(../generated)
system($$QMAKE_COPY_DIR ../platforms ../generated/)
system($$QMAKE_COPY_DIR $$VS_CUSTOMER/* ../generated/)
system($$QMAKE_COPY_FILE customer.qrc ../generated/src/)

VS_PLATFORMS_PATH=$$absolute_path(../generated/platforms)
