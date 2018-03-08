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
 * pmic.h
 *
 *  Created on: 16.04.2015
 *      Author: sebastian.friebe
 */

#include <errno.h>
#ifndef PMIC_H_
#define PMIC_H_


typedef struct pl_pmic {
	struct pl_i2c *i2c;
	uint8_t i2c_addr;
	struct vcom_cal *cal;
	int is_initialized;

	uint8_t vgl_on_offset_time;
	uint8_t vgl_off_offset_time;
	uint8_t vgh_on_offset_time;
	uint8_t vgh_off_offset_time;
	uint8_t vsl_on_offset_time;
	uint8_t vsl_off_offset_time;
	uint8_t vsh_on_offset_time;
	uint8_t vsh_off_offset_time;

	void (*delete)(struct pl_pmic *p);
	int (*init)(struct pl_pmic *p);
	int (*check_revision_code)(struct pl_pmic *p);

	int (*apply_timings)(struct pl_pmic *pmic);
	int (*configure)(struct pl_pmic *pmic, struct vcom_cal *cal);
	int (*set_vcom_register)(struct pl_pmic *pmic, int dac_value);
	int (*set_vcom_voltage)(struct pl_pmic *pmic, int mv);
	int (*get_vcom_voltage)(struct pl_pmic *pmic);
	int (*wait_pok)(struct pl_pmic *pmic);
	int (*hv_enable)(struct pl_pmic *pmic);
	int (*hv_disable)(struct pl_pmic *pmic);
	int (*vcom_enable)(struct pl_pmic *pmic);
	int (*vcom_disable)(struct pl_pmic *pmic);
	int (*temp_enable)(struct pl_pmic *pmic);
	int (*temp_disable)(struct pl_pmic *pmic);
	int (*temperature_measure)(struct pl_pmic *pmic, int16_t *measured);
} pl_pmic_t;

#endif /* PMIC_H_ */
