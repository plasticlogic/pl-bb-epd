/*
 * generic_controller.h
 *
 *  Created on: 23.03.2015
 *      Author: sebastian.friebe
 */

#ifndef GENERIC_CONTROLLER_H_
#define GENERIC_CONTROLLER_H_

#include <pl/types.h>

typedef struct pl_generic_controller{
	void *hw_ref;
	int regDefaultsCount;
	regSetting_t *regDefaults;
	unsigned xres;
	unsigned yres;
	const struct pl_wfid *wf_table;
	enum pl_epdc_power_state power_state;
	enum pl_epdc_temp_mode temp_mode;
	int manual_temp;
	char *waveform_file_path;					//!< remember last waveform path used
	char *instruction_code_path;
	int display_scrambling;
	int gate_offset;
	int source_offset;

	void (*delete)(struct pl_generic_controller *p);
	int (*init)(struct pl_generic_controller *p, int use_wf_from_nvm);
	int (*read_register)(struct pl_generic_controller *p, const regSetting_t* setting);
	int (*write_register)(struct pl_generic_controller *p, const regSetting_t setting, const uint32_t bitmask);
	int (*send_cmd)(struct pl_generic_controller *p, const regSetting_t setting);
	int (*configure_update)(struct pl_generic_controller *p, int wfid, enum pl_update_mode mode, const struct pl_area *area);
	int (*clear_update)(struct pl_generic_controller *p);
	int (*trigger_update)(struct pl_generic_controller *p);
	int (*wait_update_end)(struct pl_generic_controller *p);
	int (*fill)(struct pl_generic_controller *p, const struct pl_area *area, uint8_t g);
	int (*load_wflib)(struct pl_generic_controller *p, const char *filename);
	int (*load_image)(struct pl_generic_controller *p, const char *path, const struct pl_area *area, int left, int top);
	int (*load_buffer)(struct pl_generic_controller *p, const char* buffer, const struct pl_area *area, int left, int top);
	int (*set_power_state)(struct pl_generic_controller *p, enum pl_epdc_power_state state);
	int (*set_temp_mode)(struct pl_generic_controller *p, enum pl_epdc_temp_mode mode);
	int (*update_temp)(struct pl_generic_controller *p);

} pl_generic_controller_t;

pl_generic_controller_t *generic_controller_new();
int pl_generic_controller_get_wfid(pl_generic_controller_t *p, const char *wf_path);

#endif /* GENERIC_CONTROLLER_H_ */
