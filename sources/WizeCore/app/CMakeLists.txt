################################################################################

set(MODULE_NAME app)

################################################################################

add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/wize_api.c
        src/private/ses_dispatcher.c
        src/private/wize_app.c
        src/internal/adm_internal.c
        src/internal/dwn_internal.c
        src/internal/inst_internal.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/internal 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/private
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
        Samples::crypto
        Samples::timeevt
        Samples::parameters
        Samples::logger 
        3rd::freertos 
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST)
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    ################################################################################   
	add_subdirectory(unittest/internal)
	
    ################################################################################   
	add_subdirectory(unittest/private)
	
	################################################################################
    add_subdirectory(unittest)

	################################################################################
endif()
