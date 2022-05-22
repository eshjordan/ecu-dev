
idf_component_register(
    SRCS
        "CANSPI.c"
        "MCP2515.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        ecu-pins
)
