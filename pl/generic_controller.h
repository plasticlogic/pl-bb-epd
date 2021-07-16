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
 * generic_controller.h
 *
 *  Created on: 23.03.2015
 *      Author: sebastian.friebe
 */

#ifndef GENERIC_CONTROLLER_H_
#define GENERIC_CONTROLLER_H_

#include <pl/types.h>
#include <errno.h>

typedef struct pl_generic_controller {
	void *hw_ref;
	int regDefaultsCount;
	regSetting_t *regDefaults;
	unsigned xres;
	unsigned yres;
	int xoffset;
	int yoffset;
	const struct pl_wfid *wf_table;
	enum pl_epdc_power_state power_state;
	enum pl_epdc_temp_mode temp_mode;
	int manual_temp;
	char *waveform_file_path;			//!< remember last waveform path used
	char *instruction_code_path;
	int display_scrambling;
	int gate_offset;
	int source_offset;
	unsigned imageWidth;
	unsigned imageHeight;
	uint8_t animationMode;
	cfa_overlay_t cfa_overlay;
	enum pl_update_image_mode update_image_mode;

	void (*delete)(struct pl_generic_controller *p);
	int (*init)(struct pl_generic_controller *p, int use_wf_from_nvm);
	int (*read_register)(struct pl_generic_controller *p,
			const regSetting_t* setting);
	int (*write_register)(struct pl_generic_controller *p,
			const regSetting_t setting, const uint32_t bitmask);
	int (*send_cmd)(struct pl_generic_controller *p, const regSetting_t setting);
	int (*configure_update)(struct pl_generic_controller *p, int wfid,
			enum pl_update_mode mode, const struct pl_area *area);
	int (*clear_update)(struct pl_generic_controller *p);
	int (*trigger_update)(struct pl_generic_controller *p);
	int (*wait_update_end)(struct pl_generic_controller *p);
	int (*fill)(struct pl_generic_controller *p, const struct pl_area *area,
			uint8_t g);
	int (*load_wflib)(struct pl_generic_controller *p, const char *filename);
	int (*load_image)(struct pl_generic_controller *p, const char *path,
			const struct pl_area *area, int left, int top);
	int (*load_buffer)(struct pl_generic_controller *p, const char* buffer,
			const struct pl_area *area, int binary);
	int (*set_power_state)(struct pl_generic_controller *p,
			enum pl_epdc_power_state state);
	int (*set_temp_mode)(struct pl_generic_controller *p,
			enum pl_epdc_temp_mode mode);
	int (*update_temp)(struct pl_generic_controller *p);
	int (*get_temp)(struct pl_generic_controller *p, int* temperature);
	int (*get_resolution)(struct pl_generic_controller *p, int* xres, int* yres);
} pl_generic_controller_t;

pl_generic_controller_t *generic_controller_new();
int pl_generic_controller_get_wfid(pl_generic_controller_t *p,
		const char *wf_path);

#endif /* GENERIC_CONTROLLER_H_ */
