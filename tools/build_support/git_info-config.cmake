 
if(NOT DEFINED GIT_EXECUTABLE)
    find_package(Git QUIET REQUIRED)
endif()

option(GET_GIT_AUTHOR_NAME "Get and add the GIT_AUTHOR_NAME" OFF)
option(GET_GIT_AUTHOR_EMAIL "Get and add the GIT_AUTHOR_EMAIL" OFF)

option(GET_GIT_COMMIT_SUBJECT "Get and add the GIT_COMMIT_SUBJECT" OFF)
option(GET_GIT_COMMIT_BODY "Get and add the GIT_COMMIT_BODY" OFF)

set(_state_variable_names
    GIT_RETRIEVED_STATE
    GIT_HEAD_SHA1
    GIT_DESCRIBE
    GIT_IS_DIRTY
    GIT_TAG
    # ************
    GIT_FW_VER_TYPE
    GIT_FW_VER_MAJ
    GIT_FW_VER_MIN
    GIT_FW_VER_REV
    # ************
    GIT_AUTHOR_NAME
    GIT_AUTHOR_EMAIL
    GIT_COMMIT_DATE_ISO8601
    GIT_COMMIT_SUBJECT
    GIT_COMMIT_BODY
)

macro(RunGitCommand)
    execute_process(COMMAND
        "${GIT_EXECUTABLE}" ${ARGV}
        WORKING_DIRECTORY "${_working_dir}"
        RESULT_VARIABLE exit_code
        OUTPUT_VARIABLE output
        ERROR_VARIABLE stderr
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT exit_code EQUAL 0)
        set(ENV{GIT_RETRIEVED_STATE} "false")

        # Issue 26: git info not properly set
        #
        # Check if we should fail if any of the exit codes are non-zero.
        if(GIT_FAIL_IF_NONZERO_EXIT)
            string(REPLACE ";" " " args_with_spaces "${ARGV}")
            message(FATAL_ERROR "${stderr} (${GIT_EXECUTABLE} ${args_with_spaces})")
        endif()
    endif()
    if(${SHOW_GITCMD})
        message("Command : ${GIT_EXECUTABLE} ${ARGV}")
        message(" -> exit_code : ${exit_code}")
        message(" -> stderr    : ${stderr}")
        message(" -> Output    : ${output}")
    endif()
endmacro()

# Get SHA and tag from git and generate .h file
function(version_from_git config_file_path version_file)
    message(STATUS "Generate ${version_file} file")
    foreach(var_name ${_state_variable_names})
        set(${var_name} $ENV{${var_name}})
        if(SHOW_GITINFO)
            message(STATUS "   -> Set ${var_name} : ${${var_name}}")
        endif()
    endforeach()
    configure_file("${config_file_path}/version.in" "${version_file}" @ONLY)
endfunction(version_from_git)

# Get info from git
function(gitinfo _working_dir)
    set(ENV{GIT_RETRIEVED_STATE} "true")
    
    set(object HEAD)
    
    foreach(var_name ${_state_variable_names})
        if(${var_name} STREQUAL "GIT_HEAD_SHA1")
            RunGitCommand(show -s "--format=%H" ${object})
            if(exit_code EQUAL 0)
                set(ENV{GIT_HEAD_SHA1} ${output})
            endif()
        endif()
        if(${var_name} STREQUAL "GIT_DESCRIBE")
            RunGitCommand(describe --always ${object})
            if(NOT exit_code EQUAL 0)
                set(ENV{GIT_DESCRIBE} "unknown")
            else()
                set(ENV{GIT_DESCRIBE} "${output}")
            endif()
        endif()
        if(${var_name} STREQUAL "GIT_IS_DIRTY")
            RunGitCommand(status --porcelain)
            if(NOT exit_code EQUAL 0)
                set(ENV{GIT_IS_DIRTY} "0")
            else()
                if(NOT "${output}" STREQUAL "")
                    set(ENV{GIT_IS_DIRTY} "1")
                else()
                    set(ENV{GIT_IS_DIRTY} "0")
                endif()
            endif()
        endif()
        
        if(${var_name} STREQUAL "GIT_TAG")
            RunGitCommand(describe --exact-match --tag ${object})
            set(TAG_FOUND FALSE)
            set(BAD_FORMATTING FALSE)
            
            if(exit_code EQUAL 0)
                set(TAG_FOUND TRUE)
                set(ENV{GIT_TAG} ${output})
                #string(REPLACE "." "" output "${output}")
                string(FIND ${output} "-" loc)
                
                if(${loc} GREATER_EQUAL 0)
                    math(EXPR loc "${loc}+1") 
                    string(SUBSTRING ${output} 0 ${loc} typeStr)
                    string(REPLACE ${typeStr} "" output_LIST "${output}")
                    string(REPLACE "-" "" typeStr "${typeStr}")               
                    string(REPLACE "." ";" output_LIST "${output_LIST}")
                    
                    list(LENGTH output_LIST len)
                    if(${len} EQUAL 3)
                        list(GET output_LIST 2 GIT_FW_VER_REV_V)
                        list(GET output_LIST 1 GIT_FW_VER_MIN_V)
                        list(GET output_LIST 0 GIT_FW_VER_MAJ_V)
                        set(GIT_FW_VER_TYPE_V ${typeStr})
                    else()
                        set(GIT_FW_VER_REV_V "00")
                        set(GIT_FW_VER_MIN_V "00")
                        set(GIT_FW_VER_MAJ_V "00")
                        set(GIT_FW_VER_TYPE_V ${typeStr})
                    endif()
                    list(APPEND TypeList "REL, RC")
                    if(${GIT_FW_VER_TYPE_V} IN_LIST TypeList)
                    else()
                        set(GIT_FW_VER_TYPE_V "DEV")
                    endif()
                    
                    if(${GIT_FW_VER_REV_V} GREATER 255)
                        message(FATAL_ERROR "Error on GIT_FW_VER_REV = ${GIT_FW_VER_REV_V}\n")
                    endif()
                    if(${GIT_FW_VER_MIN_V} GREATER 255)
                        message(FATAL_ERROR "Error on GIT_FW_VER_MIN = ${GIT_FW_VER_MIN_V}\n")
                    endif()
                    if(${GIT_FW_VER_MAJ_V} GREATER 255)
                        message(FATAL_ERROR "Error on GIT_FW_VER_MAJ = ${GIT_FW_VER_MAJ_V}\n")
                    endif()
                else()
                    set(BAD_FORMATTING TRUE)
                endif()
            endif()
            
            if(BAD_FORMATTING)
                message(WARNING 
"Found git TAG \"${output}\" has a bad formatting.\n \
-> Expected is \"TYP-MAJ.MIN.REV\" with :\n \
    - TYP in DEV, RC, REL\n \
    - MAJ : major from 00 to 99\n \
    - MIN : minor from 00 to 99\n \
    - REV : revision from 00 to 99"
                    )
            endif()

            if(NOT TAG_FOUND)
                set(GIT_FW_VER_REV_V 99)
                set(GIT_FW_VER_MIN_V 99)
                set(GIT_FW_VER_MAJ_V 99)
                set(GIT_FW_VER_TYPE_V UNK)
                set(ENV{GIT_TAG} "\"${GIT_FW_VER_TYPE_V}.${GIT_FW_VER_MAJ_V}.${GIT_FW_VER_MIN_V}.${GIT_FW_VER_REV_V}\" // undefined" )            
            endif()
            
            set(ENV{GIT_FW_VER_REV} ${GIT_FW_VER_REV_V})
            set(ENV{GIT_FW_VER_MIN} ${GIT_FW_VER_MIN_V})
            set(ENV{GIT_FW_VER_MAJ} ${GIT_FW_VER_MAJ_V})
            set(ENV{GIT_FW_VER_TYPE} \"${GIT_FW_VER_TYPE_V}\")
            
        endif()
        
        if(${var_name} STREQUAL "GIT_AUTHOR_NAME")
            if(GET_GIT_AUTHOR_NAME)
                RunGitCommand(show -s "--format=%an" ${object})
                if(exit_code EQUAL 0)
                    set(ENV{GIT_AUTHOR_NAME} ${output})
                endif()
            endif()
        endif()
        if(${var_name} STREQUAL "GIT_AUTHOR_EMAIL")
            if(GET_GIT_AUTHOR_EMAIL)
                RunGitCommand(show -s "--format=%ae" ${object})
                if(exit_code EQUAL 0)
                    set(ENV{GIT_AUTHOR_EMAIL} ${output})
                endif()
            endif()
        endif()
        if(${var_name} STREQUAL "GIT_COMMIT_DATE_ISO8601")
            RunGitCommand(show -s "--format=%ci" ${object})
            if(exit_code EQUAL 0)
                set(ENV{GIT_COMMIT_DATE_ISO8601} ${output})
            endif()
        endif()
        if(${var_name} STREQUAL "GIT_COMMIT_SUBJECT")
            RunGitCommand(show -s "--format=%s" ${object})
            if(exit_code EQUAL 0)
                string(REPLACE "\"" "\\\"" output "${output}")
                set(ENV{GIT_COMMIT_SUBJECT} ${output})
            endif()
        endif()
        if(${var_name} STREQUAL "GIT_COMMIT_BODY")
            RunGitCommand(show -s "--format=%b" ${object})
            if(exit_code EQUAL 0)
                if(output)
                    string(REPLACE "\"" "\\\"" output "${output}")
                    string(REPLACE "\r\n" "\\r\\n\\\r\n" safe "${output}")
                    if(safe STREQUAL output)
                        string(REPLACE "\n" "\\n\\\n" safe "${output}")
                    endif()
                else()
                    # There was no commit body - set the safe string to empty.
                    set(safe "")
                endif()
                set(ENV{GIT_COMMIT_BODY} "\"${safe}\"")
            else()
                set(ENV{GIT_COMMIT_BODY} "\"\"") # empty string.
            endif()
        endif()
    endforeach()   
endfunction()
