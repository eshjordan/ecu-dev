set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)
set(TOOLCHAIN /usr/bin/arm-linux-gnueabihf)

set(CMAKE_C_COMPILER ${TOOLCHAIN}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}-g++)
set(CMAKE_STRIP ${TOOLCHAIN}-strip)

set(CMAKE_SYSROOT $ENV{HOME}/raspberrypi/rootfs)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(COMMON_FLAGS " \
    -march=armv8-a \
    -mtune=cortex-a53 \
    -mfloat-abi=hard \
    -DRASPPI=3 \
    -marm"
)

set(C_FLGS " \
    -Wno-psabi \
    -fsigned-char \
    -mno-unaligned-access"
)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} ${C_FLGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} ${C_FLGS}")
