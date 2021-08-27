cmake_minimum_required(VERSION 3.0)
project(FreeRTOS)

file(GLOB FreeRTOS_CORE_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/*.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/MemMang/heap_3.c
)

set(FreeRTOS_TRC_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcSnapshotRecorder.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcStreamingRecorder.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/streamports/File/trcStreamingPort.c
)

set(FreeRTOS_DEMO_SRC
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/AbortDelay.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/BlockQ.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/blocktim.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/countsem.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/death.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/dynamic.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/EventGroupsDemo.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/flop.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/GenQTest.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/integer.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/IntSemTest.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/MessageBufferAMP.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/MessageBufferDemo.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/PollQ.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/QPeek.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/QueueOverwrite.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/QueueSet.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/QueueSetPolling.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/recmutex.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/semtest.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/StaticAllocation.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/StreamBufferDemo.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/StreamBufferInterrupt.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/TaskNotify.c
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/Minimal/TimerDemo.c
)

set(FreeRTOS_CORE_INCLUDE
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
)

set(FreeRTOS_TRC_INCLUDE
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include
)

set(FreeRTOS_DEMO_INCLUDE
    ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Demo/Common/include
)

file(GLOB Application_SRC
    ${CMAKE_SOURCE_DIR}/libs/*.c
    ${CMAKE_SOURCE_DIR}/libs/*.cpp
)

set(Application_INCLUDE
${CMAKE_SOURCE_DIR}/libs
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DprojCOVERAGE_TEST=0 -DBUILD_DIR="build")

set(FreeRTOS_INCLUDE_DIRS
    ${Application_INCLUDE}
    ${FreeRTOS_CORE_INCLUDE}
    ${FreeRTOS_TRC_INCLUDE}
    ${FreeRTOS_DEMO_INCLUDE}
)

include_directories(
    ${FreeRTOS_INCLUDE_DIRS}
)

add_library(rtos
    ${Application_SRC}
    ${FreeRTOS_CORE_SRC}
    ${FreeRTOS_OPT_SRC}
    ${FreeRTOS_TRC_SRC}
    ${FreeRTOS_DEMO_SRC}
)

target_link_libraries(rtos
    pthread
)
