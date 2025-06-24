cmake_minimum_required(VERSION 3.15)
project(Tank_Simulator)

set(CMAKE_CXX_STANDARD 20)

# Source files
set(ALGORITHM_SRC
    # Add more files here
)

set(GAMEMANAGER_SRC
    # Add more files here
)

set(SIMULATOR_SRC
    # Add more files here
)

set(USERCOMMON_SRC
    # Add more files here
)


# Combine all source files
set(SOURCES
    ${ALGORITHM_SRC}
    ${GAMEMANAGER_SRC}
    ${SIMULATOR_SRC}
    ${USERCOMMON_SRC}
)

# Add executable
add_executable(Tank_Simulator ${SOURCES} main.cpp)

file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/common/*.h"
)

## Include directories
target_include_directories(Tank_Simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/Algorithm
    ${CMAKE_SOURCE_DIR}/GameManager
    ${CMAKE_SOURCE_DIR}/Simulator
    ${CMAKE_SOURCE_DIR}/UserCommon
)
