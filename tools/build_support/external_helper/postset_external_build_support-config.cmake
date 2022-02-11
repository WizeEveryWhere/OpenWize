if(IS_EXTERNAL_PRJ)
    # set install 
    set_property(
        TARGET ${TGT_NAME} 
        APPEND PROPERTY 
        PUBLIC_HEADER "${${TGT_NAME}_PUBLIC_HEADERS}"
        )
    set_property(
        TARGET ${TGT_NAME} 
        APPEND PROPERTY 
        PRIVATE_HEADER "${${TGT_NAME}_PRIVATE_HEADERS}"
        )

    setup_install(
        TARGET ${TGT_NAME}
        )
endif(IS_EXTERNAL_PRJ)