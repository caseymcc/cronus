cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(unofficial-tree-sitter CONFIG REQUIRED)

#include(${CMAKE_CURRENT_LIST_DIR}/../external/tree-sitter/CMakeLists.txt)

set(cronus_src
    ./cronus.h
    ./cronus.cpp
    ./commandHandler.h
    ./commandHandler.cpp
    ./config.h
    ./config.cpp
    ./inputParser.h
    ./inputParser.cpp
    ./logger.h
    ./logger.cpp
    ./main.cpp
    ./model.h
    ./model.cpp
    ./restApi.h
    ./restApi.cpp
    ./sourceMap.h
    ./sourceMap.cpp
    ./webServer.h
    ./webServer.cpp
    agents/coder.h
    agents/coder.cpp
    agents/promptManager.h
    agents/promptManager.cpp
    utils/diffHandler.h
    utils/diffHandler.cpp
)

# Add executable
add_executable(cronus
    ${cronus_src}
)

target_include_directories(cronus 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(cronus PRIVATE
    arbiterAI
    loreforge
    yaml-cpp
    httplib::httplib
    unofficial::tree-sitter::tree-sitter
)

set_target_properties(cronus PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "\$ORIGIN"
)

#add_dependencies(cronus web-ui)
