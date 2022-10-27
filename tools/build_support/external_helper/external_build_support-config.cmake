
################################################################################
include(ExternalProject)

function(setup_external)
    set(options NATIVE BUILD withALL)
    set(oneValueArgs SOURCE DESTINATION NAME)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(SETUP_EXTERNAL 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    #[[
    message("NAME        : ${SETUP_EXTERNAL_NAME} ")
    
    message("BUILD       : ${SETUP_EXTERNAL_BUILD} ")
    message("withALL     : ${SETUP_EXTERNAL_withALL} ")
    message("NATIVE      : ${SETUP_EXTERNAL_NATIVE} ")
    
    message("SOURCE      : ${SETUP_EXTERNAL_SOURCE} ")
    message("DEPENDS     : ${SETUP_EXTERNAL_DEPENDS} ")
    message("DESTINATION : ${SETUP_EXTERNAL_DESTINATION} ")
    ]]
    
    if(NATIVE_SUFFIX STREQUAL "")
        set(NATIVE_SUFFIX "native")
    endif()
    
    # Check if native
    if(SETUP_EXTERNAL_NATIVE)
        set(suffx "_${NATIVE_SUFFIX}")
    else()
        set(suffx "")    
    endif()
    
    # Check if build is exclude from ALL
    if(SETUP_EXTERNAL_withALL)
        set(ext_exclude_from_all FALSE)
    else()
        set(ext_exclude_from_all TRUE)
    endif()
    
    # Check if required NAME is given
    if(SETUP_EXTERNAL_NAME  STREQUAL "")
        message(FATAL_ERROR "Can't add ExternalProject with empty module NAME")
    endif()

    # Check if required SOURCE path is given
    if(SETUP_EXTERNAL_SOURCE STREQUAL "")
        message(FATAL_ERROR "Can't add ExternalProject with empty module SOURCE")
    endif()    

    set(ext_name        ${SETUP_EXTERNAL_NAME}${suffx})
    set(ext_source_dir  ${SETUP_EXTERNAL_SOURCE})
    set(ext_binary_dir  ${CMAKE_BINARY_DIR}/${ext_name})
    set(ext_install_dir ${TOP_INSTALL_DIR}/${suffx})

    set(ext_stamp_dir  ${ext_binary_dir}/${ext_name}/stamp)
    set(ext_tmp_dir    ${ext_binary_dir}/${ext_name}/tmp)
    set(ext_dwn_dir    ${ext_binary_dir}/${ext_name}/src)
  
    # Is build or headers only    
    if(SETUP_EXTERNAL_BUILD)
        set(build_module "")
    else()
        set(build_module "(headers only)")
    endif() 

    # Add dependencies, if any
    unset(depend_list CACHE)
    if(SETUP_EXTERNAL_DEPENDS)
        foreach(dep ${SETUP_EXTERNAL_DEPENDS})
            list(APPEND depend_list ${dep}${suffx})
        endforeach(dep)
        message ("   -> setup external : ${ext_name} ${build_module} : depend on \"${depend_list}\"")
    else()
        set(depend_list "")
        message ("   -> setup external : ${ext_name} ${build_module}")
    endif()
    
    # Get compile options and definitions to propagate
    get_directory_property(cmp_option COMPILE_OPTIONS)
    get_directory_property(cmp_def COMPILE_DEFINITIONS)
    
    # Get link options to propagate
    get_directory_property(lnk_option LINK_OPTIONS)
    
    #     
    set(OPTIONS_CACHE_ARGS
        # --- RTOS ---
        #-DRTOS_MAIN_PATH:PATH=${RTOS_MAIN_PATH}
        #-DRTOS_PORT_PATH:PATH=${RTOS_PORT_PATH}
        -DRTOS_CONF_FILE:FILEPATH=${RTOS_CONF_FILE}
        # --- Modules ---
        -DIS_LOGGER_ENABLE:BOOL=${IS_LOGGER_ENABLE}
        -DUSE_LOGGER_SAMPLE:BOOL=${USE_LOGGER_SAMPLE}
        -DUSE_FREERTOS_SAMPLE:BOOL=${USE_FREERTOS_SAMPLE}  
        -DUSE_CRYPTO_SAMPLE:BOOL=${USE_CRYPTO_SAMPLE}
        -DUSE_CRC_SAMPLE:BOOL=${USE_CRC_SAMPLE}
        -DUSE_REEDSOLOMON_SAMPLE:BOOL=${USE_REEDSOLOMON_SAMPLE}
        -DUSE_PARAMETERS_SAMPLE:BOOL=${USE_PARAMETERS_SAMPLE}
        -DUSE_IMGSTORAGE_SAMPLE:BOOL=${USE_IMGSTORAGE_SAMPLE}
        -DUSE_TIMEEVT_SAMPLE:BOOL=${USE_TIMEEVT_SAMPLE}             
        # --- ---
        -DTOP_DIR:PATH=${TOP_DIR} 
        # --- ---
        -DBUILD_TEST:BOOL=${BUILD_TEST}
        
        -DNATIVE_PATH_LIST:STRING=${NATIVE_PATH_LIST}
        -DNATIVE_TARGET_LIST:STRING=${NATIVE_TARGET_LIST}
        
        )

    set(CACHE_ARGS 
        -DIS_BUILD_MODULE:BOOL=${SETUP_EXTERNAL_BUILD} 
        -DIS_EXTERNAL_PRJ:BOOL=TRUE
        # --- Build support ---
        -DCMAKE_MODULE_PATH:STRING=${CMAKE_MODULE_PATH}
        -DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}
        # --- Compile options and definitions ---
        -DCOMPILE_OPTIONS:STRING=${cmp_option}
        -DCOMPILE_DEFINITIONS:STRING=${cmp_def}
        # --- Link options ---
        -DLINK_OPTIONS:STRING=${lnk_option}
        )

    if(NOT SETUP_EXTERNAL_NATIVE)
        set(CACHE_ARGS
            ${CACHE_ARGS} 
            # --- ---
            -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
            -DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}
            -DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}
            # --- Compiler ---
            -DCMAKE_C_COMPILER_TARGET:STRING=${CMAKE_C_COMPILER_TARGET} 
            -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER} 
            -DCMAKE_ASM_COMPILER:STRING=${CMAKE_ASM_COMPILER} 
            -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER} 
            -DCMAKE_OBJCOPY:STRING=${CMAKE_OBJCOPY} 
            -DCMAKE_OBJDUMP:STRING=${CMAKE_OBJDUMP}
            -DCMAKE_SIZE:STRING=${CMAKE_SIZE}
            -DCMAKE_AR:STRING=${CMAKE_AR}
            # --- Compile and link flags ---
            -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
            -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
            -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
            )
    endif()
      
    ExternalProject_Add(
        ${ext_name} 
        PREFIX ${ext_binary_dir}
        SOURCE_DIR ${ext_source_dir}
        BINARY_DIR ${ext_binary_dir}
        INSTALL_DIR ${ext_install_dir}
        STAMP_DIR ${ext_stamp_dir}
        TMP_DIR ${ext_tmp_dir}
        DOWNLOAD_DIR ${ext_dwn_dir}
        BUILD_IN_SOURCE 0
        #LOG ON
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${ext_install_dir}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
            -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
        CMAKE_CACHE_ARGS
            ${OPTIONS_CACHE_ARGS}          
            ${CACHE_ARGS}
        DEPENDS ${depend_list}
        EXCLUDE_FROM_ALL ${ext_exclude_from_all}
        #STEP_TARGETS build
        #TEST_AFTER_INSTALL TRUE
        TEST_BEFORE_INSTALL TRUE
        )

    find_file(EXTERNAL_CMAKE_UNINSTALL external_cmake_uninstall.cmake.in )
    if(EXTERNAL_CMAKE_UNINSTALL)
        get_filename_component(EXTERNAL_CMAKE_UNINSTALL_FILENAME ${EXTERNAL_CMAKE_UNINSTALL} NAME_WE)
        get_filename_component(EXTERNAL_CMAKE_UNINSTALL_FILEPATH ${EXTERNAL_CMAKE_UNINSTALL} PATH)
    
        configure_file(
            "${EXTERNAL_CMAKE_UNINSTALL_FILEPATH}/${EXTERNAL_CMAKE_UNINSTALL_FILENAME}.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/${ext_name}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY
            )
        add_custom_target(
            ${ext_name}-uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/${ext_name}/cmake_uninstall.cmake
            )
    endif()
endfunction(setup_external)
