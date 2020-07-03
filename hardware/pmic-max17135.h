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
 * max17135-pmic.h -- Driver for the MAXIM MAX17135 HVPMIC device.
 *
 * Authors:
 *  Nick Terry <nick.terry@plasticlogic.com>
 *  Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_MAX17135_PMIC_H
#define INCLUDE_MAX17135_PMIC_H 1

#include <stdint.h>
#include <pl/i2c.h>
#include <pl/vcom.h>
#include <pl/pmic.h>
#include <errno.h>

#define HVPMIC_NB_TIMINGS 8

union max17135_fault {
	struct {
		char fbpg:1;
		char hvinp:1;
		char hvinn:1;
		char fbng:1;
		char hvinpsc:1;
		char hvinnsc:1;
		char ot:1;
		char pok:1;
	};
	uint8_t byte;
};

typedef struct max17135 {

} max17135_t;

union max17135_temp_config {
	struct {
		char shutdown:1;
	};
	uint8_t byte;
};

union max17135_temp_status {
	struct {
		char isbusy:1;
		char isopen:1;
		char isshort:1;
	};
	uint8_t byte;
};

union max17135_temp_value {
	struct {
		int padding:7;
		int measured:9;
	};
	uint16_t word;
};

pl_pmic_t *max17135_new(struct pl_i2c *i2c, uint8_t i2c_addr);

#endif /* INCLUDE_MAX17135_PMIC_H */
