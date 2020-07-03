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
 * generic_interface.h
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#ifndef GENERIC_INTERFACE_H_
#define GENERIC_INTERFACE_H_

#include <beaglebone/beaglebone-spi.h>
#include <beaglebone/beaglebone-parallel.h>
#include <errno.h>

enum interfaceType{
	PARALLEL = 0,
	SPI,
	I80
};

typedef struct pl_generic_interface {
	void *hw_ref;		// hardware reference
	int fd;           // open file descriptor: /dev/spi-X.Y
	int cs_gpio; 		// chip select gpio
	//enum interfaceType interface_type; //SPI

	int (*open)(struct pl_generic_interface * p);
	int (*close)(struct pl_generic_interface * p);
	int (*read_bytes)(struct pl_generic_interface *p, uint8_t *buff, size_t size);
	int (*write_bytes)(struct pl_generic_interface *p, uint8_t *buff, size_t size);
	int (*set_cs)(struct pl_generic_interface *p, uint8_t cs);
	void (*delete)(struct pl_generic_interface *p);

	struct spi_metadata *mSpi;
}pl_generic_interface_t;

pl_generic_interface_t* interface_new(uint8_t spi_channel, struct pl_gpio* p_gpio, enum interfaceType type);

#endif /* GENERIC_INTERFACE_H_ */
