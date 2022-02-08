################################################################################

set(MODULE_NAME crc_sw)

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
        include/crc_sw.h
    )

if(USE_CRC_SAMPLE)
    # Add sources to Build
    target_sources(${MODULE_NAME}
        PRIVATE
            src/crc_sw.c
        )
    # Add unit-test(s), if any
    if(BUILD_TEST)
        # Set unittest headers to mock 
        set(MOCK_LIST )
        # Set unittest group runner list
        set(GRP_RUNNER_LIST Samples_CRC_sw)
        # set the DUT module
        set(DUT_MODULE ${MODULE_NAME})
        add_subdirectory(unittest)
    endif()
endif(USE_CRC_SAMPLE)


# Add alias
add_library(Samples::${MODULE_NAME} ALIAS ${MODULE_NAME})
