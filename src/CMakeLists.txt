# src/CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE SOURCES "*.c")

# Create executable
add_executable(chat_app ${SOURCES})

# Link libraries
target_link_libraries(chat_app 
    ${OPENSSL_LIBRARIES}
    Threads::Threads  # Use the proper CMake target for threads
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