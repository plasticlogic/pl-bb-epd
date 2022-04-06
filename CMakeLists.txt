cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

project(pl-bb-epd
        VERSION 1.4.1
        LANGUAGES C
)

set(APP_NAME epdc-app)

#-------------------------------------------------------------------------------
# Add executable
#-------------------------------------------------------------------------------

add_executable(${APP_NAME}
        src/configparser.c
        src/dictionary.c
        src/hw_setup.c
        src/iniparser.c
        src/main.c
        src/text.c
)

target_include_directories(${APP_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SYSROOT}/usr/include/freetype2
)

add_subdirectory(beaglebone)
add_subdirectory(epson)
add_subdirectory(hardware)
add_subdirectory(ite)
add_subdirectory(pl)

target_link_libraries(${APP_NAME}
        pl
        beaglebone
        ite
        epson
        hardware
)

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------