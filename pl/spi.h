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
 * spi.h
 *
 *  Created on: 28.04.2015
 *      Author: sebastian.friebe
 */

#ifndef SPI_H_
#define SPI_H_

struct spi_metadata {
	uint8_t channel;  // SPI channel number
	uint8_t mode;     // current SPI mode
	uint8_t bpw;      // current SPI bits per word setting
	uint32_t msh;     // current SPI max speed setting in Hz
};

typedef struct pl_spi
{
  void *hw_ref;		// hardware reference
  int fd;           // open file descriptor: /dev/spi-X.Y
  int cs_gpio; 		// chip select gpio

  int (*open)(struct pl_spi * p);
  int (*close)(struct pl_spi * p);
  int (*read_bytes)(struct pl_spi *p, uint8_t *buff, size_t size);
  int (*write_bytes)(struct pl_spi *p, uint8_t *buff, size_t size);
  int (*set_cs)(struct pl_spi *p, uint8_t cs);
  void (*delete)(struct pl_spi *p);

  struct spi_metadata *mSpi;
  /*uint8_t channel;  // SPI channel number
  uint8_t mode;     // current SPI mode
  uint8_t bpw;      // current SPI bits per word setting
  uint32_t msh;     // current SPI max speed setting in Hz*/
} pl_spi_t;

#endif /* SPI_H_ */
