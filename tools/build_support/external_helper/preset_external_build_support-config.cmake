if(IS_EXTERNAL_PRJ)
    # Get build_support
    find_package(build_support REQUIRED)
    # Get toolchain
    find_package(toolchain REQUIRED)
    
    # Propagate compile otions and definitions
    add_compile_options("${COMPILE_OPTIONS}")
    add_compile_definitions("${COMPILE_DEFINITIONS}")
    
    # Add some includes
    include_directories(AFTER ${CMAKE_BINARY_DIR}/../..)
    include_directories(
        ${CMAKE_INSTALL_PREFIX}/include 
        ${CMAKE_INSTALL_PREFIX}/include/private
        )
    
    link_directories(
        ${CMAKE_INSTALL_PREFIX}/lib
        )
    
    message(STATUS "External Project : ${PROJECT_NAME}" )
    message("   -> Target : ${MODULE_NAME}" )
   
endif(IS_EXTERNAL_PRJ)