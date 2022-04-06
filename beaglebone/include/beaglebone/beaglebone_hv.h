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
 * beaglebone-hv.h
 *
 *  Created on: 16 Apr 2015
 *      Author: matti.haugwitz
 */

#ifndef BEAGLEBONE_HV_H_
#define BEAGLEBONE_HV_H_

#include <pl/hv.h>
#include <pl/gpio.h>
#include <errno.h>

/**
 * creates a pl_hv_driver object based on the beaglebone gpios.
 *
 * @param gpio pointer to a pl_gpio structure
 * @return pointer to a pl_hv_driver interface
 */
pl_hv_driver_t *beaglebone_get_hv_driver(struct pl_gpio *gpio);


/**
 * creates a pl_vcom_config object based on the beaglebone gpios.
 *
 * @param gpio is pointer to a pl_gpio structure
 * @return pointer to a pl_vcom_switch interface
 */
pl_vcom_switch_t *beaglebone_get_vcom_switch(struct pl_gpio *gpio);


#endif /* BEAGLEBONE_HV_H_ */
