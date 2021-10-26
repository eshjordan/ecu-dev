cmake_minimum_required(VERSION 3.0)
project(ECU_SHARED)

file(GLOB ECU_SHARED_SRC
    ${CMAKE_SOURCE_DIR}/src/shared/*.c
    ${CMAKE_SOURCE_DIR}/src/shared/*.cpp
)

set(ECU_SHARED_INCLUDE
    ${CMAKE_SOURCE_DIR}/src/shared
)

include_directories(
    ${ECU_SHARED_INCLUDE}
)

add_library(ecu_shared
    ${ECU_SHARED_SRC}
)

target_link_libraries(ecu_shared
    pthread
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_custom_command(TARGET ecu_shared POST_BUILD
        COMMAND ${CMAKE_STRIP} -g -S -d --strip-debug
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libecu_shared.so"
    )
endif()

set(ECU_SHARED_INCLUDE
    ${ECU_SHARED_INCLUDE}
    PARENT_SCOPE
)
