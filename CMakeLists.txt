cmake_minimum_required(VERSION 3.20)

project(
    GEWEnergyGateway
    VERSION 0.1.0
    DESCRIPTION "GEW Industrial Energy Monitoring Gateway"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE SOURCES
    main/*.cpp
    src/*.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
)

message(STATUS "")
message(STATUS "=======================================")
message(STATUS "GEW Energy Gateway")
message(STATUS "Version : ${PROJECT_VERSION}")
message(STATUS "=======================================")