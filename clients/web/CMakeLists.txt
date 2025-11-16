# Add custom target for the web UI
find_program(NPM_EXECUTABLE npm)
if(NPM_EXECUTABLE)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/web)

    add_custom_target(web-ui
        COMMAND ${NPM_EXECUTABLE} install
        COMMAND ${NPM_EXECUTABLE} run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building web UI using npm"
    )
    
#    # Copy the built web UI to the output directory
#    add_custom_command(TARGET web-ui POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_CURRENT_SOURCE_DIR}/../build/web
#        ${CMAKE_BINARY_DIR}/web
#        COMMENT "Copying web UI to output directory"
#    )
else()
    message(WARNING "npm not found. Web UI will not be built automatically.")
endif()