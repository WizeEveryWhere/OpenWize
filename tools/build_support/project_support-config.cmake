################################################################################
# if not given in command line, set the install prefix to default
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${TOP_INSTALL_DIR} CACHE INTERNAL "")
    message(STATUS "Set CMAKE_INSTALL_PREFIX to default : ${CMAKE_INSTALL_PREFIX}")
endif()

################################################################################
# Get build_support
find_package(build_support REQUIRED)

# if not given in command line
if(NOT BUILD_CFG_DIR)
    # if is given as ENV 
    if($ENV{BUILD_CFG_DIR})
        set(BUILD_CFG_DIR $ENV{BUILD_CFG_DIR} )
    else()
        # set Build Config directory
        set(BUILD_CFG_DIR "${CMAKE_SOURCE_DIR}/demo" )
    endif()
endif()


################################################################################
# Set options
option(DOC_COMPILE "Enable the Documentation compilation" OFF)

option(BUILD_OPENWIZE "" ON)
option(USE_FREERTOS "" OFF)


option(BUILD_DEMO "" OFF)

option(BUILD_TEST "" OFF)
option(BUILD_UNITTEST "" OFF)
option(BUILD_INTEGRATION_TEST "" OFF)
option(BUILD_SYSTEM_TEST "" OFF)

option(ENABLE_NATIVE_UNITTEST "Enable unit-test native execution" OFF)
option(ENABLE_EMEBED_UNITTEST "" OFF)

################################################################################
# Init build config
get_cfg()

if(USE_FREERTOS)
    set(BUILD_FREERTOS TRUE)
endif()

if(BUILD_TEST)
    set(BUILD_UNITY TRUE)
endif()

if(BUILD_OPENWIZE)
    set(BUILD_TINYCRYPT TRUE)
    set(BUILD_LZMA TRUE)
endif()

#-------------------------------------------------------------------------------
# testing/Unity (must be the first one)
if(BUILD_UNITY)
    message(STATUS "Add Unity Build ")
    add_subdirectory(third-party/testing/Unity)
endif()

# firmware/STM32
if(BUILD_STM)
    message(STATUS "Add STM32 HAL Build ")
    add_subdirectory(third-party/firmware/STM32)
endif()

# rtos/FreeRTOS
if(BUILD_FREERTOS)
    message(STATUS "Add FreeRTOS Build ")
    add_subdirectory(third-party/rtos/FreeRTOS)
endif()

# libraries/Tinycrypt
if(BUILD_TINYCRYPT)
    add_subdirectory(third-party/libraries/Tinycrypt)
endif()

# libraries/Lzma
if(BUILD_LZMA)
    add_subdirectory(third-party/libraries/Lzma)
endif()

# sources
if(BUILD_OPENWIZE)
    message(STATUS "Add OpenWize Build ")
    add_subdirectory(sources)
    list(APPEND DOC_SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/sources)
endif(BUILD_OPENWIZE)

#################################################################################
if(BUILD_INTEGRATION_TEST)   
	message(STATUS "Add iTest Build ")
	add_subdirectory(tests/iTests)
endif(BUILD_INTEGRATION_TEST)

if(ENABLE_NATIVE_UNITTEST)
    list(APPEND CMAKE_PREFIX_PATH ${TOOL_BUILD_SUPPORT}/external_helper)
    list(APPEND CMAKE_PREFIX_PATH ${TOOL_BUILD_SUPPORT}/native_helper)
    
    find_package(native_build_support REQUIRED)
    init_to_native()
    
    if(BUILD_UNITY)
        add_to_native(TARGET unity PATH "third-party/testing/Unity")
    endif()

    if(BUILD_FREERTOS)
        add_link_options(-pthread)
        add_to_native(TARGET freertos PATH "third-party/rtos/FreeRTOS")
    endif()

    if(BUILD_TINYCRYPT)
        add_to_native(TARGET unity PATH "third-party/libraries/Tinycrypt")
    endif()
    
    if(BUILD_LZMA)
        add_to_native(TARGET unity PATH "third-party/libraries/Lzma")
    endif()
    
    if(BUILD_OPENWIZE)
        add_to_native(TARGET openwize PATH "sources")
    endif(BUILD_OPENWIZE)

    setup_to_native()
endif(ENABLE_NATIVE_UNITTEST)

#################################################################################
# distclean target
set(cmake_generated ${CMAKE_BINARY_DIR}/CMakeCache.txt
                    ${CMAKE_BINARY_DIR}/cmake_install.cmake
                    ${CMAKE_BINARY_DIR}/Makefile
                    ${CMAKE_BINARY_DIR}/CMakeFiles
                    ${CMAKE_BINARY_DIR}/*
)
if(NOT TARGET distclean)
    add_custom_target(distclean COMMAND rm -rf ${cmake_generated} )
    set_property(TARGET distclean PROPERTY FOLDER "CMakePredefinedTargets")
endif()

#################################################################################
# ultraclean target => which is a rm -rf aBuild
if(NOT TARGET ultraclean)
    add_custom_target(ultraclean COMMAND rm -rf "${TOP_BUILD_DIR}/*" "${TOP_INSTALL_DIR}/*" )
    set_property(TARGET ultraclean PROPERTY FOLDER "CMakePredefinedTargets")
endif()


################################################################################
# Add doc rule
if(DOC_COMPILE)
    list(APPEND DOC_SOURCE_DIRS "${CMAKE_SOURCE_DIR}/docs")
    string(REPLACE ";" " " DOXYFILE_SOURCE_DIRS "${DOC_SOURCE_DIRS}")
    # Add build support for doxygen
    find_package(doxygen_support)
endif(DOC_COMPILE)


