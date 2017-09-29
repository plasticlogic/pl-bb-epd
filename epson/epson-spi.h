/*
 * beaglebone_spi.h
 *
 *  Created on: 23.07.2014
 *      Author: sebastian.friebe
 */

#ifndef BEAGLEBONE_SPI_H_
#define BEAGLEBONE_SPI_H_

#include <stddef.h>
#include <stdint.h>
#include <pl/global.h>
#include <pl/spi.h>
#include <pl/parallel.h>
#include <epson/epson-s1d135xx.h>

//******************************************************************************
// Constants
//******************************************************************************

#define USE_8BIT_MODE                 ( 1 )

#define SPI_TRANSFER_RATE_IN_HZ       ( 1000000 )
#define MAX_SPI_TRANSFER_BUFFERS      ( 64 )
#define MAX_SPI_BYTES_PER_TRANSFER    ( 64 )

//******************************************************************************
// Definitions
//******************************************************************************

#ifdef USE_8BIT_MODE
#define SPI_WORD_TYPE                 uint8_t
#define SPI_BITS_PER_WORD             ( 8 )
#else
#define SPI_WORD_TYPE                 uint16_t
#define SPI_BITS_PER_WORD             ( 9 )
#endif


struct pl_gpio;


pl_spi_t *epson_spi_new(s1d135xx_t *c);
pl_parallel_t *epson_parallel_new(s1d135xx_t *c);

#endif /* BEAGLEBONE_SPI_H_ */
