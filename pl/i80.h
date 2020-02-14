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
 * i80.h
 *
 *  Created on: 24.01.2020
 *      Author: matti.haugwitz
 */

#ifndef I80_H_
#define I80_H_

#include <errno.h>

typedef struct pl_i80
{
  void *hw_ref;		// hardware reference

  // parallel interface
  int fd;           // open file descriptor: /dev/spi-X.Y

  // control signal interface
  int hwe_n_gpio;
  int hrd_n_gpio;
  int hcs_n_gpio;	// chip select gpio
  int hdc_gpio;
  int hrdy_gpio;
  int hirq_gpio;

  int (*open)(struct pl_i80 * p);
  int (*close)(struct pl_i80 * p);
  int (*read_bytes)(struct pl_i80 *p, uint8_t *buff, size_t size);
  int (*write_bytes)(struct pl_i80 *p, uint8_t *buff, size_t size);
  int (*set_cs)(struct pl_i80 *p, uint8_t cs);
  void (*delete)(struct pl_i80 *p);

} pl_i80_t;

#endif /* I80_H_ */
