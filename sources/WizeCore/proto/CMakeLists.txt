################################################################################

set(MODULE_NAME proto)

################################################################################
 
add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/proto.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Add dependencies
target_link_libraries(
    ${MODULE_NAME} 
    PRIVATE 
        Samples::crc_sw 
        Samples::crypto
        Samples::reedsolomon
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST)

	get_target_property(SAMPLES_CRC_SW_SRC_PATH Samples::crc_sw SOURCE_DIR)
	get_target_property(SAMPLES_CRYPTO_SRC_PATH Samples::crypto SOURCE_DIR)
	get_target_property(SAMPLES_REEDSOLOMON_SRC_PATH Samples::reedsolomon SOURCE_DIR)

    # Set unittest headers to mock 
    set(MOCK_LIST 
        ${SAMPLES_CRC_SW_SRC_PATH}/include/crc_sw.h
        ${SAMPLES_CRYPTO_SRC_PATH}/include/crypto.h
        ${SAMPLES_REEDSOLOMON_SRC_PATH}/include/rs.h
    )
    # Set unittest group runner list
    set(GRP_RUNNER_LIST WizeCoreProto)
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    add_subdirectory(unittest)
endif()
