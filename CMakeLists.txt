cmake_minimum_required(VERSION 3.0)
project(ECU_SDK)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

set(ECU_ENABLE_NETWORKING ON)

include(libs/CMakeLists.txt)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ECU_SDK_INCLUDE sdk)

file(GLOB ECU_SDK_SRC sdk/*.c sdk/*.cpp)

include_directories(${ECU_SDK_INCLUDE} ${FreeRTOS_INCLUDE_DIRS})

add_library(ecu_sdk ${ECU_SDK_SRC})

target_link_libraries(ecu_sdk pthread rtos)

set(ECU_INCLUDE sdk)

file(GLOB ECU_SRC src/*.c src/*.cpp)

include_directories(${ECU_INCLUDE})

add_executable(rtos_bin ${ECU_SRC})

target_link_libraries(rtos_bin pthread ecu_sdk)
