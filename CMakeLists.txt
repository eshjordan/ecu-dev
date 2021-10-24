cmake_minimum_required(VERSION 3.0)
project(ECU)

set(BUILD_TYP amd64)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    add_definitions(-DRASPI)
    set(BUILD_TYP raspi)
    include(${CMAKE_TOOLCHAIN_FILE})
    # set_property(GLOBAL PROPERTY LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYP})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYP})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYP})

set(BUILD_LIBS_AS SHARED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fPIC")

set(CMAKE_BUILD_TOOL DEBUG)

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/libs)
include_directories(${FreeRTOS_INCLUDE_DIRS})

add_subdirectory(src/shared)
include_directories(${ECU_SHARED_INCLUDE})

add_subdirectory(src/sdk)
include_directories(${ECU_SDK_INCLUDE})

add_subdirectory(src/program)

add_subdirectory(src/client)
