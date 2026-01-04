cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(Crow CONFIG REQUIRED)
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
    arbiterai
    loreforge
    yaml-cpp
    Crow::Crow
    unofficial::tree-sitter::tree-sitter
)

set_target_properties(cronus PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "\$ORIGIN"
)

#add_dependencies(cronus web-ui)

# ---------------------- Tests ----------------------
enable_testing()
find_package(GTest CONFIG REQUIRED)

add_executable(cronus_arbiterai_tests
    tests/arbiterAIIntegrationTest.cpp
    tests/main.cpp
)

target_include_directories(cronus_arbiterai_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(cronus_arbiterai_tests PRIVATE
    arbiterai
    yaml-cpp
    Crow::Crow
    unofficial::tree-sitter::tree-sitter
    GTest::gtest
    GTest::gmock
)

add_test(NAME ArbiterAIIntegrationTest COMMAND cronus_arbiterai_tests)

# WebServer tests
set(cronus_test_src
    ${CMAKE_CURRENT_SOURCE_DIR}/cronus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cronus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commandHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/commandHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inputParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sourceMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sourceMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/webServer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/webServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/agents/coder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/agents/coder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/agents/promptManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/agents/promptManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/diffHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/diffHandler.cpp
)

add_executable(cronus_webserver_tests
    tests/webServerTest.cpp
    tests/main.cpp
    ${cronus_test_src}
)

target_include_directories(cronus_webserver_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(cronus_webserver_tests PRIVATE
    arbiterai
    loreforge
    yaml-cpp
    Crow::Crow
    nlohmann_json::nlohmann_json
    unofficial::tree-sitter::tree-sitter
    unofficial::tree-sitter::tree-sitter-c
    unofficial::tree-sitter::tree-sitter-cpp
    unofficial::tree-sitter::tree-sitter-csharp
    unofficial::tree-sitter::tree-sitter-css
    unofficial::tree-sitter::tree-sitter-go
    unofficial::tree-sitter::tree-sitter-html
    unofficial::tree-sitter::tree-sitter-java
    unofficial::tree-sitter::tree-sitter-javascript
    unofficial::tree-sitter::tree-sitter-json
    unofficial::tree-sitter::tree-sitter-php
    unofficial::tree-sitter::tree-sitter-python
    unofficial::tree-sitter::tree-sitter-ruby
    unofficial::tree-sitter::tree-sitter-rust
    unofficial::tree-sitter::tree-sitter-typescript
    unofficial::tree-sitter::tree-sitter-toml
    unofficial::tree-sitter::tree-sitter-yaml
    GTest::gtest
    GTest::gmock
)

add_test(NAME WebServerTest COMMAND cronus_webserver_tests)
