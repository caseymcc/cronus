cmake_minimum_required(VERSION 3.15)
project(cronus VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

# Include FetchContent module for external dependencies
include(FetchContent)

# Fetch arbiterAI library
FetchContent_Declare(
    arbiterAI
    GIT_REPOSITORY https://github.com/caseymcc/arbiterAI
    GIT_TAG main
)

# Fetch loreforge library (which also depends on arbiterAI)
FetchContent_Declare(
    loreforge
    GIT_REPOSITORY https://github.com/caseymcc/loreforge
    GIT_TAG main
)

# Make the content available
FetchContent_MakeAvailable(arbiterAI loreforge)

# Add the cronus server
add_subdirectory(server/cronus)
add_subdirectory(server/arbiterAI)
add_subdirectory(server/loreforge)
#add_subdirectory(web)

