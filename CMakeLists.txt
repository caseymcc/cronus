cmake_minimum_required(VERSION 3.15)
project(cronus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

# Set PIC globally for all targets (required for linking static libs into shared libs)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include FetchContent module for external dependencies
include(FetchContent)

# Disable automatic updates of fetched content if already present
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Check if arbiterAI already exists, if so add it directly
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/server/arbiterAI/CMakeLists.txt)
    message(STATUS "Using existing arbiterAI from server/arbiterAI")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/server/arbiterAI)
else()
    # Fetch arbiterAI library
    FetchContent_Declare(
        arbiterAI
        GIT_REPOSITORY https://github.com/caseymcc/arbiterAI
        GIT_TAG main
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/arbiterAI
    )
    FetchContent_MakeAvailable(arbiterAI)
endif()

# Check if loreforge already exists, if so add it directly
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/server/loreforge/CMakeLists.txt)
    message(STATUS "Using existing loreforge from server/loreforge")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/server/loreforge)
else()
    # Fetch loreforge library (which also depends on arbiterAI)
    FetchContent_Declare(
        loreforge
        GIT_REPOSITORY https://github.com/caseymcc/loreforge
        GIT_TAG main
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/loreforge
    )
    FetchContent_MakeAvailable(loreforge)
endif()

# Add the cronus server
add_subdirectory(server/cronus)
#add_subdirectory(web)

