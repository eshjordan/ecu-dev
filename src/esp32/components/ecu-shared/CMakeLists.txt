
idf_component_register(
    SRCS
        "${CMAKE_SOURCE_DIR}/../shared/CRC.c"
        "${CMAKE_SOURCE_DIR}/../shared/ECU_Error.c"
        "${CMAKE_SOURCE_DIR}/../shared/Header.c"
        "${CMAKE_SOURCE_DIR}/../shared/ECU_Msg.c"
        "${CMAKE_SOURCE_DIR}/../shared/CAN_Msg.c"
    INCLUDE_DIRS
        "include"
        "${CMAKE_SOURCE_DIR}/../shared"
)
