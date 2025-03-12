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
    # Set cross-compilation environment
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    
    # Set cross-compilers
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    
    # Where to look for libraries and headers
    set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    
    # Set specific paths for ARM64 OpenSSL
    set(OPENSSL_ROOT_DIR /usr/include/aarch64-linux-gnu)
    set(OPENSSL_INCLUDE_DIR /usr/include/aarch64-linux-gnu/openssl)
    set(OPENSSL_CRYPTO_LIBRARY /usr/lib/aarch64-linux-gnu/libcrypto.so)
    set(OPENSSL_SSL_LIBRARY /usr/lib/aarch64-linux-gnu/libssl.so)
    
    # Output configuration
    message(STATUS "Cross-compiling for ARM64 architecture")
    message(STATUS "Using compiler: ${CMAKE_C_COMPILER}")
    message(STATUS "Using root path: ${CMAKE_FIND_ROOT_PATH}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
endif()

# Required libraries
if(BUILD_ARM64)
    # For ARM64, use the manually specified OpenSSL
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    message(STATUS "Using OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    # For host platform, use find_package
    find_package(OpenSSL REQUIRED)
endif()

find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    # Add this for ARM64 to find the parent OpenSSL directory
    $<$<BOOL:${BUILD_ARM64}>:/usr/include/aarch64-linux-gnu>
)

# Add subdirectory
add_subdirectory(src)

# Build tests if requested
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Debug configuration
if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    add_definitions(-DDEBUG)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()