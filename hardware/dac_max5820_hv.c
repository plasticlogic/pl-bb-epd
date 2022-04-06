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
 * dac-max5820-hv.c
 *
 *  Created on: 14.04.2015
 *      Author: matti.haugwitz
 */


#include <stdlib.h>
#include <hardware/dac_max5820_hv.h>
#include <pl/assert.h>
#define LOG_TAG "max5820_hv"
#include <pl/utils.h>


static int dac_max5820_vcom_config_set(struct pl_vcom_config *p, double vcomInMillivolt);

// -----------------------------------------------------------------------------
// vcom_config - interface implementation
// ------------------------------
pl_vcom_config_t *dac_max5820_get_vcom_config(dac_max5820_t *dac_max5820){
	assert(dac_max5820 != NULL);

	struct pl_vcom_config *p = vcom_config_new();
	p->hw_ref = dac_max5820;
	p->set_vcom = dac_max5820_vcom_config_set;
	p->init = NULL;
	return p;
}

static int dac_max5820_vcom_config_set(struct pl_vcom_config *p, double vcomInMillivolt){
	assert(p != NULL);
	dac_max5820_t *dac_max5820 = (dac_max5820_t*)p->hw_ref;

	return dac_max5820->set_vcom_voltage(dac_max5820, (int)vcomInMillivolt);
}
