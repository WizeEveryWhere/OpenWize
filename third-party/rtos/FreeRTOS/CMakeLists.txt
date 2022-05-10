################################################################################

set(MODULE_NAME freertos)

cmake_minimum_required( VERSION 3.12 )

################################################################################
function(generate_conf_file conf_filename)
    message("   -> Generate : ${conf_filename}.h")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config.in/${conf_filename}.in
        ${CMAKE_CURRENT_BINARY_DIR}/${conf_filename}.h @ONLY
        )
endfunction(generate_conf_file)
################################################################################

# Check FreeRTOS config file
if(RTOS_CONF_FILE)
    if(EXISTS ${RTOS_CONF_FILE})
        message("   -> Use RTOS_CONF_FILE : ${RTOS_CONF_FILE}")
        include("${RTOS_CONF_FILE}")
    else(EXISTS ${RTOS_CONF_FILE})
        message(FATAL_ERROR "RTOS_CONF_FILE doesn't exist")
    endif(EXISTS ${RTOS_CONF_FILE})
else(RTOS_CONF_FILE)
    message(FATAL_ERROR "RTOS_CONF_FILE is not defined")
endif(RTOS_CONF_FILE)

include(${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS_option.cmake)

# Set FreeRTOS main path
set(RTOS_MAIN_PATH "FreeRTOS-Kernel")

if(NOT CMAKE_CROSSCOMPILING)
    message("CMAKE_CROSSCOMPILING : ${CMAKE_CROSSCOMPILING}")
    set( configUSE_NEWLIB_REENTRANT 0U)
    set( configENABLE_BACKWARD_COMPATIBILITY 1U)
    set(RTOS_PORT_NAME "Posix")
    set(RTOS_PORT_PATH "ThirdParty/GCC/${RTOS_PORT_NAME}" )
    #message("set RTOS_MAIN_PATH to : ${RTOS_MAIN_PATH}")
    #message("set RTOS_PORT_PATH to : ${RTOS_PORT_PATH}")
endif(NOT CMAKE_CROSSCOMPILING)

# Check FreeRTOS port name and port path
if(RTOS_PORT_NAME)
    message("   -> Use Port Arch : ${RTOS_PORT_NAME}")
else(RTOS_PORT_NAME)
    message(WARNING "FreeRTOS Port Arch Name is not defined : set(RTOS_PORT_NAME ...)")
endif(RTOS_PORT_NAME)

if(RTOS_PORT_PATH)
    set(tmp "${CMAKE_CURRENT_SOURCE_DIR}/${RTOS_MAIN_PATH}/portable/${RTOS_PORT_PATH}/portmacro.h")
    if(EXISTS ${tmp})
        message("   -> Use Port Path : ${RTOS_PORT_PATH}")
    else()
        message(FATAL_ERROR "FreeRTOS Port Path not found : ${tmp}")
    endif()
else(RTOS_PORT_PATH)
    message(FATAL_ERROR "FreeRTOS Port Path is not defined : set(RTOS_PORT_PATH ...)")
endif(RTOS_PORT_PATH)

# Generate .h files
generate_conf_file(FreeRTOSConfig)
generate_conf_file(freertos_tasks_c_additions)
generate_conf_file(rtos_macro)

################################################################################
add_library(${MODULE_NAME} STATIC)

target_compile_definitions(${MODULE_NAME} PUBLIC __OS__ OS_FreeRTOS=1)

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        ${RTOS_MAIN_PATH}/croutine.c
        ${RTOS_MAIN_PATH}/event_groups.c
        ${RTOS_MAIN_PATH}/list.c
        ${RTOS_MAIN_PATH}/queue.c
        ${RTOS_MAIN_PATH}/stream_buffer.c
        ${RTOS_MAIN_PATH}/tasks.c
        ${RTOS_MAIN_PATH}/timers.c
        ${RTOS_MAIN_PATH}/portable/${RTOS_PORT_PATH}/port.c
    )

# Add extra if native/simulator 
if(NOT CMAKE_CROSSCOMPILING)
    target_sources(${MODULE_NAME}
        PRIVATE
            ${RTOS_MAIN_PATH}/portable/${RTOS_PORT_PATH}/utils/wait_for_event.c
        )
endif(NOT CMAKE_CROSSCOMPILING)

# Add memory management support, if required
if(configSUPPORT_DYNAMIC_ALLOCATION EQUAL 1)
    message(STATUS "   -> configSUPPORT_DYNAMIC_ALLOCATION is ON")
    if(configUSE_FreeRTOS_HEAP)
        if((configUSE_FreeRTOS_HEAP GREATER 5) OR (configUSE_FreeRTOS_HEAP EQUAL 0) )
            set(configUSE_FreeRTOS_HEAP 2)
            message(WARNING "   -> configUSE_FreeRTOS_HEAP invalid, change to ${configUSE_FreeRTOS_HEAP}")
        else((configUSE_FreeRTOS_HEAP GREATER 5) OR (configUSE_FreeRTOS_HEAP EQUAL 0) )
            message(STATUS "      -> configUSE_FreeRTOS_HEAP is ${configUSE_FreeRTOS_HEAP}")
        endif((configUSE_FreeRTOS_HEAP GREATER 5) OR (configUSE_FreeRTOS_HEAP EQUAL 0) )
    else(configUSE_FreeRTOS_HEAP)
        set(configUSE_FreeRTOS_HEAP 2)
        message(WARNING "   -> configUSE_FreeRTOS_HEAP undefined, set to ${configUSE_FreeRTOS_HEAP}")
    endif(configUSE_FreeRTOS_HEAP)
    target_sources(${MODULE_NAME} 
        PUBLIC 
            ${RTOS_MAIN_PATH}/portable/MemMang/heap_${configUSE_FreeRTOS_HEAP}.c
        )
endif(configSUPPORT_DYNAMIC_ALLOCATION EQUAL 1)

# Add mpu management support
if(configENABLE_MPU EQUAL 1)
    message(STATUS "   -> configENABLE_MPU is ON")
    target_sources(${MODULE_NAME} 
        PUBLIC 
            ${RTOS_MAIN_PATH}/portable/Common/mpu_wrappers.c
        )
endif(configENABLE_MPU EQUAL 1)

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR}
    PUBLIC
        ${RTOS_MAIN_PATH}/include
        ${RTOS_MAIN_PATH}/portable/${RTOS_PORT_PATH}
        ${RTOS_MAIN_PATH}/portable/${RTOS_PORT_PATH}/utils
        ${CMAKE_CURRENT_BINARY_DIR}
    )

# Add alias
add_library(3rd::${MODULE_NAME} ALIAS ${MODULE_NAME})

################################################################################
