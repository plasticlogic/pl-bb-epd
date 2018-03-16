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
 * parallel.h
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#ifndef PARALLEL_H_
#define PARALLEL_H_

#include <errno.h>

typedef struct pl_parallel
{
  void *hw_ref;		// hardware reference
  int fd;           // open file descriptor: /dev/spi-X.Y
  int cs_gpio; 		// chip select gpio
 // int interface_type;

  int (*open)(struct pl_parallel * p);
  int (*close)(struct pl_parallel * p);
  int (*read_bytes)(struct pl_parallel *p, uint8_t *buff, size_t size);
  int (*write_bytes)(struct pl_parallel *p, uint8_t *buff, size_t size);
  int (*set_cs)(struct pl_parallel *p, uint8_t cs);
  void (*delete)(struct pl_parallel *p);

  void *mSPI;
} pl_parallel_t;

#endif /* PARALLEL_H_ */
