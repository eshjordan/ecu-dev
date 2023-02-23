cmake_minimum_required(VERSION 3.0)
project(FreeRTOS)

# The minimum required FreeRTOS src files
file(GLOB FreeRTOS_CORE_SRC
    # Core
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

if(${STATIC_ALLOCATION})
    file(GLOB FreeRTOS_CORE_SRC
        ${FreeRTOS_CORE_SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MemMang/heap_1.c
    )

    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-wrap,vPortFree")

else()
    file(GLOB FreeRTOS_CORE_SRC
        ${FreeRTOS_CORE_SRC}
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MemMang/heap_4.c
    )

endif()

file(GLOB FreeRTOS_OPT_SRC
    # Optional
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/event_groups.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/libs/FreeRTOS/Source/stream_buffer.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/libs/FreeRTOS/Source/croutine.c
)

file(GLOB FreeRTOS_TCP_SRC
    # TCP
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_IP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_ARP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DNS.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c
)

file(GLOB FreeRTOS_TCP_INTERFACE_SRC
    # Interface
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/NetworkInterface/linux/NetworkInterface.c
)

file(GLOB FreeRTOS_TCP_BUFFER_SRC
    # Buffer Management
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_1.c
)

file(GLOB FreeRTOS_SRC
    ${FreeRTOS_CORE_SRC}
    ${FreeRTOS_OPT_SRC}
    ${FreeRTOS_TCP_SRC}
    ${FreeRTOS_TCP_INTERFACE_SRC}
    ${FreeRTOS_TCP_BUFFER_SRC}
    # Our stuff
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOSConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOSIPConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RTOS.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTOS.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTOS_IP.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RTOS_IP.hpp
)

# Include dirs for FreeRTOS
set(FreeRTOS_INCLUDE_DIRS
    # Core
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
    # TCP
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC
    # Our stuff
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${FreeRTOS_INCLUDE_DIRS}
)

add_library(rtos
    ${FreeRTOS_SRC}
)

target_link_libraries(rtos
    pthread
    pcap
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_custom_command(TARGET rtos POST_BUILD
        COMMAND ${CMAKE_STRIP} -g -S -d --strip-debug
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/librtos.so"
    )
endif()

set(FreeRTOS_INCLUDE_DIRS
    ${FreeRTOS_INCLUDE_DIRS}
    PARENT_SCOPE
)
