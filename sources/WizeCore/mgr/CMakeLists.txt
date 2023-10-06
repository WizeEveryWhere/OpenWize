################################################################################

set(MODULE_NAME mgr)

################################################################################
 
add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
    	src/time_mgr.c
        src/adm_mgr.c
        src/inst_mgr.c
        src/dwn_mgr.c
        src/net_mgr.c
        src/ses_common.c
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
        WizeCore::proto 
        WizeCore::net 
        Samples::timeevt
        Samples::logger 
        3rd::freertos 
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST)
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    add_subdirectory(unittest)
    # Special case for NetMgr
    add_subdirectory(unittest/special_net_case)
endif()
