#ifndef BEAGLEBONE_SPI_H_
#define BEAGLEBONE_SPI_H_

#include <stddef.h>
#include <stdint.h>
#include <pl/global.h>
#include <pl/spi_hrdy.h>
#include <pl/spi.h>
#include <errno.h>
//******************************************************************************
// Constants
//******************************************************************************

#define USE_8BIT_MODE                 ( 1 )

#define SPI_HRDY_TRANSFER_RATE_IN_HZ       ( 2000000 ) // spi transfer rate is compiled into the kernel
#define SPI_HRDY_TRANSFER_MODE       ( 0 )

#define MAX_SPI_TRANSFER_BUFFERS_hrdy     ( 64 )
#define MAX_SPI_BYTES_PER_TRANSFER_hrdy    ( 64 )

//******************************************************************************
// Definitions
//******************************************************************************
//typedef for variables
typedef unsigned char TByte; //1 byte
typedef unsigned short TWord; //2 bytes
typedef unsigned long TDWord; //4 bytes

#ifdef USE_8BIT_MODE
#define SPI_WORD_TYPE                 uint8_t
#define SPI_BITS_PER_WORD             ( 8 )
#else
#define SPI_WORD_TYPE                 uint16_t
#define SPI_BITS_PER_WORD             ( 9 )
#endif
#define WAIT_FOR_READY_TIMEOUT_SPI_HRDY 100000


struct pl_gpio;

pl_spi_hrdy_t *beaglebone_spi_hrdy_new(uint8_t spi_channel, struct pl_gpio * hw_ref);

#endif /* BEAGLEBONE_SPI_H_ */
