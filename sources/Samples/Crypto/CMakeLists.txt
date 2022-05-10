################################################################################

set(MODULE_NAME crypto)

################################################################################

add_library(${MODULE_NAME} OBJECT )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include 
    )

# Add header only to Build
target_sources(${MODULE_NAME}
    PRIVATE
        include/crypto.h
        include/key_priv.h
    )

if(USE_CRYPTO_SAMPLE)
    target_compile_definitions(${MODULE_NAME} PRIVATE SECURE)
    # Add sources to Build
    target_sources(${MODULE_NAME}
        PRIVATE
            src/confidentiality.c
            src/integrity.c
            src/key.c
            src/utils_secure.c
        )
    # Add dependencies
    target_link_libraries(
        ${MODULE_NAME}
        PUBLIC 
            3rd::tinycrypt 
        )        
    # Add unit-test(s), if any
    if(BUILD_TEST)
        # Set unittest headers to mock 
        set(MOCK_LIST )
        # Set unittest group runner list
        set(GRP_RUNNER_LIST Samples_Crypto)
        # set the DUT module
        set(DUT_MODULE ${MODULE_NAME})
        add_subdirectory(unittest)
    endif()
endif(USE_CRYPTO_SAMPLE)

# Add alias
add_library(Samples::${MODULE_NAME} ALIAS ${MODULE_NAME})
