################################################################################

set(MODULE_NAME app)

################################################################################

add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/ses_dispatcher.c
        src/time_mgr.c
        src/wize_api.c
        src/internal/adm_internal.c
        src/internal/dwn_internal.c
        src/internal/inst_internal.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/internal 
        ${CMAKE_BINARY_DIR}    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Add dependencies
target_link_libraries(
    ${MODULE_NAME} 
    PRIVATE 
        WizeCore::mgr 
        WizeCore::net 
        WizeCore::proto
        #WizeCore::* 
        Samples::crypto
        Samples::timeevt
        Samples::imgstorage
        Samples::parameters
        Samples::logger 
        3rd::freertos 
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST)
    # Set unittest headers to mock 
    set(MOCK_LIST )
    # Set unittest group runner list
    set(GRP_RUNNER_LIST )
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    add_subdirectory(unittest)
endif()
