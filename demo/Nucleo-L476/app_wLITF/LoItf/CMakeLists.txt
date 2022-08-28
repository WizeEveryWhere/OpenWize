################################################################################

set(MODULE_NAME loitf)

################################################################################

add_library(${MODULE_NAME} INTERFACE)

# Add sources to Build
target_sources(${MODULE_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/loitf.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Add dependencies
target_link_libraries(${MODULE_NAME} 
    INTERFACE 
        bsp 
        Openwize::Openwize 
        perf_utils
    )
