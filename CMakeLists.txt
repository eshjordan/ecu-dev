cmake_minimum_required(VERSION 3.0)
project(ECU)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fPIC")

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/libs)
include_directories(${FreeRTOS_INCLUDE_DIRS})

add_subdirectory(src/shared)
include_directories(${ECU_SHARED_INCLUDE})

add_subdirectory(src/sdk)
include_directories(${ECU_SDK_INCLUDE})

add_subdirectory(src/program)
