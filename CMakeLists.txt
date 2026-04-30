cmake_minimum_required(VERSION 3.16)

project(same LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_BACKEND "Build backend core library" ON)
option(BUILD_FRONTEND "Build frontend application" ON)

find_package(Crow REQUIRED)

# Add backend subdirectory
if(BUILD_BACKEND)
    add_subdirectory(backend)
endif()

# Add frontend subdirectory
if(BUILD_FRONTEND)
    add_subdirectory(frontend)
endif()