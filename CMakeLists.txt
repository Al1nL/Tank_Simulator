cmake_minimum_required(VERSION 3.15)
project(Tank_Simulator)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")  # Check if compiler is GCC (g++)
        add_compile_options(
            -Wall
            -Wextra
            -Werror
            -pedantic
        )
    endif()
endif()

# Source files
set(ALGORITHM_SRC
        Algorithm/source/Player_212535058_324022904.cpp
        Algorithm/source/TankAlgorithm_212535058_324022904.cpp
        # Add more files here
)

set(GAMEMANAGER_SRC
    # Add more files here
)

set(SIMULATOR_SRC
        Simulator/source/AlgorithmRegistrar.cpp
        Simulator/source/PlayerRegistration.cpp
        Simulator/source/TankAlgorithmRegistration.cpp
        Simulator/source/Simulator.cpp
)

set(USERCOMMON_SRC
        UserCommon/source/Mine.cpp
        UserCommon/source/Shell.cpp
        UserCommon/source/Tank.cpp
        UserCommon/source/Wall.cpp
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
    "${CMAKE_SOURCE_DIR}/Simulator/header/*.h"
    "${CMAKE_SOURCE_DIR}/Algorithm/header/*.h"
    "${CMAKE_SOURCE_DIR}/UserCommon/header/*.h"
)

## Include directories
target_include_directories(Tank_Simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/Algorithm
    ${CMAKE_SOURCE_DIR}/GameManager
    ${CMAKE_SOURCE_DIR}/Simulator
    ${CMAKE_SOURCE_DIR}/UserCommon
)
