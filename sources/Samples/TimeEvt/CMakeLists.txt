################################################################################

set(MODULE_NAME timeevt)

################################################################################

add_library(${MODULE_NAME} OBJECT )

# Add include dir    
target_include_directories(
    ${MODULE_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

# Add header only to Build
target_sources(${MODULE_NAME}
    PRIVATE
        include/time_evt.h
    )

if(USE_TIMEEVT_SAMPLE)
    # Add sources to Build
    target_sources(${MODULE_NAME}
        PRIVATE
            src/time_evt.c
        )
    # Add dependencies
    target_link_libraries(
        ${MODULE_NAME} 
        PRIVATE 
            3rd::freertos
    )

    # Add unit-test(s), if any
    if(BUILD_TEST)

		get_target_property(RTOS_SRC_PATH 3rd::freertos SOURCE_DIR)
		get_target_property(RTOS_INC_PATH 3rd::freertos INCLUDE_DIRECTORIES)
		
		#message("    RTOS_SRC_PATH = ${RTOS_SRC_PATH}")
		#message("    RTOS_INC_PATH = ${RTOS_INC_PATH}")
       
        # Set unittest headers to mock 
    	set(MOCK_LIST 
        	${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/task.h
        	${RTOS_SRC_PATH}/FreeRTOS-Kernel/include/queue.h
        	${RTOS_SRC_PATH}/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/portmacro.h
        )
        # Set unittest group runner list
        set(GRP_RUNNER_LIST Samples_TimeEvtInt Samples_TimeEvt)
        # set the DUT module
        set(DUT_MODULE ${MODULE_NAME})
        add_subdirectory(unittest)
		
		target_include_directories(${DUT_MODULE}_utest_mock PRIVATE ${RTOS_INC_PATH})
	    target_include_directories(${DUT_MODULE}_utest PRIVATE ${RTOS_INC_PATH})
    endif()
endif(USE_TIMEEVT_SAMPLE)

# Add alias
add_library(Samples::${MODULE_NAME} ALIAS ${MODULE_NAME})
