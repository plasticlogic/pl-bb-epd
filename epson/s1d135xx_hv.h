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
 * s1d135xx_hv.h
 *
 *  Created on: 29.04.2015
 *      Author: sebastian.friebe
 */

#ifndef S1D135XX_HV_H_
#define S1D135XX_HV_H_

#include "epson-s1d135xx.h"
#include "pl/hv.h"
#include <errno.h>
#include <errno.h>
/**
 * creates a pl_hv_driver object based on the EPSON controllers.
 *
 * @param s1d135xx pointer to a pmic HW implementation
 * @return pointer to a pl_hv_driver interface for s1d135xx
 * @see pl_hv_driver
 */
pl_hv_driver_t *s1d135xx_get_hv_driver(s1d135xx_t *s1d135xx);

/**
 * creates a pl_vcom_driver object based on the EPSON controllers
 *
 * @param s1d135xx pointer to a s1d135xx variable
 * @return pointer to a pl_vcom_driver interface for s1d135xx
 * @see pl_vcom_driver
 */
pl_vcom_driver_t *s1d135xx_get_vcom_driver(s1d135xx_t *s1d135xx);

/**
 * creates a pl_vcom_switch object based on the EPSON controllers
 *
 * @param s1d135xx pointer to a s1d135xx variable
 * @return pointer to a pl_vcom_switch interface for s1d135xx
 * @see pl_vcom_switch
 */
pl_vcom_switch_t *s1d135xx_get_vcom_switch(s1d135xx_t *s1d135xx);


#endif /* S1D135XX_HV_H_ */
