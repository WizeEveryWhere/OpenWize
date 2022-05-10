################################################################################
# This should be added to ... when native (as ExternalPrj) environment is request
# 
# Previous request :
#   - call init_to_native()
#   - call add_to_native(TARGET target_name_1 PATH your_target_path_1)
#   - call add_to_native(TARGET .... PATH ....)
#   - call setup_to_native()
#
if(IS_EXTERNAL_PRJ)
    find_package(preset_external_build_support)
    message("   -> NATIVE_PATH_LIST : ${NATIVE_PATH_LIST}")
    
    list(LENGTH NATIVE_PATH_LIST len1)
    math(EXPR len2 "${len1} - 1")
    
    foreach(i RANGE ${len2})
        list(GET NATIVE_TARGET_LIST ${i} t)
        list(GET NATIVE_PATH_LIST ${i} p)
        message(STATUS "add ${t} : ${p}")
        add_subdirectory(${p})
        set(TGT_NAME ${t})
        #get_target_property(type ${TGT_NAME} TYPE)
        
        find_package(postset_external_build_support)
    endforeach()
    
endif(IS_EXTERNAL_PRJ)
################################################################################
