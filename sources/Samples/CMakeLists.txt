################################################################################

set(MODULE_NAME samples)

################################################################################
set(SUB_DIR_LIST CRC_sw Crypto ReedSolomon Parameters ImgStorage TimeEvt Logger)
foreach(SUB_DIR ${SUB_DIR_LIST})
    string(TOLOWER ${SUB_DIR} lib_name)
    add_subdirectory(${SUB_DIR})
    list(APPEND target_obj_list $<TARGET_OBJECTS:${lib_name}>)
    list(APPEND target_dep_list ${lib_name})
    
    add_library(Samples::${lib_name} ALIAS ${lib_name})
    
endforeach(SUB_DIR ${SUB_DIR_LIST})

################################################################################

add_library( ${MODULE_NAME} STATIC ${target_obj_list} )
target_link_libraries(${MODULE_NAME} PUBLIC ${target_dep_list})

add_library(Samples::Samples ALIAS ${MODULE_NAME})

################################################################################
