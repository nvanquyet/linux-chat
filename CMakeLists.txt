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

    # Add GTK configuration for ARM64
    set(GTK3_INCLUDE_DIRS 
        /usr/include/aarch64-linux-gnu/gtk-3.0
        /usr/include/aarch64-linux-gnu/glib-2.0
        /usr/lib/aarch64-linux-gnu/glib-2.0/include
        /usr/include/aarch64-linux-gnu/pango-1.0
        /usr/include/aarch64-linux-gnu/cairo
        /usr/include/aarch64-linux-gnu/gdk-pixbuf-2.0
        /usr/include/aarch64-linux-gnu/atk-1.0
        /usr/include/gtk-3.0
        /usr/include/pango-1.0
        /usr/include/cairo
        /usr/include/glib-2.0
        /usr/include/harfbuzz
        /usr/include/gdk-pixbuf-2.0
        /usr/include/atk-1.0)

    set(GTK3_LIBRARIES 
        /usr/lib/aarch64-linux-gnu/libgtk-3.so
        /usr/lib/aarch64-linux-gnu/libgdk-3.so
        /usr/lib/aarch64-linux-gnu/libpangocairo-1.0.so
        /usr/lib/aarch64-linux-gnu/libpango-1.0.so
        /usr/lib/aarch64-linux-gnu/libatk-1.0.so
        /usr/lib/aarch64-linux-gnu/libcairo-gobject.so
        /usr/lib/aarch64-linux-gnu/libcairo.so
        /usr/lib/aarch64-linux-gnu/libgdk_pixbuf-2.0.so
        /usr/lib/aarch64-linux-gnu/libgio-2.0.so
        /usr/lib/aarch64-linux-gnu/libgobject-2.0.so
        /usr/lib/aarch64-linux-gnu/libglib-2.0.so)
    
    # Output configuration
    message(STATUS "Cross-compiling for ARM64 architecture")
    message(STATUS "Using compiler: ${CMAKE_C_COMPILER}")
    message(STATUS "Using root path: ${CMAKE_FIND_ROOT_PATH}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    
    # Required libraries for ARM64
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    message(STATUS "Using OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    # For host platform (x64), use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    
    # Find OpenSSL for host platform
    find_package(OpenSSL REQUIRED)
    
    message(STATUS "Building for host architecture (x64)")
    message(STATUS "GTK3 Include Dirs: ${GTK3_INCLUDE_DIRS}")
    message(STATUS "GTK3 Libraries: ${GTK3_LIBRARIES}")
endif()

# Find threading library for both architectures
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    $<$<BOOL:${BUILD_ARM64}>:/usr/include/aarch64-linux-gnu>
    ${GTK3_INCLUDE_DIRS}
)

# Build debug configuration 
if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    add_definitions(-DDEBUG)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add subdirectory
add_subdirectory(src)

# Build tests if requested
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()