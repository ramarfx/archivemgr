cmake_minimum_required(VERSION 3.16)
project(archivemgr_simple VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEPLOY_BUILD "Build for deployment (single executable, no external libzip required)" OFF)

if(DEPLOY_BUILD)
    include(FetchContent)
    
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    # 1. Fetch zlib
    FetchContent_Declare(
        zlib
        URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz
    )
    FetchContent_MakeAvailable(zlib)
    
    # Bypass find_package(ZLIB) in libzip
    if(WIN32)
        set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/libzlibstatic.a" CACHE FILEPATH "" FORCE)
    else()
        set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/libz.a" CACHE FILEPATH "" FORCE)
    endif()
    set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE PATH "" FORCE)
    set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)

    # 2. Fetch libzip
    set(ENABLE_GNUTLS OFF CACHE BOOL "" FORCE)
    set(ENABLE_MBEDTLS OFF CACHE BOOL "" FORCE)
    set(ENABLE_OPENSSL OFF CACHE BOOL "" FORCE)
    set(ENABLE_BZIP2 OFF CACHE BOOL "" FORCE)
    set(ENABLE_LZMA OFF CACHE BOOL "" FORCE)
    set(ENABLE_ZSTD OFF CACHE BOOL "" FORCE)
    set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(BUILD_REGRESS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_DOC OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        libzip
        URL https://github.com/nih-at/libzip/archive/refs/tags/v1.10.1.tar.gz
    )
    FetchContent_MakeAvailable(libzip)
    
    add_executable(archivemgr src/main.cpp src/archiver.cpp)
    
    if(WIN32)
        target_link_libraries(archivemgr zip ws2_32)
    else()
        target_link_libraries(archivemgr zip stdc++fs)
    endif()
    
    message(STATUS "Deployment build enabled: libzip and zlib compiled statically from source")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZIP REQUIRED libzip)
    
    include_directories(${ZIP_INCLUDE_DIRS})
    link_directories(${ZIP_LIBRARY_DIRS})
    
    add_executable(archivemgr src/main.cpp src/archiver.cpp)
    target_link_libraries(archivemgr ${ZIP_LIBRARIES} stdc++fs)
    
    message(STATUS "Test build enabled: standard dynamic linking")
endif()
