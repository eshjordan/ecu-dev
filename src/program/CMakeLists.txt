cmake_minimum_required(VERSION 3.0)
project(ECU_PROGRAM)

file(GLOB ECU_PROGRAM_SRC
    ${CMAKE_SOURCE_DIR}/src/program/*.c
    ${CMAKE_SOURCE_DIR}/src/program/*.cpp
)

set(ECU_PROGRAM_INCLUDE
    ${CMAKE_SOURCE_DIR}/src/program
)

add_executable(ecu_program
    ${ECU_PROGRAM_SRC}
)

target_link_libraries(ecu_program
    pthread
    rtos
    ecu_shared
    ecu_sdk
)

set(ECU_PROGRAM_INCLUDE
    ${ECU_PROGRAM_INCLUDE}
    PARENT_SCOPE
)
