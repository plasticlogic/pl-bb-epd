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
 * generic_epdc.h
 *
 *  Created on: 23.03.2015
 *      Author: sebastian.friebe
 */

#ifndef GENERIC_EPDC_H_
#define GENERIC_EPDC_H_

#include <pl/hv.h>
#include <pl/nvm.h>
#include <pl/generic_controller.h>
#include <errno.h>

typedef struct pl_generic_epdc {
	int default_vcom;
	pl_hv_t *hv;
	pl_nvm_t *nvm;
	pl_generic_controller_t *controller;

	void (*delete)(struct pl_generic_epdc *p);
	int (*init)(struct pl_generic_epdc *p, int load_nvm_content);
	int (*clear_init)(struct pl_generic_epdc *p);
	int (*update)(struct pl_generic_epdc *p, int wfid, enum pl_update_mode mode,
			const struct pl_area *area);
	int (*acep_update)(struct pl_generic_epdc *p, struct pl_gpios *gpios, int wfid, enum pl_update_mode mode,
			const struct pl_area *area);
	int (*set_vcom)(struct pl_generic_epdc *p, int vcomInMillivolt);
	int (*get_vcom)(struct pl_generic_epdc *p);
	int (*read_register)(struct pl_generic_epdc *p, const regSetting_t* setting);
	int (*write_register)(struct pl_generic_epdc *p, const regSetting_t setting,
			const uint32_t bitmask);
	int (*send_cmd)(struct pl_generic_epdc *p, const regSetting_t setting);
	int (*get_resolution)(struct pl_generic_controller *p, int* xres, int* yres);

} pl_generic_epdc_t;

#define TEST = 1;

struct pl_generic_epdc *generic_epdc_new();
int do_load_nvm_content(struct pl_generic_epdc *p);

#endif /* GENERIC_EPDC_H_ */
