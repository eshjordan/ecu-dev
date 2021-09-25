cmake_minimum_required(VERSION 3.0)
project(FreeRTOS)

# The minimum required FreeRTOS src files
file(GLOB FreeRTOS_CORE_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/tasks.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/queue.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/list.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/MemMang/heap_4.c
)

# Additional FreeRTOS src files we need for timers
file(GLOB FreeRTOS_OPT_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/timers.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/event_groups.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/stream_buffer.c
    # ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/croutine.c
)

# Include dirs for FreeRTOS
set(FreeRTOS_CORE_INCLUDE
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
)

if(ECU_ENABLE_NETWORKING)
    add_compile_options(-DECU_ENABLE_NETWORKING)

    file (GLOB FreeRTOS_Plus_TCP_SRC
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_IP.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_ARP.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DNS.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c
    )

    file (GLOB FreeRTOS_Plus_TCP_Interface_SRC
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/NetworkInterface/linux/NetworkInterface.c
    )

    file (GLOB FreeRTOS_Plus_TCP_BufferManagement_SRC
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_2.c
    )

    set(FreeRTOS_Plus_TCP_INCLUDE
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC
    )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${FreeRTOS_CORE_INCLUDE}
    ${FreeRTOS_Plus_TCP_INCLUDE}
    ${CMAKE_SOURCE_DIR}/libs
)

add_library(rtos
    ${FreeRTOS_CORE_SRC}
    ${FreeRTOS_OPT_SRC}
    ${FreeRTOS_Plus_TCP_SRC}
    ${FreeRTOS_Plus_TCP_Interface_SRC}
    ${FreeRTOS_Plus_TCP_BufferManagement_SRC}
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOSConfig.h
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOSIPConfig.h
    ${CMAKE_SOURCE_DIR}/libs/RTOS.cpp
    ${CMAKE_SOURCE_DIR}/libs/RTOS.hpp
    ${CMAKE_SOURCE_DIR}/libs/RTOS_IP.cpp
    ${CMAKE_SOURCE_DIR}/libs/RTOS_IP.hpp
)

target_link_libraries(rtos
    pthread
)

   
if(ECU_ENABLE_NETWORKING)
    target_link_libraries(rtos
        pcap
    )
endif()
