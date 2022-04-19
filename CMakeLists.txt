cmake_minimum_required(VERSION 3.0)

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

find_library(LIBPNG NAMES png)
find_library(LIBGD NAMES gd)
find_library(LIBFT NAMES freetype)
find_library(LIBM NAMES m)
find_library(LIBZ NAMES z)
find_library(LIBFC NAMES fontconfig)
find_library(LIBJPEG NAMES jpeg)
find_library(LIBXPM NAMES Xpm)
find_library(LIBTIFF NAMES tiff)
find_library(LIBWEBP NAMES libwebp.so.6 HINTS ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/)
find_library(LIBEXPAT NAMES expat)
find_library(LIBUUID NAMES uuid)
find_library(LIBPT NAMES pthread)
find_library(LIBX11 NAMES X11)
find_library(LIBZSTD NAMES libzstd.so.1)
find_library(LIBLZMA NAMES lzma)
find_library(LIBJBIG NAMES jbig)
find_library(LIBXCB NAMES xcb)
find_library(LIBDL NAMES dl)
find_library(LIBXAU NAMES Xau)
find_library(LIBXDMCP NAMES Xdmcp)
find_library(LIBBSD NAMES libbsd.so.0)
find_library(LIBRT NAMES rt)

target_link_libraries(${APP_NAME}
        pl
        beaglebone
        ite
        epson
        hardware
        ${LIBPNG}
        ${LIBGD}
        ${LIBFT}
        ${LIBM}
        ${LIBZ}
        ${LIBFC}
        ${LIBJPEG}
        ${LIBXPM}
        ${LIBTIFF}
        ${LIBWEBP}
        ${LIBEXPAT}
        ${LIBUUID}
        ${LIBPT}
        ${LIBX11}
        ${LIBZSTD}
        ${LIBLZMA}
        ${LIBJBIG}
        ${LIBXCB}
        ${LIBDL}
        ${LIBXAU}
        ${LIBXDMCP}
        ${LIBBSD}
        ${LIBRT}
)

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Build Targets
#-------------------------------------------------------------------------------

# Custom target for build only
add_custom_target(build ALL)

# Custom command for building and debugging
add_custom_target(build_and_debug ALL DEPENDS ${APP_NAME})

set(BB_IP 192.168.7.2)
set(BB_PORT 8090)
set(BB_USR debian)
set(BB_PW temppwd)

add_custom_command(TARGET build_and_debug
        POST_BUILD
        COMMAND pscp -pw ${BB_PW} ${CMAKE_BINARY_DIR}/${APP_NAME} ${BB_USR}@${BB_IP}:/home/${BB_USR}
        COMMAND plink -pw ${BB_PW} ${BB_USR}@${BB_IP}:/home/${BB_USR} 'chmod 0755 ${APP_NAME}'
        COMMAND plink -pw ${BB_PW} ${BB_USR}@${BB_IP}:/home/${BB_USR} 'gdbserver :${BB_PORT} ./${APP_NAME}'
)