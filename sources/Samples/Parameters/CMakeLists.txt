################################################################################

set(MODULE_NAME parameters)

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
        include/parameters.h
        include/parameters_def.h
        include/parameters_lan_ids.h
    )

if(USE_PARAMETERS_SAMPLE)
    # Add sources to Build
    target_sources(${MODULE_NAME}
        PRIVATE
            src/parameters.c
        )
    # Add unit-test(s), if any
    if(BUILD_TEST)
        # Set unittest headers to mock 
        set(MOCK_LIST )
        # Set unittest group runner list
        set(GRP_RUNNER_LIST Samples_Parameters)
        # set the DUT module
        set(DUT_MODULE ${MODULE_NAME})
        add_subdirectory(unittest)
    endif()
endif(USE_PARAMETERS_SAMPLE)

# Add alias
add_library(Samples::${MODULE_NAME} ALIAS ${MODULE_NAME})
