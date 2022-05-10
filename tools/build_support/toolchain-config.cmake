find_package(native_toolchain REQUIRED)
find_package(cross_toolchain REQUIRED)

if( CMAKE_CROSSCOMPILING )
    set_cross_compiler()
else()
    set_native_compiler()
endif()

# Add compile definition
#add_compile_definitions("$<$<BOOL:${CMAKE_CROSSCOMPILING}>:__CROSS__>")
add_compile_definitions("$<$<CONFIG:Debug>:DEBUG>")
add_compile_definitions("$<$<BOOL:${USE_FREERTOS}>:__OS__>")
add_compile_definitions("$<$<BOOL:${USE_FREERTOS}>:OS_FreeRTOS=1>")
