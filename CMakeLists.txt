cmake_minimum_required( VERSION 3.12 ) 

################################################################################
# set top module name, info...
set(MODULE_NAME OpenWizeBuilder)
set(MODULE_DESCRIPTION "\"This project is...\"")
set(MODULE_URL "http://www.OpenWize.fr")
set(MODULE_VER_MAJOR 1)
set(MODULE_VER_MINOR 0)
set(MODULE_VER_PATCH 0)

file(RELATIVE_PATH PROJECT_LOGO "${CMAKE_SOURCE_DIR}/Doc" "${CMAKE_SOURCE_DIR}/Doc/logo_GRDF_150-89.png")
file(RELATIVE_PATH PARTNER_LOGO "${CMAKE_SOURCE_DIR}/Doc" "${CMAKE_SOURCE_DIR}/Doc/logo_Wize_alliance.png")


################################################################################
# set top directory
get_filename_component(__root_dir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
set(TOP_DIR "${__root_dir}" CACHE INTERNAL "")

# Set default build and install dirs
set(TOP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "")
set(TOP_INSTALL_DIR "${TOP_DIR}/_install" CACHE INTERNAL "")
set(CMAKE_INSTALL_MESSAGE LAZY) # message of only newly installed

# set build support directory
set(TOOL_BUILD_SUPPORT "${TOP_DIR}/tools/build_support")

# Add build support to module search path
list(APPEND CMAKE_PREFIX_PATH ${TOOL_BUILD_SUPPORT})

################################################################################
project(
    ${MODULE_NAME} 
    VERSION ${MODULE_VER_MAJOR}.${MODULE_VER_MINOR}.${MODULE_VER_PATCH} 
    #DESCRIPTION ${MODULE_DESCRIPTION} 
    #HOMEPAGE_URL ${MODULE_URL} 
    LANGUAGES C ASM
    )

if(NOT IS_EXTERNAL_PRJ)
    find_package(project_support)
else()
    list(APPEND CMAKE_PREFIX_PATH ${TOOL_BUILD_SUPPORT}/external_helper)
    list(APPEND CMAKE_PREFIX_PATH ${TOOL_BUILD_SUPPORT}/native_helper)
    
    # Add install include and lib directories for all targets
    get_property(INSTALL_PATH GLOBAL PROPERTY INSTALL_DIR )
    include_directories(${INSTALL_PATH}/include ${INSTALL_PATH}/include/private)
    link_directories(${INSTALL_PATH}/lib)
    
    include(CTest)
    find_package(native_support)
endif()

################################################################################
