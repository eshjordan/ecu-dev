
idf_component_register(
    SRCS
        "app_main.c"
        "spi_commands.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        ecu-pins
        ecu-shared
)
