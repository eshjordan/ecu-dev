
idf_component_register(
    SRCS
        "${CMAKE_SOURCE_DIR}/../shared/CANMsg.c"
        "${CMAKE_SOURCE_DIR}/../shared/CRC.c"
        "${CMAKE_SOURCE_DIR}/../shared/Header.c"
        "${CMAKE_SOURCE_DIR}/../shared/ESP32Msg.c"
        "${CMAKE_SOURCE_DIR}/../shared/Message.c"
    INCLUDE_DIRS
        "include"
        "${CMAKE_SOURCE_DIR}/../shared"
)
