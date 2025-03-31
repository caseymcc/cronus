cmake_minimum_required(VERSION 3.15)
project(tree-sitter LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/FetchTreeSitter.cmake)

# Fetch and build Tree-sitter core
FetchContent_Declare(
    tree-sitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.25.2
)
FetchContent_MakeAvailable(tree-sitter)

FetchContent_GetProperties(tree-sitter)
if(NOT tree-sitter_POPULATED)
    FetchContent_Populate(tree-sitter)
endif()

set(OLD_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CURRENT_SOURCE_DIR ${tree-sitter_SOURCE_DIR}/lib)
include(${tree-sitter_SOURCE_DIR}/lib/CMakeLists.txt)
set(CMAKE_CURRENT_SOURCE_DIR ${OLD_CURRENT_SOURCE_DIR})

#message(FATAL_ERROR "tree-sitter_SOURCE_DIR: ${tree-sitter_SOURCE_DIR}")

#file(GLOB TS_SRC_FILES "${tree-sitter_SOURCE_DIR}/lib/src/*.c")
#list(REMOVE_ITEM TS_SRC_FILES "${tree-sitter_SOURCE_DIR}/lib/src/lib.c")
#
#add_library(tree-sitter STATIC
#    ${TS_SRC_FILES}
#    PRIVATE $<BUILD_INTERFACE:${tree-sitter_SOURCE_DIR}/src>
#    PUBLIC $<BUILD_INTERFACE:${tree-sitter_SOURCE_DIR}/include>
#    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#)
#
#set_target_properties(
#    tree-sitter
#    PROPERTIES C_STANDARD 99
#               C_STANDARD_REQUIRED ON
#               POSITION_INDEPENDENT_CODE ON
#               SOVERSION ${TS_SOVERSION_MAJOR}.${TS_SOVERSION_MINOR}
#)
#  
#configure_file("${tree-sitter_SOURCE_DIR}/lib/tree-sitter.pc.in"
#                "${CMAKE_CURRENT_BINARY_DIR}/tree-sitter.pc" @ONLY)
#
## Install Tree-sitter core to the build directory
#install(TARGETS tree-sitter DESTINATION ${CMAKE_BINARY_DIR}/lib)
#install(DIRECTORY ${treesitter_SOURCE_DIR}/lib/include/ DESTINATION ${CMAKE_BINARY_DIR}/include/tree-sitter)

# Add desired parsers
add_tree_sitter_parser(c https://github.com/tree-sitter/tree-sitter-c.git v0.23.5)
add_tree_sitter_parser(python https://github.com/tree-sitter/tree-sitter-python.git v0.23.6)
add_tree_sitter_parser(javascript https://github.com/tree-sitter/tree-sitter-javascript.git v0.23.1)
