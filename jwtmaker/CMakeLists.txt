# In order for shoveler to work, we must output a jwtmaker binary file in its binary directory
# In the project structure, this is a sub-directory of shoveler, so this steps need to
# generate "jwtmaker" in the parent directory.
add_custom_command(OUTPUT jwtmaker
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${GOLANG_CLI}" ARGS "build" "-o" "${CMAKE_CURRENT_BINARY_DIR}/jwtmaker"
    MAIN_DEPENDENCY "main.go")

add_custom_target(make_jwtmaker ALL 
    DEPENDS jwtmaker)

add_custom_command(TARGET make_jwtmaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy jwtmaker ${CMAKE_CURRENT_BINARY_DIR}/../workers/client
)

add_custom_command(TARGET make_jwtmaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy jwtmaker ${CMAKE_CURRENT_BINARY_DIR}/../workers/server
)

add_custom_command(TARGET make_jwtmaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy jwtmaker ${CMAKE_CURRENT_BINARY_DIR}/../workers/updater
)

add_custom_command(TARGET make_jwtmaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/private_key.pem ${CMAKE_CURRENT_BINARY_DIR}/../workers/client
)

add_custom_command(TARGET make_jwtmaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/private_key.pem ${CMAKE_CURRENT_BINARY_DIR}/../workers/server
)

add_custom_command(TARGET make_jwtmaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/private_key.pem ${CMAKE_CURRENT_BINARY_DIR}/../workers/updater
)

# If installation is required, we'll have to copy the jwtmaker file besides shoveler binary,
# which is installed in ${CMAKE_INSTALL_PREFIX}/bin because of the "RUNTIME DESTINATION bin"
# option in its installation command. If "RUNTIME DESTINATION" is changed in shoveler,
# this must be manually updated.

if(SHOVELER_INSTALL)
    install(PROGRAM "${CMAKE_CURRENT_BINARY_DIR}/jwtmaker"
        DESTINATION bin)
endif()