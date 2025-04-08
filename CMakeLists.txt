# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(MyChatProject C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_DEBUG "Build with debug symbols" OFF)
option(BUILD_ARM64 "Build for ARM64 architecture" OFF)

# Configure cross-compilation for ARM64
if(BUILD_ARM64)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

    set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    set(OPENSSL_ROOT_DIR /usr/include/aarch64-linux-gnu)
    set(OPENSSL_INCLUDE_DIR /usr/include/aarch64-linux-gnu/openssl)
    set(OPENSSL_CRYPTO_LIBRARY /usr/lib/aarch64-linux-gnu/libcrypto.so)
    set(OPENSSL_SSL_LIBRARY /usr/lib/aarch64-linux-gnu/libssl.so)

    message(STATUS "Cross-compiling for ARM64 architecture")
    message(STATUS "Using compiler: ${CMAKE_C_COMPILER}")
endif()

# Tự động tìm tất cả file .c trong src/
file(GLOB_RECURSE SOURCE_FILES src/*.c)

# Required libraries
if(BUILD_ARM64)
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
else()
    find_package(OpenSSL REQUIRED)
endif()

find_package(Threads REQUIRED)

# Include directories
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${OPENSSL_INCLUDE_DIR}
)

# Find GTK3
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Tạo executable từ tất cả các file .c
add_executable(MyChatProject ${SOURCE_FILES}
        include/group.h
        src/models/group.c
        include/request_manager.h
        src/manager/request_manager.c
        include/json_utils.h
        src/utils/json_utils.c
        include/mess_form.h
        src/ui/join_group.c
        include/chat_message.h
)

# Link GTK3 và các thư viện cần thiết
target_include_directories(MyChatProject PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(MyChatProject PRIVATE ${GTK3_LIBRARIES} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
target_compile_options(MyChatProject PRIVATE ${GTK3_CFLAGS_OTHER})

# Debug configuration
if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    add_definitions(-DDEBUG)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# Build tests if requested
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
