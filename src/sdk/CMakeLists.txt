cmake_minimum_required(VERSION 3.0)
project(ECU_SDK)

file(GLOB ECU_SDK_SRC
    ${CMAKE_SOURCE_DIR}/src/sdk/*.c
    ${CMAKE_SOURCE_DIR}/src/sdk/*.cpp
)

set(ECU_SDK_INCLUDE
    ${CMAKE_SOURCE_DIR}/src/sdk
)

include_directories(
    ${ECU_SDK_INCLUDE}
)

add_library(ecu_sdk SHARED
    ${ECU_SDK_SRC}
)

target_link_libraries(ecu_sdk
    pthread
    rtos
    ecu_shared
)

set(ECU_SDK_INCLUDE
    ${ECU_SDK_INCLUDE}
    PARENT_SCOPE
)
