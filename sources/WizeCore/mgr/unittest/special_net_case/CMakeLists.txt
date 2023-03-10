set(DUT_NAME ${DUT_MODULE})

################################################################################

get_target_property(SAMPLES_TIMEEVT_SRC_PATH Samples::timeevt SOURCE_DIR)
get_target_property(SAMPLES_LOGGER_SRC_PATH Samples::logger SOURCE_DIR)

get_target_property(WIZECORE_MGR_SRC_PATH WizeCore::mgr SOURCE_DIR)
get_target_property(WIZECORE_NET_SRC_PATH WizeCore::net SOURCE_DIR)

get_target_property(RTOS_SRC_PATH 3rd::freertos SOURCE_DIR)

################################################################################


# Set unittest headers to mock 
set(MOCK_LIST
    ${WIZECORE_NET_SRC_PATH}/include/net_api_private.h 
    ${SAMPLES_TIMEEVT_SRC_PATH}/include/time_evt.h 
    ${SAMPLES_LOGGER_SRC_PATH}/include/logger.h 
    
    ${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/task.h 
   	${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/queue.h
   	 
   	${RTOS_SRC_PATH}/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/portmacro.h  
)

# Set unittest group runner list
set(GRP_RUNNER_LIST
    WizeCoreMgr_net
)

# Set unittest sources
set( ${DUT_NAME}_UNITTEST_SOURCES 
	"${CMAKE_CURRENT_SOURCE_DIR}/TestWizeCoreMgrNet.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestGrpRunWizeCoreMgr_Net.c"
    )

set(PRJ_MOCK "${CMAKE_CURRENT_SOURCE_DIR}/prj_mock.yml")

################################################################################

if(${DUT_NAME}_UNITTEST_SOURCES )
    # add unittest library target
    add_unittest(
        DUT ${DUT_MODULE}
        NAME ${DUT_NAME}_net_utest
        SOURCES ${${DUT_NAME}_UNITTEST_SOURCES}
        CONFIG ${PRJ_MOCK} 
        MOCKLIST ${MOCK_LIST}
    )
    # add unittest executable target
    add_utest_exec(
        DUT ${DUT_MODULE}
        NAME ${DUT_NAME}_net_utest_exec
        GRP_RUNNER_LIST ${GRP_RUNNER_LIST}
        LINK_DEPENDS ${DUT_NAME}_net_utest
        NATIVE_ONLY TRUE
    )   
    
    get_target_property(WIZECORE_PROTO_INC_PATH WizeCore::proto INCLUDE_DIRECTORIES)    
    get_target_property(RTOS_INC_PATH 3rd::freertos INCLUDE_DIRECTORIES)
    
    target_include_directories(${DUT_NAME}_net_utest_mock 
    	PRIVATE 
    		${WIZECORE_PROTO_INC_PATH}
    		${RTOS_INC_PATH} 
    )
    
	target_include_directories(${DUT_NAME}_net_utest 
		PRIVATE 
			${WIZECORE_PROTO_INC_PATH}
    		${RTOS_INC_PATH} 
	)
    
    #[[
	get_target_property(WIZECORE_NET_INC_PATH WizeCore::net INCLUDE_DIRECTORIES)
	get_target_property(WIZECORE_PROTO_INC_PATH WizeCore::proto INCLUDE_DIRECTORIES)    
    
    target_include_directories(${DUT_NAME}_utest_mock 
    	PRIVATE 
    		${WIZECORE_NET_INC_PATH} 
    		#${WIZECORE_PROTO_INC_PATH}
    )
    
	target_include_directories(${DUT_NAME}_utest 
		PRIVATE 
    		${WIZECORE_NET_INC_PATH} 
    		${WIZECORE_PROTO_INC_PATH}
	)
	#]]
endif()
