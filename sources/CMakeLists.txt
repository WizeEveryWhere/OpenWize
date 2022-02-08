set(MODULE_NAME openwize)

cmake_minimum_required( VERSION 3.12 )
################################################################################

include(OpenWize_Options.cmake)

# Add compile definition
add_compile_definitions(WIZE_OPT_USE_CONST_ERR_MSG)

# Samples
add_subdirectory("Samples")
# WizeCore
add_subdirectory("WizeCore")

# OpenWize lib (including WizeCore, Crypto Crc_sw, ReedSolomon, TimeEvt, Parameters, ImgStorage, Logger libraries)
add_library(
    ${MODULE_NAME} STATIC 
    $<TARGET_OBJECTS:WizeCore::proto> 
    $<TARGET_OBJECTS:WizeCore::net>
    $<TARGET_OBJECTS:WizeCore::mgr>
    $<TARGET_OBJECTS:WizeCore::app>
    $<TARGET_OBJECTS:Samples::crc_sw>
    $<TARGET_OBJECTS:Samples::crypto>
    $<TARGET_OBJECTS:Samples::reedsolomon>
    $<TARGET_OBJECTS:Samples::imgstorage>
    $<TARGET_OBJECTS:Samples::parameters>
    $<TARGET_OBJECTS:Samples::timeevt>
    $<TARGET_OBJECTS:Samples::logger>
    )

target_link_libraries(
    ${MODULE_NAME} PUBLIC 
    WizeCore::proto 
    WizeCore::net
    WizeCore::mgr
    WizeCore::app
    Samples::crc_sw
    Samples::crypto
    Samples::reedsolomon
    Samples::imgstorage
    Samples::parameters
    Samples::timeevt
    Samples::logger
    3rd::freertos
    )

target_include_directories(
    ${MODULE_NAME} 
    INTERFACE
        ${CMAKE_BINARY_DIR}
    )

add_library(Openwize::Openwize ALIAS ${MODULE_NAME})
    
################################################################################

function(generate_conf_file conf_filename)
    message("   -> Generate : ${conf_filename}.h")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config.in/${conf_filename}.in
        ${CMAKE_BINARY_DIR}/${conf_filename}.h @ONLY
        )
endfunction(generate_conf_file)

generate_conf_file(OpenWize_config)
