cmake_minimum_required(VERSION 3.0)
project(ECU)

option(RASPI "Try to compile for a Raspberry Pi 3b+" OFF)
string(FIND "${CMAKE_C_COMPILER}" "arm" ARM_COMPILER)

# Find if we're using an x86 or ARM compiler
if(ARM_COMPILER EQUAL -1)
    set(MANAGER_BUILD_DIR "stm32-ecu-manager-server")

    set(TARGET_NAME "amd64")

    set(COMMON_FLAGS "-march=znver1 -mtune=znver1")

    set(C_FLGS "")

elseif(RASPI)

    set(TARGET_NAME "raspi")

    add_definitions(-DRASPI)
    set(COMMON_FLAGS "-march=armv8-a -mtune=cortex-a53 -mfloat-abi=hard -DRASPPI=3 -marm")

    set(C_FLGS "-Wno-psabi -fsigned-char -mno-unaligned-access")

else()

    set(TARGET_NAMEE "arm")

    set(COMMON_FLAGS "")

    set(C_FLGS "")

endif()

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS} -k")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} ${C_FLGS} -std=gnu99 -Wno-implicit -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${C_FLGS} -std=c++11 -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/${TARGET_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/${TARGET_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/${TARGET_NAME})

set(CMAKE_BUILD_TYPE "Debug")
set(STATIC_PROGRAM ON)
set(LOG_STATIC_ALLOCATION_REQUESTS OFF)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions(-DUSERSPACE_BUILD)

if(STATIC_PROGRAM)
    add_definitions(-DSTATIC_PROGRAM)
    if(LOG_STATIC_ALLOCATION_REQUESTS)
        add_definitions(-DLOG_STATIC_ALLOCATION_REQUESTS)
    endif()
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    add_definitions(-DDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Ofast")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Ofast")
    add_definitions(-DNDEBUG)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -Ofast")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -Ofast")
    add_definitions(-DNDEBUG)
endif()

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/libs)
include_directories(${FreeRTOS_INCLUDE_DIRS})

add_subdirectory(src/shared)
include_directories(${ECU_SHARED_INCLUDE})

add_subdirectory(src/sdk)
include_directories(${ECU_SDK_INCLUDE})

add_subdirectory(src/program)

if(${TARGET_NAME} MATCHES "amd64")
    add_subdirectory(src/client)
endif()
