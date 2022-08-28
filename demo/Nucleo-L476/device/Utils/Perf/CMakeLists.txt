################################################################################
 
set(MODULE_NAME perf_utils)

################################################################################
# include the build support cmake
find_package(build_support REQUIRED)

add_library( ${MODULE_NAME} STATIC )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/perf_utils.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR}/samples/STM32_CMSIS_HAL 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
# Add dependencies 
target_link_libraries(${MODULE_NAME} 
    PUBLIC 
        3rd::stm32l4xx_hal
    )
