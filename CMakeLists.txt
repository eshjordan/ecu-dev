cmake_minimum_required(VERSION 3.0)
project(ECU)

set(BUILD_TYP amd64)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    add_definitions(-DRASPI)
    set(BUILD_TYP raspi)
    include(${CMAKE_TOOLCHAIN_FILE})
    # set_property(GLOBAL PROPERTY LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
else()
    set(COMMON_FLAGS " \
        -march=znver1 \
        -mtune=znver1"
    )

    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} ${C_FLGS} -std=gnu99 -Wno-implicit")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${C_FLGS} -std=c++11")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYP})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYP})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYP})

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -k")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(CMAKE_BUILD_TYPE "Debug")
set(STATIC_PROGRAM ON)
set(LOG_STATIC_ALLOCATION_REQUESTS OFF)
set(BUILD_SHARED_LIBS ON)

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

if(NOT ${BUILD_TYP} STREQUAL "raspi")
    add_subdirectory(src/client)
endif()
