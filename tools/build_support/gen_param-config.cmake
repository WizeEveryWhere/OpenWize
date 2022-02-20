include(CMakeDependentOption)
option(GENERATE_PARAM "Enable the default parameters auto-generation" OFF)
cmake_dependent_option(GENERATE_PARAM_VERBOSE "Enable verbosity when generate parameters." OFF "GENERATE_PARAM" OFF)

################################################################################
# Generate the parameters default tables
# In :
#   SOURCE      : 
#   DESTINATION : 
# Out : 
#   
function(gen_param)
    set(options "")
    set(oneValueArgs SOURCE DESTINATION)
    set(multiValueArgs "")
    cmake_parse_arguments(GEN_PARAM 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(GENERATE_PARAM)        
        message("Generate default parameters")
    
        # Check if required SOURCE is given
        if(GEN_PARAM_SOURCE STREQUAL "")
            message(FATAL_ERROR "Can't generate parameters without SOURCE dir")
        endif()
        
        # Check if required SOURCE is given
        if(GEN_PARAM_DESTINATION STREQUAL "")
            message(FATAL_ERROR "Can't generate parameters without DESTINATION dir")
        endif()
        
        if(GENERATE_PARAM_VERBOSE)
            set(verbo 1)
        else()
            set(verbo 0)
        endif()
        
        execute_process (
            COMMAND 
                bash -c "xmlmerge ${GEN_PARAM_SOURCE}/DefaultParams.xml ${GEN_PARAM_SOURCE}/DefaultRestr.xml -o ${GEN_PARAM_SOURCE}/MergedParam.xml"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
    
        execute_process (
            COMMAND 
                bash -c "export IS_VERBOSE_ENV=${verbo}; export PATH=$PATH:./third-party/.OpenWize/tools/scripts/gen_param; gen_table.sh --in ${GEN_PARAM_SOURCE}/MergedParam.xml --dest ${GEN_PARAM_DESTINATION};"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
    endif()
endfunction()
