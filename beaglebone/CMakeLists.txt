#-------------------------------------------------------------------------------
# Add source files
#-------------------------------------------------------------------------------

set(LIB_NAME beaglebone)

add_library(${LIB_NAME} STATIC
        beaglebone_gpio.c
        beaglebone_hv.c
        beaglebone_i2c.c
        beaglebone_i80.c
        beaglebone_parallel.c
        beaglebone_spi_hrdy.c
        beaglebone_spi.c
)

target_include_directories(${LIB_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME}
        pl        
)

#-------------------------------------------------------------------------------
# Add Tests
#-------------------------------------------------------------------------------