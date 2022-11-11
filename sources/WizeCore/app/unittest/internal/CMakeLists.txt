set(DUT_NAME ${DUT_MODULE}_internal)

################################################################################
get_target_property(SAMPLES_CRYPTO_SRC_PATH Samples::crypto SOURCE_DIR)
get_target_property(SAMPLES_PARAMETERS_SRC_PATH Samples::parameters SOURCE_DIR)

# Set unittest headers to mock 
set(MOCK_LIST 
   	${SAMPLES_CRYPTO_SRC_PATH}/include/crypto.h
   	${SAMPLES_PARAMETERS_SRC_PATH}/include/parameters.h
)

# Set unittest group runner list
set(GRP_RUNNER_LIST
   	WizeCoreAppInt_inst
   	WizeCoreAppInt_adm
)

# Set unittest sources
set( ${DUT_NAME}_UNITTEST_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/TestWizeCoreAppIntInst.c" 
    "${CMAKE_CURRENT_SOURCE_DIR}/TestWizeCoreAppIntAdm.c" 
    "${CMAKE_CURRENT_SOURCE_DIR}/TestGrpRunWizeCoreAppInt.c"
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
        LINK_DEPENDS ${DUT_NAME}_utest
        NATIVE_ONLY TRUE
    )

	get_target_property(WIZECORE_NET_INC_PATH WizeCore::net INCLUDE_DIRECTORIES)
	get_target_property(WIZECORE_PROTO_INC_PATH WizeCore::proto INCLUDE_DIRECTORIES)
    
    target_include_directories(${DUT_NAME}_utest_mock 
    	PRIVATE 
    		${WIZECORE_NET_INC_PATH} 
    		${WIZECORE_PROTO_INC_PATH}
    )
	    
	target_include_directories(${DUT_NAME}_utest 
		PRIVATE 
			${WIZECORE_NET_INC_PATH} 
    		${WIZECORE_PROTO_INC_PATH}
	)
endif()

################################################################################
