################################################################################

set(MODULE_NAME lzma2201)

################################################################################

# Set lzma path
set( ${MODULE_NAME}_PATH "lzma2201/C")
set( ${MODULE_NAME}_INCLUDE "${${MODULE_NAME}_PATH}" )

add_compile_definitions(_LZMA_UINT32_IS_ULONG)

# Add library
add_library(${MODULE_NAME} STATIC)

 # Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        ${${MODULE_NAME}_PATH}/LzmaDec.c
    )

# Add include dir
target_include_directories(
    ${MODULE_NAME} 
    PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}/${${MODULE_NAME}_INCLUDE}
    )

# Add alias
add_library(3rd::${MODULE_NAME} ALIAS ${MODULE_NAME})

############################################################################
