cmake_minimum_required(VERSION 3.21)

if(NOT WIN32)
    message(FATAL_ERROR "Unsupported platform")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

project(msdis_access VERSION 1.0.0 LANGUAGES C)

add_library(msdis_access INTERFACE)

target_include_directories(msdis_access INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

option(MSDIS_ACCESS_BUILD_SAMPLES "Build samples" ON)

if(MSDIS_ACCESS_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()