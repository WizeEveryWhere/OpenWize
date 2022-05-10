################################################################################

set(MODULE_NAME tinycrypt)

################################################################################

# Set tinycrypt path
set( ${MODULE_NAME}_PATH "tinycrypt/lib")
set( ${MODULE_NAME}_INCLUDE "${${MODULE_NAME}_PATH}/include" )

# Add library
add_library(${MODULE_NAME} STATIC)

 # Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        ${${MODULE_NAME}_PATH}/source/aes_decrypt.c
        ${${MODULE_NAME}_PATH}/source/aes_encrypt.c
        ${${MODULE_NAME}_PATH}/source/cbc_mode.c
        ${${MODULE_NAME}_PATH}/source/ccm_mode.c
        ${${MODULE_NAME}_PATH}/source/cmac_mode.c
        ${${MODULE_NAME}_PATH}/source/ctr_mode.c
        ${${MODULE_NAME}_PATH}/source/ctr_prng.c
        ${${MODULE_NAME}_PATH}/source/ecc.c
        ${${MODULE_NAME}_PATH}/source/ecc_dh.c
        ${${MODULE_NAME}_PATH}/source/ecc_dsa.c
        ${${MODULE_NAME}_PATH}/source/ecc_platform_specific.c
        ${${MODULE_NAME}_PATH}/source/hmac.c
        ${${MODULE_NAME}_PATH}/source/hmac_prng.c
        ${${MODULE_NAME}_PATH}/source/sha256.c
        ${${MODULE_NAME}_PATH}/source/utils.c
    )

# Add include dir
target_include_directories(
    ${MODULE_NAME} 
    PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/${${MODULE_NAME}_INCLUDE}
         ${CMAKE_CURRENT_SOURCE_DIR}/${${MODULE_NAME}_INCLUDE}/tinycrypt
    )

# Add alias
add_library(3rd::${MODULE_NAME} ALIAS ${MODULE_NAME})

############################################################################
