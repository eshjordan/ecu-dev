cmake_minimum_required(VERSION 3.0)
project(ECU_CLIENT)

file(GLOB ECU_CLIENT_SRC
    ${CMAKE_SOURCE_DIR}/src/client/*.c
    ${CMAKE_SOURCE_DIR}/src/client/*.cpp
)

set(ECU_CLIENT_INCLUDE
    ${CMAKE_SOURCE_DIR}/src/client
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
