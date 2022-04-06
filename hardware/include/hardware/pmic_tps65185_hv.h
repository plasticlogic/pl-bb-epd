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
 * pmic-tps65185-hv.h
 *
 *  Created on: 16.04.2015
 *      Author: sebastian.friebe
 */

#ifndef PMIC_TPS65185_HV_H_
#define PMIC_TPS65185_HV_H_

#include <hardware/pmic_tps65185.h>
#include <pl/hv.h>
#include <errno.h>

/**
 * creates a pl_hv_driver object based on the Texas Instruments pmic TPS65185.
 *
 * @param tps65185 pointer to a pmic HW implementation
 * @return pointer to a pl_hv_driver interface for tps65185
 * @see pl_hv_driver
 */
pl_hv_driver_t *tps65185_get_hv_driver(pl_pmic_t *tps65185);

/**
 * creates a pl_hv_timing object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_hv_timing interface for tps65185
 * @see pl_hv_timing
 */
pl_hv_timing_t *tps65185_get_hv_timing(pl_pmic_t *tps65185);

/**
 * creates a pl_hv_config object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_hv_config interface for tps65185
 * @see pl_hv_config
 */
pl_hv_config_t *tps65185_get_hv_config(pl_pmic_t *tps65185);

/**
 * creates a pl_vcom_driver object based on the  Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_vcom_driver interface for tps65185
 * @see pl_vcom_driver
 */
pl_vcom_driver_t *tps65185_get_vcom_driver(pl_pmic_t *tps65185);

/**
 * creates a pl_vcom_config object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_vcom_config interface for tps65185
 * @see pl_vcom_config
 */
pl_vcom_config_t *tps65185_get_vcom_config(pl_pmic_t *tps65185);

/**
 * creates a pl_vcom_switch object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_vcom_switch interface for tps65185
 * @see pl_vcom_switch
 */
pl_vcom_switch_t *tps65185_get_vcom_switch(pl_pmic_t *tps65185);
#endif /* PMIC_TPS65185_HV_H_ */
