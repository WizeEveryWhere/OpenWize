################################################################################
 
set(MODULE_NAME stm32_xx)

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

# Rename the project with the correct mcuarch
set(MODULE_NAME ${lower_VENDOR_MCU_ARCH})
string(APPEND  MODULE_NAME  "xx")

################################################################################
# include the build support cmake
find_package(build_support REQUIRED)

# Request to configure the file
if(HAL_CONF_FILE)
    if(EXISTS ${HAL_CONF_FILE})
        message("   -> Use HAL_CONF_FILE : ${HAL_CONF_FILE}")
        include("${HAL_CONF_FILE}")
    else(EXISTS ${HAL_CONF_FILE})
        message(FATAL_ERROR "HAL_CONF_FILE doesn't exist")
    endif(EXISTS ${HAL_CONF_FILE})
else(HAL_CONF_FILE)
    message(FATAL_ERROR "HAL_CONF_FILE is not defined")
endif(HAL_CONF_FILE)

# Request to configure the file
set(CONF_FILE_NAME "${lower_VENDOR_MCU_ARCH}")
string(APPEND  CONF_FILE_NAME  "xx_hal_conf")
generate_conf_file(${CONF_FILE_NAME})
generate_conf_file(stm32_assert)

# Add some compilation flags
set(${MODULE_NAME}_C_FLAGS "")
if(HAL_MODULE_ENABLED)
    set(${MODULE_NAME}_C_FLAGS "${${MODULE_NAME}_C_FLAGS} -DUSE_HAL_DRIVER ")
endif()

################################################################################
# Path definition
# HAL/LL
set(HAL_PATH  "${lower_VENDOR_MCU_ARCH}xx_hal_driver")
set(__HAL_INCLUDE_PATH "${HAL_PATH}/Inc")
set(__HAL_SOURCE_PATH "${HAL_PATH}/Src")

# CMSIS (core)
set(CMSIS_PATH "cmsis_core")
set(__CMSIS_INCLUDE_PATH "${CMSIS_PATH}/Include")

# CMSIS (device)
set(CMSIS_DEVICE_PATH  "cmsis_device_l4")
set(__CMSIS_DEVICE_INCLUDE_PATH "${CMSIS_DEVICE_PATH}/Include")
set(__CMSIS_DEVICE_SOURCE_PATH "${CMSIS_DEVICE_PATH}/Source/Templates")
set(__CMSIS_DEVICE_ASM_PATH "${CMSIS_DEVICE_PATH}/Source/Templates/gcc")

################################################################################
# HAL
set(SUB_MOD "_hal")
file( GLOB ${MODULE_NAME}${SUB_MOD}_SOURCES "${__HAL_SOURCE_PATH}/*${SUB_MOD}*.c" "${__HAL_SOURCE_PATH}/Legacy/*.c" )
file( GLOB ${MODULE_NAME}${SUB_MOD}_HEADERS "${__HAL_INCLUDE_PATH}/*${SUB_MOD}*.h" )

# Build 
add_library( ${MODULE_NAME}${SUB_MOD} STATIC ${${MODULE_NAME}${SUB_MOD}_SOURCES} )

target_include_directories(
    ${MODULE_NAME}${SUB_MOD} PUBLIC
    ${__HAL_SOURCE_PATH} 
    ${__HAL_INCLUDE_PATH}/Legacy 
    ${__HAL_INCLUDE_PATH} 
    ${__CMSIS_INCLUDE_PATH} 
    ${__CMSIS_DEVICE_INCLUDE_PATH} 
    ${CMAKE_CURRENT_BINARY_DIR} 
    )
# Add alias
add_library(3rd::${MODULE_NAME}${SUB_MOD} ALIAS ${MODULE_NAME}${SUB_MOD})

###################
# LL
set(SUB_MOD "_ll")
file( GLOB ${MODULE_NAME}${SUB_MOD}_SOURCES "${__HAL_SOURCE_PATH}/*${SUB_MOD}*.c" )
file( GLOB ${MODULE_NAME}${SUB_MOD}_HEADERS "${__HAL_INCLUDE_PATH}/*${SUB_MOD}*.h" )

# Build 
add_library( ${MODULE_NAME}${SUB_MOD} STATIC ${${MODULE_NAME}${SUB_MOD}_SOURCES} )
    
target_include_directories(
    ${MODULE_NAME}${SUB_MOD} PUBLIC 
    ${__LL_SOURCE_PATH} 
    ${__LL_INCLUDE_PATH} 
    ${__HAL_INCLUDE_PATH} 
    ${__CMSIS_INCLUDE_PATH} 
    ${__CMSIS_DEVICE_INCLUDE_PATH} 
    ${CMAKE_CURRENT_BINARY_DIR} 
    )
# Add alias
add_library(3rd::${MODULE_NAME}${SUB_MOD} ALIAS ${MODULE_NAME}${SUB_MOD})

###################
# CMSIS
set(SUB_MOD "_cmsis")
file( GLOB ${MODULE_NAME}${SUB_MOD}_SOURCES "${__CMSIS_DEVICE_SOURCE_PATH}/system_${lower_VENDOR_MCU_ARCH}xx.c" )
file( GLOB ${MODULE_NAME}${SUB_MOD}_HEADERS "${__CMSIS_DEVICE_INCLUDE_PATH}/*.h" "${__CMSIS_INCLUDE_PATH}/*.h" )

# Build 
add_library( ${MODULE_NAME}${SUB_MOD} STATIC ${${MODULE_NAME}${SUB_MOD}_SOURCES} )
    
target_include_directories(
    ${MODULE_NAME}${SUB_MOD} PUBLIC
    ${__CMSIS_INCLUDE_PATH}     
    ${__CMSIS_DEVICE_INCLUDE_PATH} 
    ${__HAL_INCLUDE_PATH} 
    ${CMAKE_CURRENT_BINARY_DIR} 
    )
# Add alias
add_library(3rd::${MODULE_NAME}${SUB_MOD} ALIAS ${MODULE_NAME}${SUB_MOD})
