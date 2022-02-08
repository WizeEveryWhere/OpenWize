################################################################################

set(MODULE_NAME localitf)

################################################################################

add_library(${MODULE_NAME} STATIC)

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/local_itf.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Add dependencies
target_link_libraries(${MODULE_NAME} 
    PUBLIC 
        bsp 
        Openwize::Openwize        
    )
