#-------------------------------------------------------------------------------
# Add source files
#-------------------------------------------------------------------------------

set(LIB_NAME epson)

add_library(${LIB_NAME} STATIC
        epson_i2c.c
        epson_s1d135xx.c
        epson_spi.c
        s1d135xx_hv.c
        s1d13524_controller.c
        s1d13541_controller.c
        s1d13541_nvm.c
)

target_include_directories(${LIB_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/beaglebone/include
)

target_link_libraries(${LIB_NAME}
        pl
)

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------