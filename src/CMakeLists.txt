# src/CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE SOURCES "*.c")

# Create executable
add_executable(chat_app ${SOURCES}
        ../include/ui_controller.h)

# Link libraries
target_link_libraries(chat_app
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GTK3_LIBRARIES}
)

# Set output directory
set_target_properties(chat_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add architecture suffix for cross-compiled binaries
if(BUILD_ARM64)
    set_target_properties(chat_app PROPERTIES
        OUTPUT_NAME "chat_app_arm64"
    )
endif()

# Install rules
install(TARGETS chat_app
    RUNTIME DESTINATION bin
)
