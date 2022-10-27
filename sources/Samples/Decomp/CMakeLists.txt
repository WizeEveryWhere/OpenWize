################################################################################

set(MODULE_NAME decomp)

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
        include/lzma_decode.h
    )

if(USE_DECOMP_SAMPLE)
    target_compile_definitions(${MODULE_NAME} PRIVATE LZMA_DECOMP)
    # Add sources to Build
    target_sources(${MODULE_NAME}
        PRIVATE
            src/lzma_decode.c
            src/decomp.c
        )
    # Add dependencies
    target_link_libraries(
        ${MODULE_NAME}
        PUBLIC 
            3rd::lzma2201 
        )        
    # Add unit-test(s), if any
    if(BUILD_TEST)
        # Set unittest headers to mock 
        set(MOCK_LIST )
        # Set unittest group runner list
        set(GRP_RUNNER_LIST Samples_Decomp)
        # set the DUT module
        set(DUT_MODULE ${MODULE_NAME})
        add_subdirectory(unittest)
    endif()
endif(USE_DECOMP_SAMPLE)

# Add alias
add_library(Samples::${MODULE_NAME} ALIAS ${MODULE_NAME})
