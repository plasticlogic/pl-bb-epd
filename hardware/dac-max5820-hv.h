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
 * dac-max5820-hv.h
 *
 *  Created on: 14.04.2015
 *      Author: matti.haugwitz
 */


#ifndef DAC_MAX5820_HV_H_
#define DAC_MAX5820_HV_H_

#include <hardware/dac-max5820.h>
#include <pl/hv.h>


/**
 * creates a pl_vcom_config object based on the maxim dac max5820.
 *
 * @param dac_max5820 pointer to a dac_max5820 variable
 * @return pointer to a pl_hv_config interface for dac_max5820
 * @see pl_vcom_config
 */
pl_vcom_config_t *dac_max5820_get_vcom_config(dac_max5820_t *dac_max5820);


#endif /* DAC_MAX5820_HV_H_ */
