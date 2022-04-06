#-------------------------------------------------------------------------------
# Add source files
#-------------------------------------------------------------------------------

set(LIB_NAME ite)

add_library(${LIB_NAME} STATIC
        it8951_controller.c
        it8951_hv.c
        it8951_i2c.c
        it8951.c
)

target_include_directories(${LIB_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME}
        pl
)

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------