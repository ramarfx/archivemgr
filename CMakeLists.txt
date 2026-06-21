cmake_minimum_required(VERSION 3.16)
project(archivemgr_simple VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libzip
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZIP REQUIRED libzip)

include_directories(${ZIP_INCLUDE_DIRS})
link_directories(${ZIP_LIBRARY_DIRS})

add_executable(archivemgr src/main.cpp src/archiver.cpp)
target_link_libraries(archivemgr ${ZIP_LIBRARIES} stdc++fs)
