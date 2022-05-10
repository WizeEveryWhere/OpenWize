################################################################################

set(MODULE_NAME mgr)

################################################################################
 
add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/adm_mgr.c
        src/inst_mgr.c
        src/dwn_mgr.c
        src/net_mgr.c
    )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Add dependencies
target_link_libraries(
    ${MODULE_NAME} 
    PRIVATE 
        WizeCore::proto 
        WizeCore::net 
        Samples::timeevt
        Samples::logger 
        3rd::freertos 
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST AND FALSE)
    # Set unittest headers to mock 
    set(MOCK_LIST
        ${CMAKE_SOURCE_DIR}/sources/WizeCore/mgr/include/net_mgr.h
        ${CMAKE_SOURCE_DIR}/sources/Samples/TimeEvt/include/time_evt.h
        ${CMAKE_SOURCE_DIR}/sources/Samples/Logger/include/logger.h
        #${THIRD_PARTY_PATH}/rtos/FreeRTOS-Kernel/include/task.h
        #${THIRD_PARTY_PATH}/rtos/FreeRTOS-Kernel/include/queue.h
        #${THIRD_PARTY_PATH}/rtos/FreeRTOS-Kernel/include/semphr.h
        )
    # Set unittest group runner list
    set(GRP_RUNNER_LIST
         #WizeCore_netmgr
         WizeCore_admmgr
         WizeCore_dwnmgr
         WizeCore_instmgr
        )
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    add_subdirectory(unittest)
endif()

#set(CMAKE_DEBUG_TARGET_PROPERTIES
#  INCLUDE_DIRECTORIES
  #COMPILE_DEFINITIONS
  #POSITION_INDEPENDENT_CODE
  #CONTAINER_SIZE_REQUIRED
  #LIB_VERSION
#)

#message("PROJECT_IS_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL}")
#message("PROJECT_NAME ${PROJECT_NAME}")
#message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
#message("PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR}")
#message(" ")
#message("CMAKE_PROJECT_NAME ${CMAKE_PROJECT_NAME}")
#message("CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
#message("CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}")
#message("CMAKE_PARENT_LIST_FILE ${CMAKE_PARENT_LIST_FILE}")
#message(" ${}")
