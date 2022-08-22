cmake_minimum_required(VERSION 3.0)
project(ECU_PROGRAM)

set(ECU_PROGRAM_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)

set(ECU_PROGRAM_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}
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

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_custom_command(TARGET ecu_program POST_BUILD
        COMMAND ${CMAKE_STRIP} -g -S -d --strip-debug
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ecu_program"
    )
endif()

set(ECU_PROGRAM_INCLUDE
    ${ECU_PROGRAM_INCLUDE}
    PARENT_SCOPE
)
