/*
  Plastic Logic EPD project on BeagleBone

  Copyright (C) 2018 Plastic Logic

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
#include <errno.h>
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
