################################################################################

set(MODULE_NAME net)

################################################################################
 
add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/net_api.c
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
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST)
    # Set unittest headers to mock 
    set(MOCK_LIST 
        ${CMAKE_SOURCE_DIR}/sources/WizeCore/proto/include/proto_api.h
        )
    # Set unittest group runner list
    set(GRP_RUNNER_LIST WizeCore_net)
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    add_subdirectory(unittest)
endif()
