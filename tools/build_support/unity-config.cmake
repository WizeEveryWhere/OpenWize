set(CMOCK_CMD ${CMAKE_SOURCE_DIR}/third-party/testing/Unity/CMock/lib/cmock.rb )

################################################################################
# Add mocks unittest library
# In :
#   NAME     : mock library target name
#   CONFIG   : CMock project yml file
#   MOCKLIST : the list of file to mock (as path/to/mock/mock_file.h)
# Out : mock library target as given by "NAME"
#   
function(_add_mocks)
    set(options "")
    set(oneValueArgs CONFIG NAME)
    set(multiValueArgs MOCKLIST)
    cmake_parse_arguments(ADD_MOCKS 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ADD_MOCKS_MOCKLIST )# STREQUAL "")
        message("   -> mock list is empty...return.")
        return()
    endif()

    if(NOT ADD_MOCKS_CONFIG ) #OR ${ADD_MOCKS_CONFIG} STREQUAL "")
        message(FATAL_ERROR "   -> mock project yml file is required")
    endif()
    
    if(NOT ADD_MOCKS_NAME )
        set(TEST_NAME_MOCK ${TEST_NAME}_mock)
    else()
        set(TEST_NAME_MOCK ${ADD_MOCKS_NAME})
    endif() 
    
    #message("MOCKS_CONFIG : ${ADD_MOCKS_CONFIG}")
    #message("MOCKS_LIST : ${ADD_MOCKS_MOCKLIST}")

    add_library(${TEST_NAME_MOCK} STATIC )
    foreach(MOCK ${ADD_MOCKS_MOCKLIST})
        get_filename_component(MOCK_FILE ${MOCK} NAME_WE )
        get_filename_component(MOCK_DIR ${MOCK} PATH )
        add_custom_command (
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mocks/mock_${MOCK_FILE}.c
            COMMAND ruby ${CMOCK_CMD}
            -o${ADD_MOCKS_CONFIG} ${MOCK}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )
        target_sources(
            ${TEST_NAME_MOCK} 
            PUBLIC 
                ${CMAKE_CURRENT_BINARY_DIR}/mocks/mock_${MOCK_FILE}.c
            )
        target_include_directories(
            ${TEST_NAME_MOCK} 
            PRIVATE 
                ${CMAKE_BINARY_DIR} 
            PUBLIC 
                ${MOCK_DIR}
                ${CMAKE_CURRENT_BINARY_DIR}/mocks
            )
    endforeach()
    # add unity include
    get_target_property(UNITY_INCLUDES unity INCLUDE_DIRECTORIES)
    target_include_directories(${TEST_NAME_MOCK} PUBLIC ${UNITY_INCLUDES})
    
endfunction(_add_mocks)

################################################################################
# Add unittest library
# In :
#   DUT      : target name of module to test
#   NAME     : (optional) unit-test library target name
#   MOCKLIST : (optional) the list of file to mock (as path/to/mock/mock_file.h)
#   CONFIG   : (optional if MOCKLIST is empty) CMock project yml file
# Out : 
#   - unit-test target (static library) named as given by "NAME" (or DUT_utest)
#   - mock-test target (static library) named NAME_utest_mock (or DUT_utest_mock)
#   
function(add_unittest)
    set(options "")
    set(oneValueArgs NAME DUT CONFIG)
    set(multiValueArgs SOURCES MOCKLIST)
    cmake_parse_arguments(ADD_UNITTEST 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ADD_UNITTEST_DUT )
        message(FATAL_ERROR "   -> a device under test (DUT) is required")
    endif() 

    if(NOT ADD_UNITTEST_SOURCES )
        message(FATAL_ERROR "   -> unittest sources are required")
    endif()

    if(NOT ADD_UNITTEST_NAME )
        set(ADD_UNITTEST_NAME ${ADD_UNITTEST_DUT}_utest)
    endif() 

    #message("\nxxx ADD_UNITTEST_DUT      : ${ADD_UNITTEST_DUT}")
    #message("xxx ADD_UNITTEST_SOURCES  : ${ADD_UNITTEST_SOURCES}")
    #message("xxx ADD_UNITTEST_MOCKLIST : ${ADD_UNITTEST_MOCKLIST}")
    
    if (ADD_UNITTEST_MOCKLIST)
        _add_mocks(
            NAME ${ADD_UNITTEST_NAME}_mock
            CONFIG ${ADD_UNITTEST_CONFIG} 
            MOCKLIST ${ADD_UNITTEST_MOCKLIST}
            )
    endif()

    add_library(${ADD_UNITTEST_NAME} STATIC ${ADD_UNITTEST_SOURCES} )
    target_link_libraries(${ADD_UNITTEST_NAME} PUBLIC unity)

    if(TARGET ${ADD_UNITTEST_NAME}_mock)
        target_link_libraries(${ADD_UNITTEST_NAME} PRIVATE ${ADD_UNITTEST_NAME}_mock)
    endif()    

    #target_link_libraries(${ADD_UNITTEST_NAME} PUBLIC ${ADD_UNITTEST_DUT})
    target_link_libraries(${ADD_UNITTEST_NAME} PRIVATE ${ADD_UNITTEST_DUT})
    #target_link_libraries(${ADD_UNITTEST_NAME} INTERFACE ${ADD_UNITTEST_DUT})
    #target_link_libraries(${ADD_UNITTEST_NAME} INTERFACE_LINK_LIBRARIES ${ADD_UNITTEST_DUT})    


    get_target_property(DUT_INCLUDES ${ADD_UNITTEST_DUT} INCLUDE_DIRECTORIES)
    get_target_property(DUT_LINKS ${ADD_UNITTEST_DUT} LINK_LIBRARIES)

    #message("xxx ${ADD_UNITTEST_NAME} : ")
    #message("     --- DUT_LINKS : ${DUT_LINKS}")
    #message("     --- DUT_INCLUDES : ${DUT_INCLUDES}")
    if(DUT_LINKS)
        list(LENGTH DUT_LINKS DUT_LINKS_LEN)
        #message("xxx DUT_LINKS LEN : ${DUT_LINKS_LEN}")
        
        if(DUT_LINKS)
            foreach(DUT_LINKS_EL ${DUT_LINKS})
                get_target_property(DUT_LINKS_INCLUDES ${DUT_LINKS_EL} INCLUDE_DIRECTORIES)
                #message("     --- ${DUT_LINKS_EL} => DUT_LINKS_INCLUDES : ${DUT_LINKS_INCLUDES}")
            endforeach()
            #target_link_libraries(${ADD_UNITTEST_NAME} PUBLIC ${DUT_LINKS})
            #target_link_libraries(${ADD_UNITTEST_NAME} PRIVATE ${DUT_LINKS})
            target_link_libraries(${ADD_UNITTEST_NAME} INTERFACE ${DUT_LINKS})
            #target_link_libraries(${ADD_UNITTEST_NAME} INTERFACE_LINK_LIBRARIES ${DUT_LINKS})
        endif()
    endif()

    target_include_directories(
        ${ADD_UNITTEST_NAME} PUBLIC
        ${DUT_INCLUDES} 
        ${UNITY_INCLUDES}
        )
    
endfunction(add_unittest)

################################################################################
# Generate run_all.c file
# In :
#   GRP_RUNNER_LIST : a list of group runner function name to add in run_all.c
#   DESTINATION     : if given, the run_all.c destination. Otherwise, destination is CMAKE_CURRENT_BINARY_DIR
# Out : run_all.c file
#   
function(_generate_run_all)
    set(options "")
    set(oneValueArgs DESTINATION)
    set(multiValueArgs GRP_RUNNER_LIST)
    cmake_parse_arguments(GEN_RUNALL 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT GEN_RUNALL_GRP_RUNNER_LIST)
        message("   -> group runner list is empty...return.")
        return()
    endif()
    
    if(NOT GEN_RUNALL_DESTINATION)
        set(GEN_RUNALL_DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    
    set(RUN_TEST_GROUP_LIST)
    foreach(GRP ${GEN_RUNALL_GRP_RUNNER_LIST})
        set(RUN_TEST_GROUP_LIST 
"${RUN_TEST_GROUP_LIST}\n\
    RUN_TEST_GROUP(${GRP});\
"
            )
    endforeach(GRP ${GEN_RUNALL_GRP_RUNNER_LIST})
    
    set(RUN_ALL_IN 
"#include \"unity_fixture.h\"\n\n\
static void runAllTests(void)\n\
{\n\
    @RUN_TEST_GROUP_LIST@\n\
}\n\n\
int main(int argc, const char* argv[])\n\
{\n\
    return UnityMain(argc, argv, runAllTests)\; \n\
}\n "
    )

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run_all.in ${RUN_ALL_IN})
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/run_all.in ${GEN_RUNALL_DESTINATION}/run_all.c @ONLY)
endfunction(_generate_run_all)

################################################################################
# Add unittest executable
#
#
#
#
function(add_utest_exec )
    set(options NATIVE_ONLY)
    set(oneValueArgs NAME DUT)
    set(multiValueArgs GRP_RUNNER_LIST LINK_DEPENDS)
    cmake_parse_arguments(ADD_UTEST_EXEC 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(CMAKE_CROSSCOMPILING OR NOT ADD_UTEST_EXEC_NATIVE_ONLY)
        return()
    endif()

    if(NOT ADD_UTEST_EXEC_DUT )
        message(FATAL_ERROR "   -> a device under test (DUT) target name is required")
    endif()

    if(NOT ADD_UTEST_EXEC_GRP_RUNNER_LIST )
        message(FATAL_ERROR "   -> not empty group runner list is required")
    endif()

    if(NOT ADD_UTEST_EXEC_NAME )
        set(ADD_UTEST_EXEC_NAME ${ADD_UTEST_EXEC_DUT}_utest_exec)
    endif() 

    #message("xxx ADD_UTEST_EXEC_DUT             : ${ADD_UTEST_EXEC_DUT}")
    #message("xxx ADD_UTEST_EXEC_NAME            : ${ADD_UTEST_EXEC_NAME}")
    #message("xxx ADD_UTEST_EXEC_GRP_RUNNER_LIST : ${ADD_UTEST_EXEC_GRP_RUNNER_LIST}")

    if(TARGET ${ADD_UTEST_EXEC_DUT})
        # generate the run_all.c
        _generate_run_all(GRP_RUNNER_LIST ${ADD_UTEST_EXEC_GRP_RUNNER_LIST})
       
        # add executable target
        add_executable(${ADD_UTEST_EXEC_NAME} run_all.c)
        
        target_include_directories(
            ${ADD_UTEST_EXEC_NAME} PUBLIC 
            ${UNITY_INCLUDES} 
            )
        
        # link with unity
        target_link_libraries(${ADD_UTEST_EXEC_NAME} unity)
        
        # link with all given libs
        foreach(link IN LISTS ADD_UTEST_EXEC_LINK_DEPENDS)
            target_link_libraries(${ADD_UTEST_EXEC_NAME} ${link})
        endforeach()  
        
        # add test target 
        add_test(
            NAME test-${ADD_UTEST_EXEC_NAME}
            #COMMAND ${CMAKE_BINARY_DIR}/bin/${ADD_UTEST_EXEC_NAME}
            COMMAND ${ADD_UTEST_EXEC_NAME} -v
            #WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/result 
            )
        # install executable
        install(TARGETS ${ADD_UTEST_EXEC_NAME} 
                CONFIGURATIONS Debug 
                COMPONENT deb 
                RUNTIME DESTINATION  ${SETUP_INSTALL_PREFIX}bin 
            )
    endif()
endfunction(add_utest_exec)
