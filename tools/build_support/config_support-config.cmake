################################################################################
# Get the input configuration file
macro(get_cfg)  
    # Setup GIT info
    gitinfo("${TOP_DIR}")
    # Setup cfg
    message("****************************************")
    version_from_git(${TOOL_BUILD_SUPPORT} "version.h")

    if(NOT BUILD_CFG)
        # Check if an empty file with app name exist in the current build dir  
        set(app_file "${CMAKE_BINARY_DIR}/app")
        if (EXISTS "${app_file}")
            file(STRINGS ${app_file} APP LIMIT_COUNT 1)
            set(BUILD_CFG ${APP})
        endif()
    endif()

    message(STATUS "Selected build config : ")
    message("   -> File : \"${BUILD_CFG}\"")
    message("   -> From : \"${BUILD_CFG_DIR}\"")
	# check if given file exist
    set(build_cfg_file "${BUILD_CFG_DIR}/${BUILD_CFG}.cmake")
    if (NOT EXISTS "${build_cfg_file}")
        message(FATAL_ERROR "${build_cfg_file} not found\n")
    endif()
    
	# Add ".cmake" file
    include("${build_cfg_file}")

    message(STATUS "End of Selected build config")
    message("****************************************")
endmacro(get_cfg)
################################################################################
# Generate config file 
function(setup_config)
    set(options "")
    set(oneValueArgs SOURCE NAME DESTINATION)
    set(multiValueArgs "")
    cmake_parse_arguments(SETUP_CONFIG "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
   
    if(SETUP_CONFIG_NAME)
        message(STATUS "   -> Generate : ${SETUP_CONFIG_NAME}.h")
    else()
        message(FATAL_ERROR "setup_config file name is missing")
    endif()

    set(src_path "")
    if(SETUP_CONFIG_SOURCE)
        set(src_path ${SETUP_CONFIG_SOURCE}/)
    else()
        set(src_path ${CMAKE_CURRENT_SOURCE_DIR}/)
    endif()
    
    set(dest_path "")
    if(SETUP_CONFIG_DESTINATION)
        set(dest_path ${SETUP_CONFIG_DESTINATION}/)
    else()
        set(dest_path ${CMAKE_CURRENT_BINARY_DIR}/)
    endif()

    configure_file(
        ${src_path}${SETUP_CONFIG_NAME}.in
        ${dest_path}${SETUP_CONFIG_NAME}.h @ONLY
        )
endfunction(setup_config)
################################################################################
