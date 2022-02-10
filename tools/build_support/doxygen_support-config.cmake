
option(BUILD_DOC "Build documentation" ON)

# Build documentation

find_package(Doxygen)
if(DOXYGEN_FOUND)
    # Try to find a Doxyfile.in 
    find_file(DOXYFILE_IN "Doxyfile.in"
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}" "./" "docs" "tools/build_support"
        NO_DEFAULT_PATH
        DOC "Path to the doxygen configuration template file"
        REQUIRED)
    #set(DOXYFILE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
  
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(DOXYFILE_IN DEFAULT_MSG "DOXYFILE_IN")

    if(DOXYFILE_IN_FOUND)
        set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)

        # request to configure the file
        configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
        message("Doxygen build started")
    
    
        set(DOXYFILE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Doxygen" CACHE PATH "Doxygen output directory")
        set(DOXYFILE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/doc" CACHE PATH "Doxygen documentation installation directory")
        if(NOT DOXYFILE_SOURCE_DIRS)
            set(DOXYFILE_SOURCE_DIRS "")
        endif()
        set(DOXYFILE_LATEX "YES" CACHE BOOL "Generate LaTeX API documentation")
        set(DOXYFILE_MATHJAX "YES" CACHE BOOL "Use MATHJAX to render formulas in HTML")
        set(DOXYFILE_PDFLATEX "NO" CACHE BOOL "Use pdflatex for better quality pdf files")
        set(DOXYFILE_CITE_BIB_FILES "")

        mark_as_advanced(
            DOXYFILE_OUTPUT_DIR 
            DOXYFILE_SOURCE_DIRS 
            DOXYFILE_INSTALL_DIR 
            DOXYFILE_EXTRA_SOURCE_DIRS 
            DOXYFILE_MATHJAX 
            DOXYFILE_IN 
            DOXYFILE_OUT 
            )

        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES 
            "${DOXYFILE_OUTPUT_DIR}/html"
            )

        add_custom_target(doxygen
            COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE_OUT}"
            COMMENT "Writing documentation to ${DOXYFILE_OUTPUT_DIR}..."
            #WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
            WORKING_DIRECTORY "${DOXYFILE_OUTPUT_DIR}"
            )

        add_custom_target(doxydir
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYFILE_OUTPUT_DIR}
            COMMENT "Creating doc directory"
            )

        add_dependencies(doxygen doxydir)

        set(DOXYFILE_DOT "NO")
        if(DOXYGEN_DOT_EXECUTABLE)
            set(DOXYFILE_DOT "YES")
        endif()

        set(DOXYFILE_GENERATE_LATEX "NO")
    
        ## LaTeX
        if(DOXYFILE_LATEX)
            find_package(LATEX)
            if(LATEX_FOUND)
                set_property(
                    DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES 
                    "${DOXYFILE_OUTPUT_DIR}/latex"
                    )

                set(DOXYFILE_GENERATE_LATEX "YES")
                if(PDFLATEX_COMPILER)
                    set(DOXYFILE_PDFLATEX "YES")
                endif()

                add_custom_command(
                    TARGET doxygen
                    POST_BUILD COMMAND "${CMAKE_MAKE_PROGRAM}"
                    COMMENT "Running LaTeX for Doxygen documentation in ${DOXYFILE_OUTPUT_DIR}/${DOXYFILE_LATEX_DIR}..."
                    WORKING_DIRECTORY "${DOXYFILE_OUTPUT_DIR}/latex"
                    )

                install(
                    FILES ${DOXYFILE_OUTPUT_DIR}/latex/refman.pdf 
                    DESTINATION ${DOXYFILE_INSTALL_DIR} OPTIONAL
                    )
            endif()
        endif()

        if(DOXYFILE_LATEX AND NOT DOXYFILE_GENERATE_LATEX)
            message(WARNING "Cannot generate latex documentation")
        endif()

        if(NOT TARGET doc)
            add_custom_target(doc)
        endif()

        add_dependencies(doc doxygen)

        #add_custom_command(TARGET uninstall
        #  COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYFILE_INSTALL_DIR}
        #  COMMENT "Removing Doxygen documentation")

    endif()

    install(
        DIRECTORY ${DOXYFILE_OUTPUT_DIR}/html 
        DESTINATION ${DOXYFILE_INSTALL_DIR} OPTIONAL
    )

    set(DOXYFILE_SOURCE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygenTpl")
endif()
