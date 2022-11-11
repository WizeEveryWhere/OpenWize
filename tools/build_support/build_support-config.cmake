# include the gitinfo build support cmake
find_package(git_info REQUIRED)

# include the config_support cmake
find_package(config_support REQUIRED)

# include unity cmake
find_package(unity REQUIRED)

# include install_support cmake
find_package(install_support REQUIRED)

################################################################################
# Display the list of all targets
function (_get_all_cmake_targets out_var current_dir from ncall)
	
	MATH(EXPR ncall_x "${ncall}+1")
	string(REPLACE "${from}/" "" mTmp "${current_dir}")
	#string(REPLACE "" "" mTmp "${current_dir}")

    get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)
 	
 	set(head_str "")
  	set(tab_str "")
  	set(space_str "")
  	foreach(tab RANGE 0 ${ncall} 1)
  		#string(APPEND tab_str "${ncall}")
  		string(APPEND tab_str "*")
  		string(APPEND space_str " ") 		
	endforeach(tab)

	#if(ncall EQUAL 0 )
	#	message("${tab_str} ${mTmp} : ")
	#endif()
	
	message("${tab_str} ${mTmp} : ")	
	foreach(tgt ${targets})
		get_target_property(tgt_type ${tgt} TYPE)
			
		if(NOT ( ("${tgt_type}" STREQUAL "UTILITY") OR ("${tgt_type}" STREQUAL "INTERFACE_LIBRARY") ) )
			get_target_property(tgt_inc ${tgt} INCLUDE_DIRECTORIES)
			get_target_property(tgt_lnk ${tgt} LINK_LIBRARIES)
		endif()
		
		message("${space_str} --> ${tgt}")
		message("${space_str}  - type     : ${tgt_type}")
		if(tgt_inc)
			message("${space_str}  - includes : ${tgt_inc}")
		endif()		
		if(tgt_lnk)
			message("${space_str}  - link : ")
			foreach(l ${tgt_lnk})
				message("${space_str}    --- ${l} ")	
			endforeach()			
		endif()
	endforeach()
	
    foreach(subdir ${subdirs})
        _get_all_cmake_targets(subdir_targets ${subdir} ${current_dir} ${ncall_x})
        list(APPEND targets ${subdir_targets})
    endforeach()

    set(${out_var} ${targets} PARENT_SCOPE)
endfunction()

function (get_all_targets current_dir)
	_get_all_cmake_targets(all_targets ${current_dir} ${current_dir} 0)
endfunction()

