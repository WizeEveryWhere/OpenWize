################################################################################
# Initialize the native environment build 
# In : none
#
# Out : 
#   - clear NATIVE_PATH_LIST in cache
#   - clear NATIVE_TARGET_LIST in cache
#   - set NATIVE_SUFFIX to "native"  in cache
#   
function(init_to_native)
    set(NATIVE_PATH_LIST CACHE INTERNAL "")
    set(NATIVE_TARGET_LIST CACHE INTERNAL "")
    set(NATIVE_SUFFIX "native" CACHE INTERNAL "")

    find_file(NATIVE_ENDIAN_FILE_IN endian.in )
    if(NATIVE_ENDIAN_FILE_IN)
        get_filename_component(NATIVE_ENDIAN_FILENAME ${NATIVE_ENDIAN_FILE_IN} NAME_WE)
        get_filename_component(NATIVE_ENDIAN_FILEPATH ${NATIVE_ENDIAN_FILE_IN} PATH)
        configure_file(
            ${NATIVE_ENDIAN_FILEPATH}/${NATIVE_ENDIAN_FILENAME}.in
            ${CMAKE_INSTALL_PREFIX}/_${NATIVE_SUFFIX}/include/machine/${NATIVE_ENDIAN_FILENAME}.h
            IMMEDIATE @ONLY
            )
    else()
        message(WARNING "${NATIVE_ENDIAN_FILE_IN}")            
    endif()
endfunction(init_to_native)

################################################################################
# Add target to install in native environment build
# In :
#   TARGET : the target name to add
#   PATH   : the target path to add
#
# Out : 
#   - the TARGET is added in NATIVE_TARGET_LIST list
#   - the PATH is added in NATIVE_PATH_LIST list
#   
function(add_to_native)
    set(options "")
    set(oneValueArgs PATH TARGET)
    set(multiValueArgs "")
    cmake_parse_arguments(ADD_TO_NATIVE 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    
    # Check if required PATH is given
    if(ADD_TO_NATIVE_PATH  STREQUAL "")
        message(FATAL_ERROR "Can't add to native list with empty module path")
    endif()

    if(ADD_TO_NATIVE_TARGET  STREQUAL "")
        message(FATAL_ERROR "Can't add to native list with empty module target")
    endif()
    
    set(NATIVE_PATH_LIST ${NATIVE_PATH_LIST} ${ADD_TO_NATIVE_PATH} CACHE INTERNAL "")
    set(NATIVE_TARGET_LIST ${NATIVE_TARGET_LIST} ${ADD_TO_NATIVE_TARGET} CACHE INTERNAL "")
    
endfunction(add_to_native)

################################################################################
# Setup (generate an ExternalPrj) the native environment
# In : none
#
# Out : native environment external project in env_NATIVE_SUFFIX
#   
function(setup_to_native)
    find_package(external_build_support REQUIRED)
    
    list(LENGTH NATIVE_PATH_LIST len1)
    math(EXPR len2 "${len1} - 1")
    message(STATUS "Native Env. Build support")
    foreach(i RANGE ${len2})
        list(GET NATIVE_TARGET_LIST ${i} t)
        list(GET NATIVE_PATH_LIST ${i} p)
        message("   -> add to native : ${t} : ${p}")
    endforeach()
    
    set(withALL FALSE)
    if(BUILD_TEST)
        # Build with "ALL"
        set(withALL TRUE)
    endif()
    setup_external(NAME env SOURCE ${CMAKE_SOURCE_DIR} BUILD ON NATIVE TRUE withALL ${withALL})
endfunction(setup_to_native)

################################################################################
