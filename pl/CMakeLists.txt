#-------------------------------------------------------------------------------
# Add library
#-------------------------------------------------------------------------------

set(LIB_NAME pl)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(${LIB_NAME} STATIC
        color.c
        display.c
        generic_controller.c
        generic_epdc.c
        generic_interface.c
        gpio.c
        hv.c
        hwinfo.c
        i2c.c
        nvm.c
        parser.c
        scramble.c
        utils.c
        vcom.c
)

target_include_directories(${LIB_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/beaglebone/include
        PRIVATE ${CMAKE_SOURCE_DIR}/epson/include
)

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------