################################################################################

set(MODULE_NAME bsp)

################################################################################

add_library(${MODULE_NAME} STATIC)

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/bsp_boot.c
        src/bsp_flash.c
        src/bsp_gpio_it.c
        src/bsp_gpio.c
        src/bsp_lp.c
        src/bsp_rtc.c
        src/bsp_uart.c
        src/bsp.c
        src/fault_handler.c
        src/platform.c
        src/sys_handler.c
        src/syscalls.c
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
        3rd::stm32l4xx_hal 
    )
