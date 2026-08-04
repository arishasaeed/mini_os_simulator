cmake_minimum_required(VERSION 3.16)
project(MiniOSSimulator VERSION 1.0 LANGUAGES CXX)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt automatic generation utilities
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt 6 libraries
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source and header files discovery
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
)

add_executable(MiniOSSimulator ${SOURCES})

# Link libraries
target_link_libraries(MiniOSSimulator PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
