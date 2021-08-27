cmake_minimum_required(VERSION 3.0)
project(RTOS_TEST)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

# add_subdirectory(libs)
include(libs/CMakeLists.txt)

add_definitions(-DBUILD_DIR="build")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(RTOS_TEST_INCLUDE
    src
)

file(GLOB RTOS_TEST_SRC
    src/*.c
    src/*.cpp
)

include_directories(
    ${RTOS_TEST_INCLUDE}
    ${FreeRTOS_INCLUDE_DIRS}
)

add_executable(rtos_bin
    ${RTOS_TEST_SRC}
)

target_link_libraries(rtos_bin
    pthread
    rtos
)
