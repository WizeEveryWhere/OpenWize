set(DUT_NAME ${DUT_MODULE}_api)

################################################################################

get_target_property(SAMPLES_PARAMETERS_SRC_PATH Samples::parameters SOURCE_DIR)
get_target_property(SAMPLES_TIMEEVT_SRC_PATH Samples::timeevt SOURCE_DIR)
get_target_property(SAMPLES_LOGGER_SRC_PATH Samples::logger SOURCE_DIR)
get_target_property(SAMPLES_CRYPTO_SRC_PATH Samples::crypto SOURCE_DIR)
get_target_property(WIZECORE_MGR_SRC_PATH WizeCore::mgr SOURCE_DIR)
get_target_property(WIZECORE_APP_SRC_PATH WizeCore::app SOURCE_DIR)
get_target_property(RTOS_SRC_PATH 3rd::freertos SOURCE_DIR)
	
# Set unittest headers to mock 
set(MOCK_LIST 
   	${SAMPLES_PARAMETERS_SRC_PATH}/include/parameters.h
   	${SAMPLES_TIMEEVT_SRC_PATH}/include/time_evt.h  
    ${SAMPLES_LOGGER_SRC_PATH}/include/logger.h 
    ${SAMPLES_CRYPTO_SRC_PATH}/include/crypto.h
    
	${WIZECORE_MGR_SRC_PATH}/include/net_mgr.h 
	${WIZECORE_MGR_SRC_PATH}/include/time_mgr.h 
	
	#${WIZECORE_APP_SRC_PATH}/include/private/ses_dispatcher.h
	${WIZECORE_APP_SRC_PATH}/include/private/wize_app.h

   	${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/task.h 
   	${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/queue.h 
   	#${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/event_groups.h 
   	${RTOS_SRC_PATH}/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/portmacro.h  
)

# Set unittest group runner list
set(GRP_RUNNER_LIST
   	WizeCoreApp_wizeapi
)

# Set unittest sources
set( ${DUT_NAME}_UNITTEST_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/TestWizeCoreAppWizeApi.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestGrpRunWizeCoreApi.c" 
    )

set(PRJ_MOCK "${CMAKE_CURRENT_SOURCE_DIR}/prj_mock.yml")

################################################################################

if(${DUT_NAME}_UNITTEST_SOURCES )
    # add unittest library target
    add_unittest(
        DUT ${DUT_MODULE}
        NAME ${DUT_NAME}_utest
        SOURCES ${${DUT_NAME}_UNITTEST_SOURCES}
        CONFIG ${PRJ_MOCK} 
        MOCKLIST ${MOCK_LIST}
    )
    # add unittest executable target
    add_utest_exec(
        DUT ${DUT_MODULE}
        NAME ${DUT_NAME}_utest_exec
        GRP_RUNNER_LIST ${GRP_RUNNER_LIST}
        LINK_DEPENDS ${DUT_NAME}_utest  WizeCore::WizeCore
        NATIVE_ONLY TRUE
    )
    
    get_target_property(SAMPLES_TIMEEVT_INC_PATH Samples::timeevt INCLUDE_DIRECTORIES)
    get_target_property(WIZECORE_APP_INC_PATH WizeCore::app INCLUDE_DIRECTORIES)
	get_target_property(WIZECORE_NET_INC_PATH WizeCore::net INCLUDE_DIRECTORIES)
	get_target_property(WIZECORE_PROTO_INC_PATH WizeCore::proto INCLUDE_DIRECTORIES)
	get_target_property(RTOS_INC_PATH 3rd::freertos INCLUDE_DIRECTORIES)
    
    target_include_directories(${DUT_NAME}_utest_mock 
    	PRIVATE 
    		${SAMPLES_TIMEEVT_INC_PATH} 
    		${WIZECORE_APP_INC_PATH}
    		${WIZECORE_NET_INC_PATH}
    		${WIZECORE_PROTO_INC_PATH}
    		${RTOS_INC_PATH} 
    )
    
	target_include_directories(${DUT_NAME}_utest 
		PRIVATE 
			${SAMPLES_TIMEEVT_INC_PATH} 
			${WIZECORE_APP_INC_PATH}
			${WIZECORE_NET_INC_PATH}
			${WIZECORE_PROTO_INC_PATH}
			${RTOS_INC_PATH} 
	)
endif()

################################################################################
