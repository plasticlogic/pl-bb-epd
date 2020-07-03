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
 * epson-i2c.h -- Epson i2c master Controller driver
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef EPSON_I2C_H
#define EPSON_I2C_H 1

#include <epson/epson-s1d135xx.h>
#include <errno.h>
struct s1d135xx;
struct pl_i2c;

extern int epson_i2c_init(struct s1d135xx *p, struct pl_i2c *i2c);

#endif /* EPSON_I2C_H */
