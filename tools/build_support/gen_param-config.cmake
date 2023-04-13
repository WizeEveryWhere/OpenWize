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
        message("   -----------------------------")
        message("   -> Generate Parameters Tables")
        # Check if required SOURCE is given
        if(GEN_PARAM_SOURCE STREQUAL "")
            message(FATAL_ERROR "Can't generate parameters without SOURCE files")
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
        
        find_program(XMLSTARLET xmlstarlet REQUIRED)
        if(NOT XMLSTARLET)
            message(FATAL_ERROR "xmlstarlet not found!\nInstall it :\n\t sudo apt-get install xmlstarlet\n")
        endif()
        
        find_program(XMLMERGE xmlmerge REQUIRED)
        if(NOT XMLMERGE)
            message(FATAL_ERROR "xmlmerge not found!\nInstall it :\n\t sudo apt-get install gwenhywfar-tools\n")
        endif()

        message("      -> From :")
        string(REPLACE " " ";" PARAM_XML_FILE_LIST ${GEN_PARAM_SOURCE})
        foreach(file_name ${PARAM_XML_FILE_LIST})
            message("         - ${file_name}")
        endforeach()
        
        execute_process (
            COMMAND 
                bash -c "mkdir -p ${GEN_PARAM_DESTINATION}/gen; xmlmerge -o ${GEN_PARAM_DESTINATION}/gen/.MergedParam.xml ${GEN_PARAM_SOURCE}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )

        execute_process (
            COMMAND 
                bash -c "export IS_VERBOSE_ENV=${verbo}; export PATH=$PATH:./tools/scripts/gen_param:./third-party/.OpenWize/tools/scripts/gen_param; gen_table.sh --in ${GEN_PARAM_DESTINATION}/gen/.MergedParam.xml --dest ${GEN_PARAM_DESTINATION};"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
        message("   -----------------------------")
    endif()
endfunction()
