################################################################################

set(MODULE_NAME reedsolomon)

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
        include/rs.h
    )

if(USE_REEDSOLOMON_SAMPLE)
    # Add sources to Build
    target_sources(${MODULE_NAME}
        PRIVATE
            src/rs.c
        )
    # Add dependencies
    target_link_libraries(
        ${MODULE_NAME} 
        PRIVATE 
            Samples::crypto
        )
    # Add unit-test(s), if any
    if(BUILD_TEST)
        # Set unittest headers to mock 
        set(MOCK_LIST )
        # Set unittest group runner list
        set(GRP_RUNNER_LIST Samples_ReedSolomon)
        # set the DUT module
        set(DUT_MODULE ${MODULE_NAME})
        add_subdirectory(unittest)
    endif()
endif(USE_REEDSOLOMON_SAMPLE)

# Add alias
add_library(Samples::${MODULE_NAME} ALIAS ${MODULE_NAME})
