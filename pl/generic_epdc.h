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

typedef struct pl_generic_epdc{
	int default_vcom;
	pl_hv_t *hv;
	pl_nvm_t *nvm;
	pl_generic_controller_t *controller;

	void (*delete)(struct pl_generic_epdc *p);
	int (*init)(struct pl_generic_epdc *p, int load_nvm_content);
	int (*clear_init)(struct pl_generic_epdc *p);
	int (*update)(struct pl_generic_epdc *p, int wfid, enum pl_update_mode mode, const struct pl_area *area);
	int (*set_vcom)(struct pl_generic_epdc *p, int vcomInMillivolt);
	int (*get_vcom)(struct pl_generic_epdc *p);
	int (*read_register)(struct pl_generic_epdc *p, const regSetting_t* setting);
	int (*write_register)(struct pl_generic_epdc *p, const regSetting_t setting, const uint32_t bitmask);
	int (*send_cmd)(struct pl_generic_epdc *p, const regSetting_t setting);

} pl_generic_epdc_t;


struct pl_generic_epdc *generic_epdc_new();
int do_load_nvm_content(struct pl_generic_epdc *p);

#endif /* GENERIC_EPDC_H_ */
