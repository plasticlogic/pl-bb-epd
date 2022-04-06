#-------------------------------------------------------------------------------
# Add library
#-------------------------------------------------------------------------------

set(LIB_NAME hardware)

add_library(${LIB_NAME} STATIC
        dac_max5820_hv.c
        dac_max5820.c
        nvm_i2c_24AA256.c
        nvm_i2c_24LC014H.c
        nvm_spi_MX25U4033E.c
        pmic_max17135_hv.c
        pmic_max17135.c
        pmic_tps65185_hv.c
        pmic_tps65185.c
)

target_include_directories(${LIB_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME}
        pl
)

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------