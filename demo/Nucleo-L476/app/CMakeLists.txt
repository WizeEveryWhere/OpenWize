################################################################################
# include the build support cmake
find_package(build_support REQUIRED)

set(MODULE_NAME DemoApp)

################################################################################

# Add executable 
add_executable(${MODULE_NAME})

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/app_entry.c
        src/storage.c
        sys/port.c
        sys/rtos.c
        sys/sys_init.c
        gen/parameters_cfg.c
        gen/parameters_default.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/sys
        ${CMAKE_CURRENT_SOURCE_DIR}/gen
    )

# Add some link flags
target_link_options(${MODULE_NAME}
    PUBLIC
        --specs=nano.specs 
        -u_printf_float 
        -Wl,-Map=${MODULE_NAME}.map
    )

################################################################################
setup_install(
    TARGET ${MODULE_NAME} 
    DEPENDS 
        Board_NucL476RG 
        phyfake 
        bsp 
        Openwize::Openwize
    )
