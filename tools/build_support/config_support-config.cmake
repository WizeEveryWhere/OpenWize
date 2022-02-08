################################################################################
# Get the input configuration file
macro(get_cfg)  
    # Setup GIT info
    gitinfo("${TOP_DIR}")
    # Setup cfg
    message("****************************************")
    version_from_git(${TOOL_BUILD_SUPPORT} "version.h")
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
# Get SHA and tag from git
function(version_from_git config_file_path version_file)
    message(STATUS "Generate ${version_file} file")
    foreach(var_name ${_state_variable_names})
        set(${var_name} $ENV{${var_name}})
        if(SHOW_GITINFO)
            message(STATUS "   -> Set ${var_name} : ${${var_name}}")
        endif()
    endforeach()
    configure_file("${config_file_path}/version.in" "${version_file}" @ONLY)
endfunction(version_from_git)
################################################################################
