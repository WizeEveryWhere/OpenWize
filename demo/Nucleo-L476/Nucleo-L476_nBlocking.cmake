cmake_minimum_required( VERSION 3.12 ) 
################################################################################

set(BOARD_NAME "ST L476 Board")
set(APP_NAME "Demo Wize App with non blocking stack")

# add_compile_definitions("HAS_NO_BANNER=1")
add_compile_definitions(USE_RTOS_TRACE=1)
add_compile_definitions(LOGGER_HAS_COLOR=1)
add_compile_definitions(LOGGER_USE_FWRITE=1)
add_compile_definitions(DUMP_CORE_HAS_TRACE=1)
add_compile_definitions(DUMP_CORE_HAS_FAULT_STATUS_REGISTER=1)
add_compile_definitions(L6VERS=L6VER_WIZE_REV_1_2)
add_compile_definitions(WIZEAPI_NOT_BLOCKING=1)
add_compile_definitions(HAS_LPOWER=1)

# add_compile_definitions(USE_SPI=1)
# add_compile_definitions(USE_I2C=1)

add_compile_definitions(COM_SWAP_PINS=1)
add_compile_definitions(USE_CYCCNT=1)

# add_compile_definitions("_POSIX_TIMERS")

add_compile_options(-Wall -ffunction-sections -fdata-sections -fstack-usage)


################################################################################
# Set Coss compile
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING TRUE )

# Use FreeRTOS
set(USE_FREERTOS ON)

# Set MCU
set(VENDOR_MCU "STM32L476xx")

set(HAL_CONF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/demo/Nucleo-L476/STM32HALConfig.cmake")
set(RTOS_CONF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/demo/Nucleo-L476/FreeRTOSConfig.cmake")

################################################################################
find_package(cross_stm32 REQUIRED)
set_cross_stm32()

find_package(toolchain REQUIRED)

################################################################################
if(USE_FREERTOS)
    # Set port arch
    if(${MCU_ARCH} STREQUAL "cortex-m4")
        set(RTOS_PORT_NAME "ARM_CM4F")
    else()
        message(FATAL_ERROR "MCU_ARCH is not defined")
    endif()

    # Set port arch path
    set(RTOS_PORT_PATH "GCC/${RTOS_PORT_NAME}" )
endif(USE_FREERTOS)

################################################################################
# Generate parameters 
# (only if "-DGENERATE_PARAM=ON" is added on cmake command line)
find_package(gen_param REQUIRED)
gen_param(
    SOURCE demo/Nucleo-L476/app_nBlocking/cfg
    DESTINATION demo/Nucleo-L476/app_nBlocking
    )

################################################################################
## subdirectories
set(MOD_DIR_LST 
    "demo/Nucleo-L476/bsp" 
    "demo/Nucleo-L476/board" 
    "demo/Nucleo-L476/device/PhyFake" 
    "demo/Nucleo-L476/app_nBlocking" 
    )

# Add subdirectories
set(MAIN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
foreach(mod_dir ${MOD_DIR_LST})
	message("     *** Add ${mod_dir} : ")
	set(tmp_src_dir "${MAIN_SRC_DIR}/${mod_dir}")
	if(EXISTS "${tmp_src_dir}")
		# Add Sources
		add_subdirectory("${tmp_src_dir}")
		# Add docs
		list(APPEND DOC_SOURCE_DIRS "${tmp_src_dir}")
	endif()
endforeach(mod_dir)
