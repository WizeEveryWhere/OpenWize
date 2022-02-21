################################################################################
# Setup install target  
# In : 
#   - TARGET    : install target name 
#   - NAMESPACE : add an alias with the given NAMESPACE
#   - PREFIX    : prefix all installation directories with PREFIX
#   - DEPENDS   : add dependencies to this install target
# Out : 
#   - the required install target
#  
function(setup_install)
    set(options "")
    set(oneValueArgs TARGET NAMESPACE PREFIX)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(SETUP_INSTALL "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    if(SETUP_INSTALL_NAMESPACE)
        add_library(${SETUP_INSTALL_NAMESPACE}::${SETUP_INSTALL_TARGET} ALIAS ${SETUP_INSTALL_TARGET})
    endif()
    
    if(SETUP_INSTALL_DEPENDS)
        target_link_libraries(${SETUP_INSTALL_TARGET} PRIVATE ${SETUP_INSTALL_DEPENDS})
        #add_dependencies(${SETUP_INSTALL_TARGET} ${SETUP_INSTALL_DEPENDS})
    endif()

    get_target_property(target_type ${SETUP_INSTALL_TARGET} TYPE)
    if (target_type STREQUAL "EXECUTABLE")
        set(CMAKE_EXECUTABLE_SUFFIX ".elf")
        
        set_target_properties(
            ${SETUP_INSTALL_TARGET} 
            PROPERTIES 
                SUFFIX ".elf"
        )
        
        set(PROJECT_MAP "$<TARGET_FILE_DIR:${MODULE_NAME}>/${MODULE_NAME}.map")
        set(PROJECT_BIN "$<TARGET_FILE_DIR:${MODULE_NAME}>/${MODULE_NAME}.bin")
        add_custom_command(
            TARGET ${MODULE_NAME} POST_BUILD
            COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:${MODULE_NAME}>" "${PROJECT_BIN}"
            COMMAND "${CMAKE_SIZE}" "$<TARGET_FILE:${MODULE_NAME}>"
        )
        
        set(PROJECT_LST "$<TARGET_FILE_DIR:${MODULE_NAME}>/${MODULE_NAME}.lst")
        add_custom_command(
            TARGET ${MODULE_NAME} POST_BUILD
            COMMAND "${CMAKE_OBJDUMP}" -h -S -z "$<TARGET_FILE:${MODULE_NAME}>" > "${PROJECT_LST}"
        )
        
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.bin DESTINATION bin)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.map DESTINATION bin)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.lst DESTINATION bin)
    endif ()
    
        # set target install
#     install(TARGETS ${SETUP_INSTALL_TARGET} 
#             CONFIGURATIONS Coverage 
#             COMPONENT cov 
#             ARCHIVE DESTINATION ${SETUP_INSTALL_PREFIX}lib 
#             RUNTIME DESTINATION  ${SETUP_INSTALL_PREFIX}bin 
#             PUBLIC_HEADER DESTINATION  ${SETUP_INSTALL_PREFIX}include 
#             PRIVATE_HEADER DESTINATION  ${SETUP_INSTALL_PREFIX}include/private 
#         )

    install(TARGETS ${SETUP_INSTALL_TARGET} 
            CONFIGURATIONS Debug 
            COMPONENT dbg 
            ARCHIVE DESTINATION  ${SETUP_INSTALL_PREFIX}lib 
            RUNTIME DESTINATION  ${SETUP_INSTALL_PREFIX}bin 
            PUBLIC_HEADER DESTINATION  ${SETUP_INSTALL_PREFIX}include 
            PRIVATE_HEADER DESTINATION  ${SETUP_INSTALL_PREFIX}include/private 
        )

    install(TARGETS ${SETUP_INSTALL_TARGET}
            CONFIGURATIONS Release 
            COMPONENT rel 
            ARCHIVE DESTINATION  ${SETUP_INSTALL_PREFIX}lib 
            RUNTIME DESTINATION  ${SETUP_INSTALL_PREFIX}bin 
            PUBLIC_HEADER DESTINATION  ${SETUP_INSTALL_PREFIX}include 
            PRIVATE_HEADER DESTINATION  ${SETUP_INSTALL_PREFIX}include/private
        )
endfunction(setup_install)

################################################################################
# Setup uninstall target  
# In : 
#   - TARGET    : un-install target name 
# Out : 
#   - the required uninstall target
# 
function(setup_uninstall)
    set(options "")
    set(oneValueArgs TARGET )
    set(multiValueArgs "")
    cmake_parse_arguments(SETUP_UNINSTALL "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    if(NOT TARGET ${SETUP_UNINSTALL_TARGET}-uninstall)
        configure_file(
            "${TOOL_BUILD_SUPPORT}/cmake_uninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
    
        add_custom_target(${SETUP_UNINSTALL_TARGET}-uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    endif()
endfunction(setup_uninstall)
