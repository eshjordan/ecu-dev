
file(GLOB SRC
    "${CMAKE_SOURCE_DIR}/../shared/*.c"
    "${CMAKE_SOURCE_DIR}/../shared/*.cpp"
)


idf_component_register(
    SRCS
        ${SRC}
    INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/../shared"
)
