################################################################################

set(MODULE_NAME proto)

################################################################################
 
add_library(${MODULE_NAME} OBJECT )

# Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        src/proto.c
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
        Samples::crc_sw 
        Samples::crypto
        Samples::reedsolomon
    )

# Add alias
add_library(WizeCore::${MODULE_NAME} ALIAS ${MODULE_NAME})

# Add unit-test(s), if any
if(BUILD_TEST)
    # Set unittest headers to mock 
    set(MOCK_LIST 
        ${CMAKE_SOURCE_DIR}/sources/Samples/CRC_sw/include/crc_sw.h
        ${CMAKE_SOURCE_DIR}/sources/Samples/Crypto/include/crypto.h
        ${CMAKE_SOURCE_DIR}/sources/Samples/ReedSolomon/include/rs.h
    )
    # Set unittest group runner list
    set(GRP_RUNNER_LIST WizeCore_proto)
    # set the DUT module
    set(DUT_MODULE ${MODULE_NAME})
    add_subdirectory(unittest)
endif()

# uTests - mock

# - proto
#   - crc_sw
#   - crypto
#   - reedsolomon

# - net
#   - proto
#   - phy

# - mgr - net
#   - net
#   - rtos
#   - timeevt
#   - logger
# - mgr - inst
#   - net_mgr
#   - timeevt
#   - rtos
#   - logger
# - mgr - adm
#   - net_mgr
#   - timeevt
#   - rtos
#   - logger
# - mgr - down
#   - net_mgr
#   - timeevt
#   - rtos
#   - logger

# - app - time_mgr
#   - rtos
#   - logger
#   - timeevt
#   - parameters
# - app - ses_dispatcher
#   - rtos
#   - logger
#   - mgr : net, inst, adm, dwn
#   - parameters
#   - timeevt
#   - crypto
#   - imgstorage



