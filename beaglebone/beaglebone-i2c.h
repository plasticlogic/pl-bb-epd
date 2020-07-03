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
 * beaglebone-i2c.h
 *
 *      Authors:
 *           Nick Terry <nick.terry@plasticlogic.com>
 *   		 Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *   		 sebastian.friebe
 *   		 matti.haugwitz
 */

#ifndef BEAGLEBONE_I2C_H
#define BEAGLEBONE_I2C_H 1

#include <stdint.h>
#include <errno.h>
struct pl_gpio;
struct pl_i2c;

extern int beaglebone_i2c_init(uint8_t channel, struct pl_i2c *i2c);


#endif /* BEAGLEBONE_I2C_H */
