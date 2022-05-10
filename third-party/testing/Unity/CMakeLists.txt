################################################################################

set(MODULE_NAME unity)

cmake_minimum_required( VERSION 3.12 )

################################################################################

# Set unity and cmock path
set( ${MODULE_NAME}_PATH "CMock")
set( CMOCK_PATH "${${MODULE_NAME}_PATH}")
set( UNITY_PATH "${${MODULE_NAME}_PATH}/vendor/unity")


# Add library
add_library(${MODULE_NAME} STATIC)

 # Add sources to Build
target_sources(${MODULE_NAME}
    PRIVATE
        ${UNITY_PATH}/src/unity.c
        ${UNITY_PATH}/extras/fixture/src/unity_fixture.c
        ${UNITY_PATH}/extras/memory/src/unity_memory.c
        ${CMOCK_PATH}/src/cmock.c
    )

# Add include dir
target_include_directories(
    ${MODULE_NAME} 
    PUBLIC
         ${UNITY_PATH}/src
         ${UNITY_PATH}/extras/fixture/src
         ${UNITY_PATH}/extras/memory/src
         ${CMOCK_PATH}/src
    )

# Add alias
add_library(3rd::${MODULE_NAME} ALIAS ${MODULE_NAME})

############################################################################
