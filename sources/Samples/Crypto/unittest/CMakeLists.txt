################################################################################

# Set unittest sources
file( GLOB ${DUT_MODULE}_UNITTEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" )

set(PRJ_MOCK "${CMAKE_CURRENT_SOURCE_DIR}/prj_mock.yml")

################################################################################

if(${DUT_MODULE}_UNITTEST_SOURCES )
    # add unittest library target
    add_unittest(
        DUT ${DUT_MODULE}
        SOURCES ${${DUT_MODULE}_UNITTEST_SOURCES}
        CONFIG ${PRJ_MOCK} 
        MOCKLIST ${MOCK_LIST}
        )
    # add unittest executable target
    add_utest_exec(
        DUT ${DUT_MODULE}
        GRP_RUNNER_LIST ${GRP_RUNNER_LIST}
        LINK_DEPENDS ${DUT_MODULE}_utest
        NATIVE_ONLY TRUE
        )
endif()

################################################################################
