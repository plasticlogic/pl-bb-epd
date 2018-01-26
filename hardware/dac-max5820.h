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
 * dac_max5820.h
 *
 *  Created on: 25.07.2014
 *      Author: sebastian.friebe
 */

#ifndef DAC_MAX5820_H_
#define DAC_MAX5820_H_

#define MAX5820_I2C_ADDRESS 0x39

#include <stdint.h>
#include <pl/i2c.h>
#include <pl/vcom.h>

typedef struct dac_max5820 {
	struct pl_i2c *i2c;
	uint8_t i2c_addr;
	struct vcom_cal *cal;

	void (*delete)(struct dac_max5820 *p);
	int (*configure)(struct dac_max5820 *p, struct vcom_cal *cal);
	int (*set_output_mode)(struct dac_max5820 *p, int channel_mask, int output_mode);

	int (*set_vcom_register)(struct dac_max5820 *p, int dac_value);
	int (*get_vcom_register)(struct dac_max5820 *p, int *dac_value);
	int (*set_vcom_voltage)(struct dac_max5820 *p, int mv);

} dac_max5820_t;

#define MAX5820_PWR_ON						0x00
#define MAX5820_PWR_OFF_MODE0  				0x01
#define MAX5820_PWR_OFF_MODE1				0x02
#define MAX5820_PWR_OFF_MODE2				0x03
#define MAX5820_CHANNEL_A					0x04
#define MAX5820_CHANNEL_B					0x08

dac_max5820_t *dac_max5820_new(struct pl_i2c *i2c, uint8_t i2c_addr);

#endif /* DAC_MAX5820_H_ */

