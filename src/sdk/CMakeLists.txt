cmake_minimum_required(VERSION 3.0)
project(ECU_SDK)

set(ECU_SDK_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ChannelList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Connection_commands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParameterList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RoutineManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/System.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/portable/x86/IO.cpp
)

set(ECU_SDK_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/portable/x86/include
)

include_directories(
    ${ECU_SDK_INCLUDE}
)

add_library(ecu_sdk
    ${ECU_SDK_SRC}
)

target_link_libraries(ecu_sdk
    pthread
    rtos
    ecu_shared
)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    add_custom_command(TARGET ecu_sdk POST_BUILD
        COMMAND ${CMAKE_STRIP} -g -S -d --strip-debug
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libecu_sdk.so"
    )
endif()

set(ECU_SDK_INCLUDE
    ${ECU_SDK_INCLUDE}
    PARENT_SCOPE
)
