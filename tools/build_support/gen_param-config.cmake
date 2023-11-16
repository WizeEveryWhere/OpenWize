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
    set(oneValueArgs SOURCE DESTINATION OPT)
    set(multiValueArgs "")
    cmake_parse_arguments(GEN_PARAM 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(GENERATE_PARAM)
        message("   -----------------------------")
        message("   -> Generate Parameters Tables (to : \"${GEN_PARAM_DESTINATION}\" directory)")

        # Check if required SOURCE is given
        if(GEN_PARAM_DESTINATION STREQUAL "")
            message(FATAL_ERROR "Can't generate parameters without DESTINATION dir")
        endif()

        # Check if required SOURCE is given
        if(GEN_PARAM_SOURCE STREQUAL "")
            message(FATAL_ERROR "Can't generate parameters without SOURCE files")
        endif()
        
        if(GEN_PARAM_OPT)
            set(opt " --setup ")
        else()
            set(opt "")
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
        string(REPLACE " " ";" local_PARAM_XML_FILE_LIST ${GEN_PARAM_SOURCE})
        foreach(file_name ${local_PARAM_XML_FILE_LIST})
            message("         - ${file_name}")
        endforeach()
        
        execute_process (
            COMMAND 
                bash -c "mkdir -p ${GEN_PARAM_DESTINATION}/gen; xmlmerge -o ${GEN_PARAM_DESTINATION}/gen/.MergedParam.xml ${GEN_PARAM_SOURCE}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
        
        execute_process (
            COMMAND 
                bash -c "export IS_VERBOSE_ENV=${verbo}; export PATH=$PATH:./tools/scripts/gen_param:./third-party/.OpenWize/tools/scripts/gen_param; gen_table.sh ${opt} --in ${GEN_PARAM_DESTINATION}/gen/.MergedParam.xml --dest ${GEN_PARAM_DESTINATION};"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
        message("   -----------------------------")
    endif()
endfunction()

################################################################################
# Find and add one parameter xml file to the given PARENT_SCOPE list.
# 
# The file is search :
#    - First into CUSTOM_DIR
#    - Then into DEFAULT_CFG_FILE_DIR
# If the DEFAULT_CFG_FILE_DIR variable is not previously set, the default will
# be to "sources/app".
#
# In :
#   SOURCE      : the xml file name with ".xml" and no path
#   DESTINATION : the output list name to store into
# Out : 
#   The list of xml file name with a relative path to CMAKE_CURRENT_SOURCE_DIR
#
function(add_xml_cfg)
    set(options "")
    set(oneValueArgs SOURCE DESTINATION)
    set(multiValueArgs "")
    cmake_parse_arguments(XML_CFG 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})  
    
    if(NOT DEFAULT_CFG_FILE_DIR)
        set(DEFAULT_CFG_FILE_DIR "sources/app")
    endif()
    
    if(NOT CUSTOM_DIR)
        set(CUSTOM_DIR "never_found_path")
    endif()
    
    set(SEARCH_DIR ${CUSTOM_DIR} ${DEFAULT_CFG_FILE_DIR})
    
    find_file(XML_CFG_FILE ${XML_CFG_SOURCE} PATHS ${SEARCH_DIR} PATH_SUFFIXES cfg NO_DEFAULT_PATH )
    
    if(XML_CFG_FILE-NOTFOUND)
        message(WARNING "${XML_CFG_SOURCE} file not found")
    else()
        file(RELATIVE_PATH XML_REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${XML_CFG_FILE} )
        set(${XML_CFG_DESTINATION} "${${XML_CFG_DESTINATION}} ${XML_REL_PATH}" PARENT_SCOPE)
        #message("- ${XML_CFG_FILE}")
    endif()
    unset(XML_CFG_FILE CACHE)
    
endfunction(add_xml_cfg)
