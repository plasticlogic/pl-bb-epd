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
 * vcom.c -- VCOM Calculation support
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "assert.h"
#include "vcom.h"
#include <stdlib.h>

#define LOG_TAG "vcom"
#include "utils.h"

void vcom_init(struct vcom_cal *v, struct pl_hw_vcom_info *c)
{
	assert(v != NULL);
	assert(c != NULL);

	v->dac_dx = c->dac_x2 - c->dac_x1;
	v->dac_dy = c->dac_y2 - c->dac_y1;
	v->dac_offset = c->dac_y1 - DIV_ROUND_CLOSEST((c->dac_x1 * v->dac_dy),  v->dac_dx);
	v->swing = c->vgpos_mv - c->vgneg_mv;
	v->swing_ideal = c->swing_ideal;
	v->dac_step_mv = DIV_ROUND_CLOSEST(v->dac_dy, v->dac_dx);
}

int vcom_calculate(const struct vcom_cal *v, int input_mv)
{
	int32_t scaled_mv;
	int dac_value;
	assert(v != NULL);

	scaled_mv = DIV_ROUND_CLOSEST(input_mv * v->swing, v->swing_ideal);
	dac_value = DIV_ROUND_CLOSEST((scaled_mv - v->dac_offset) * v->dac_dx,
				      v->dac_dy);

	LOG("input: %d, scaled: %d, DAC reg: 0x%02X",
	    input_mv, scaled_mv, dac_value);

	return dac_value;
}

int vcom_calculate_dac(const struct vcom_cal *v, int dac_value)
{
	int32_t scaled_mv;
	int mv;
	assert(v != NULL);

	scaled_mv = /*DIV_ROUND_CLOSEST*/(dac_value * v->dac_dy/ v->dac_dx) + v->dac_offset;
	mv = /*DIV_ROUND_CLOSEST*/(scaled_mv * v->swing_ideal/ v->swing);
#if VERBOSE
	LOG("input: %d, scaled: %d, DAC reg: 0x%02X",
	    mv, scaled_mv, dac_value);
#endif
	return mv;
}
