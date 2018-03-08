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
 * nvm-i2c-24AA256.h
 *
 *  Created on: 12.12.2014
 *      Author: matti.haugwitz
 */

#include <pl/nvm.h>
#include <pl/i2c.h>
#include <errno.h>

#define I2C_24AA256		0x54
#define SIZE_24AA256	0x8000

int nvm_24AA256_i2c_init(struct pl_nvm * nvm, struct pl_i2c *i2c);




