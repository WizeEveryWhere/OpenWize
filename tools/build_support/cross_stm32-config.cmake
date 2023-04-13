# Set STM32/arm specific compile options
macro(set_cross_stm32)
    # check the mcu/arch vendor
    if(NOT VENDOR_MCU)
        message(FATAL_ERROR "VENDOR_MCU is not defined")
    endif(NOT VENDOR_MCU)

    string(FIND ${VENDOR_MCU} "STM32L4" IS_L4)
    if(${IS_L4} EQUAL 0)
        set(VENDOR_MCU_ARCH "STM32L4")
        set(MCU_ARCH cortex-m4)
        string(TOLOWER ${VENDOR_MCU_ARCH} lower_VENDOR_MCU_ARCH)
    else()        
        message(FATAL_ERROR "${VENDOR_MCU} is not (yet) compatible")
    endif()

    set(BUILD_STM TRUE)
    # ---
    message("      -> Use VENDOR_MCU     : \"${VENDOR_MCU}\"")
 
    if(NOT MCU_FLOAT_ABI)
        set(MCU_FLOAT_ABI hard)
        set(tmp "Set")
    else(NOT MCU_FLOAT_ABI)
        set(tmp "Use")
    endif(NOT MCU_FLOAT_ABI)
    message("      -> ${tmp} MCU_FLOAT_ABI  : \"${MCU_FLOAT_ABI}\"")

    if(NOT MCU_FPU)
        set(MCU_FPU fpv4-sp-d16)
        set(tmp "Set")
    else(NOT MCU_FPU)
        set(tmp "Use")
    endif(NOT MCU_FPU)
    message("      -> ${tmp} MCU_FPU        : \"${MCU_FPU}\"")

    if(NOT MCU_ENDIANESS)
        set(MCU_ENDIANESS little)
        set(tmp "Set")
    else(NOT MCU_ENDIANESS)
        set(tmp "Use")
    endif(NOT MCU_ENDIANESS)
    message("      -> ${tmp} MCU_ENDIANESS  : \"${MCU_ENDIANESS}\"")
     
    set(COMMON_FLAGS "${COMMON_FLAGS} -mcpu=${MCU_ARCH}")
    set(COMMON_FLAGS "${COMMON_FLAGS} -mthumb -m${MCU_ENDIANESS}-endian -mfloat-abi=${MCU_FLOAT_ABI} ")
    if(MCU_FLOAT_ABI STREQUAL hard)
        set(COMMON_FLAGS "${COMMON_FLAGS} -mfpu=${MCU_FPU}")
    endif()
    set(COMMON_FLAGS "${COMMON_FLAGS} -D${VENDOR_MCU}")
    
    set(COMMON_LINKER_FLAGS "")
    #set(COMMON_LINKER_FLAGS " -Wl,--gc-sections -static ")
    set(COMMON_LINKER_FLAGS " -static ")
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMMON_LINKER_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
    
    # Add compile standard
    # FIXME : => doesn't work with ExternalProject
    #set(CMAKE_C_STANDARD 99) 
    #set(CMAKE_CXX_STANDARD 11)
    #message("CMAKE_C_COMPILER_ID : ${CMAKE_C_COMPILER_ID}")
    #add_compile_options("$<$<CMAKE_C_COMPILER_ID:GNU>:-std=gnu99>")
    #add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-std=c++11>")
    #add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-std=gnu++11>")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endmacro(set_cross_stm32 show_me)

