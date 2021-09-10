cmake_minimum_required(VERSION 3.0)
project(FreeRTOS)

# The minimum required FreeRTOS src files
file(GLOB FreeRTOS_CORE_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/tasks.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/queue.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/list.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/MemMang/heap_3.c
)

# Additional FreeRTOS src files we need for timers
file(GLOB FreeRTOS_OPT_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/timers.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/event_groups.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/stream_buffer.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/croutine.c
)

# Include dirs for FreeRTOS
set(FreeRTOS_CORE_INCLUDE
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${FreeRTOS_CORE_INCLUDE}
    ${CMAKE_SOURCE_DIR}/libs
)

add_library(rtos
    ${FreeRTOS_CORE_SRC}
    ${FreeRTOS_OPT_SRC}
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOSConfig.h
    ${CMAKE_SOURCE_DIR}/libs/RTOS.cpp
    ${CMAKE_SOURCE_DIR}/libs/RTOS.hpp
)

target_link_libraries(rtos
    pthread
)
