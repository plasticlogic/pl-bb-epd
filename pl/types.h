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
 * types.h -- General type and constant definitions
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_TYPES_H
#define INCLUDE_PL_TYPES_H 1

#include <stdint.h>

struct pl_area {
	int left;
	int top;
	int width;
	int height;
};

typedef struct {
	unsigned int addr;
	unsigned int valCount;
	uint16_t *val;
} regSetting_t;

struct pl_wfid {
	const char *path;
	int id;
};

typedef struct rgbw{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t w;
}rgbw_pixel_t;

typedef struct cfa{
	/* RGBW overlay
	 * 01
	 * 23
	 * => GRBW => R:1, G:0, B:2, W:3
	 */
	int8_t r_position;
	int8_t g_position;
	int8_t b_position;
	int8_t w_position;
}cfa_overlay_t;

enum pl_update_mode {
	PL_FULL_UPDATE = 0,
	PL_PART_UPDATE,
	PL_FULL_AREA_UPDATE,
	PL_PART_AREA_UPDATE,
	PL_FULL_UPDATE_NOWAIT,
	PL_PART_UPDATE_NOWAIT,
	PL_FULL_AREA_UPDATE_NOWAIT,
	PL_PART_AREA_UPDATE_NOWAIT,
};

enum pl_epdc_power_state {
	PL_EPDC_RUN = 0,
	PL_EPDC_STANDBY,
	PL_EPDC_SLEEP,
	PL_EPDC_OFF,
};

enum pl_epdc_temp_mode {
	PL_EPDC_TEMP_MANUAL,
	PL_EPDC_TEMP_EXTERNAL,
	PL_EPDC_TEMP_INTERNAL,
};

#endif /* INCLUDE_PL_TYPES_H */
