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
 * beaglebone-gpio.h
 *
 *  Created on: 21.07.2014
 *      Author: sebastian.friebe, matti.haugwitz
 */

#ifndef BEAGLEBONE_GPIO_H_
#define BEAGLEBONE_GPIO_H_

#include "pl/gpio.h"

#define BEAGLEBONE_GPIO(_port, _pin) ((_port) << 5 | _pin)

struct pl_gpio;

extern int beaglebone_gpio_init(struct pl_gpio *gpio);

#endif /* BEAGLEBONE_GPIO_H_ */
