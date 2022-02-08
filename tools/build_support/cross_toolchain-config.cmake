# Set cross compilation tool
macro(set_cross_compiler)
    set(processor arm)
    set(system none)
    set(variant eabi)    

    set(triple ${processor}-${system}-${variant})

    set(CMAKE_SYSTEM_PROCESSOR ${processor})
    set(CMAKE_C_COMPILER_TARGET ${triple})
    unset(COMPILER_CC)
    
    # COMPILER_CC
    find_program(
        COMPILER_CC ${CMAKE_C_COMPILER_TARGET}-gcc
        PATHS $ENV{CROSS_TOOL_PATH} PATH_SUFFIXES bin
        REQUIRED NO_DEFAULT_PATH NO_CACHE
        )

    get_filename_component(CMAKE_FIND_ROOT_PATH ${COMPILER_CC} DIRECTORY)

    set(CMAKE_C_COMPILER ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-gcc)
    set(CMAKE_ASM_COMPILER ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-gcc)
    set(CMAKE_CXX_COMPILER ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-g++)
    set(CMAKE_OBJCOPY ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-objcopy)    
    set(CMAKE_OBJDUMP ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-objdump)
    set(CMAKE_SIZE ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-size)
    set(CMAKE_AR ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-gcc-ar)
    #set(CMAKE_RANLIB ${CMAKE_FIND_ROOT_PATH}/${CMAKE_C_COMPILER_TARGET}-gcc-ranlib)

    # TODO : these following 3 lines break the search process of cmake find_file 
    #        function. 
    #        Commented for now  
    #set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    #set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    #set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
   
    # Set CMAKE_C_COMPILER_VERSION
    execute_process(COMMAND
        "${CMAKE_C_COMPILER}" -dumpversion
        RESULT_VARIABLE exit_code
        OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION
        ERROR_VARIABLE stderr
        OUTPUT_STRIP_TRAILING_WHITESPACE)

endmacro(set_cross_compiler)
