# Set native compilation
macro (set_native_compiler)
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_ASM_COMPILER "gcc")
    # Set processor CFLAG
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -std=gnu99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32 -Wl,--gc-sections ")
endmacro(set_native_compiler)
