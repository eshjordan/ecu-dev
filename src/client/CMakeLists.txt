cmake_minimum_required(VERSION 3.0)
project(ECU_CLIENT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${C_FLGS} -std=c++17")

file(GLOB ECU_CLIENT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

set(ECU_CLIENT_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${ECU_CLIENT_INCLUDE}
)

add_executable(ecu_client
    ${ECU_CLIENT_SRC}
)

target_link_libraries(ecu_client
    pthread
    ecu_shared
)

set(ECU_CLIENT_INCLUDE
    ${ECU_CLIENT_INCLUDE}
    PARENT_SCOPE
)
