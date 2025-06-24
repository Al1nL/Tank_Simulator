cmake_minimum_required(VERSION 3.30)
project(Tank_Simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/Algorithm
    ${CMAKE_SOURCE_DIR}/GameManager
    ${CMAKE_SOURCE_DIR}/Simulator
    ${CMAKE_SOURCE_DIR}/UserCommon
)
# Source files
file(GLOB_RECURSE ALGORITHM_SRC ${CMAKE_SOURCE_DIR}/Algorithm/*.cpp)
file(GLOB_RECURSE GAMEMANAGER_SRC ${CMAKE_SOURCE_DIR}/GameManager/*.cpp)
file(GLOB_RECURSE SIMULATOR_SRC ${CMAKE_SOURCE_DIR}/Simulator/*.cpp)
file(GLOB_RECURSE USERCOMMON_SRC ${CMAKE_SOURCE_DIR}/UserCommon/*.cpp)

# Add executable
#add_executable(Tank_Simulator
#    ${COMMON_SRC}
#    ${ALGORITHM_SRC}
#    ${GAMEMANAGER_SRC}
#    ${SIMULATOR_SRC}
#    ${USERCOMMON_SRC}
#)
