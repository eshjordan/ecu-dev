cmake_minimum_required(VERSION 3.0)
project(FreeRTOS)

# The minimum required FreeRTOS src files
file(GLOB FreeRTOS_CORE_SRC
    # Core
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/tasks.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/queue.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/list.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/portable/MemMang/heap_4.c
)

file(GLOB FreeRTOS_OPT_SRC
    # Optional
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/timers.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/event_groups.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/stream_buffer.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/croutine.c
)

file(GLOB FreeRTOS_TCP_SRC
    # TCP
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_IP.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_ARP.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DNS.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c
)

file(GLOB FreeRTOS_TCP_INTERFACE_SRC
    # Interface
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/NetworkInterface/linux/NetworkInterface.c
)

file(GLOB FreeRTOS_TCP_BUFFER_SRC
    # Buffer Management
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_2.c
)

file(GLOB FreeRTOS_SRC
    ${FreeRTOS_CORE_SRC}
    ${FreeRTOS_OPT_SRC}
    ${FreeRTOS_TCP_SRC}
    ${FreeRTOS_TCP_INTERFACE_SRC}
    ${FreeRTOS_TCP_BUFFER_SRC}
    # Our stuff
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOSConfig.h
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOSIPConfig.h
    ${CMAKE_SOURCE_DIR}/src/libs/RTOS.cpp
    ${CMAKE_SOURCE_DIR}/src/libs/RTOS.hpp
    ${CMAKE_SOURCE_DIR}/src/libs/RTOS_IP.cpp
    ${CMAKE_SOURCE_DIR}/src/libs/RTOS_IP.hpp
)

# Include dirs for FreeRTOS
set(FreeRTOS_INCLUDE_DIRS
    # Core
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
    # TCP
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include
    ${CMAKE_SOURCE_DIR}/src/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC
    # Our stuff
    ${CMAKE_SOURCE_DIR}/src/libs
)

include_directories(
    ${FreeRTOS_INCLUDE_DIRS}
)

add_library(rtos SHARED
    ${FreeRTOS_SRC}
)

target_link_libraries(rtos
    pthread
    pcap
)

set(FreeRTOS_INCLUDE_DIRS
    ${FreeRTOS_INCLUDE_DIRS}
    PARENT_SCOPE
)
