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
 * hv.c
 *
 *  Created on: 19.03.2015
 *      Author: sebastian.friebe
 */

#include <stdlib.h>
#include <pl/hv.h>
#include <pl/assert.h>
#define LOG_TAG "hv"
#include <pl/utils.h>
static void hv_delete(struct pl_hv *p);
static int hv_init(struct pl_hv *p);
static void hv_driver_delete(struct pl_hv_driver *p);
static void hv_config_delete(struct pl_hv_config *p);
static void hv_timing_delete(struct pl_hv_timing *p);
static void vcom_switch_delete(struct pl_vcom_switch *p);
static void vcom_config_delete(struct pl_vcom_config *p);
static void vcom_driver_delete(struct pl_vcom_driver *p);

// -----------------------------------------------------------------------------
// public constructor functions
// ------------------------------
/**
 * allocates memory to hold a pl_hv structure
 *
 * @return pointer to allocated memory
 */
struct pl_hv *hv_new(){
	struct pl_hv *hv = (struct pl_hv *)malloc(sizeof(struct pl_hv));;

	hv->delete = hv_delete;
	hv->init = hv_init;

	hv->hvConfig = NULL;
	hv->hvDriver = NULL;
	hv->hvTiming = NULL;
	hv->vcomConfig = NULL;
	hv->vcomDriver = NULL;
	hv->vcomSwitch = NULL;

	return hv;
}
/**
 * allocates memory to hold a pl_hv_driver structure
 *
 * @return pointer to allocated memory
 */
struct pl_hv_driver *hv_driver_new(){
	struct pl_hv_driver *p = (struct pl_hv_driver *)malloc(sizeof(struct pl_hv_driver));
	p->delete = hv_driver_delete;
	p->hw_ref = NULL;
	p->init = NULL;
	p->switch_off = NULL;
	p->switch_on = NULL;
	return p;
}
/**
 * allocates memory to hold a pl_hv_config structure
 *
 * @return pointer to allocated memory
 */
struct pl_hv_config *hv_config_new(){
	struct pl_hv_config *p = (struct pl_hv_config *)malloc(sizeof(struct pl_hv_config));
	p->delete = hv_config_delete;
	p->hw_ref = NULL;
	p->init = NULL;
	p->set_vgh = NULL;
	p->set_vgl = NULL;
	p->set_vsh = NULL;
	p->set_vsl = NULL;
	return p;
}
/**
 * allocates memory to hold a pl_hv_timing structure
 *
 * @return pointer to allocated memory
 */
struct pl_hv_timing *hv_timing_new(){
	struct pl_hv_timing *p = (struct pl_hv_timing *)malloc(sizeof(struct pl_hv_timing));
	p->delete = hv_timing_delete;
	p->get_timings = NULL;
	p->hw_ref = NULL;
	p->init = NULL;
	p->set_timings = NULL;
	p->toffset_vgh_off = 0;
	p->toffset_vgh_on = 0;
	p->toffset_vgl_off = 0;
	p->toffset_vgl_on = 0;
	p->toffset_vsh_off = 0;
	p->toffset_vsh_on = 0;
	p->toffset_vsl_off = 0;
	p->toffset_vsl_on = 0;
	return p;
}
/**
 * allocates memory to hold a pl_vcom_config structure
 *
 * @return pointer to allocated memory
 */
struct pl_vcom_config *vcom_config_new(){
	struct pl_vcom_config *p = (struct pl_vcom_config *)malloc(sizeof(struct pl_vcom_config));
	p->delete = vcom_config_delete;
	p->hw_ref = NULL;
	p->init = NULL;
	p->set_vcom = NULL;
	p->get_vcom = NULL;
	return p;
}
/**
 * allocates memory to hold a pl_vcom_driver structure
 *
 * @return pointer to allocated memory
 */
struct pl_vcom_driver *vcom_driver_new(){
	struct pl_vcom_driver *p = (struct pl_vcom_driver *)malloc(sizeof(struct pl_vcom_driver));
	p->delete = vcom_driver_delete;
	p->hw_ref = NULL;
	p->init = NULL;
	p->switch_off = NULL;
	p->switch_on = NULL;
	return p;
}
/**
 * allocates memory to hold a pl_vcom_switch structure
 *
 * @return pointer to allocated memory
 */
struct pl_vcom_switch *vcom_switch_new(){
	struct pl_vcom_switch *p = (struct pl_vcom_switch *)malloc(sizeof(struct pl_vcom_switch));
	p->delete = vcom_switch_delete;
	p->close = NULL;
	p->disable_bypass_mode = NULL;
	p->enable_bypass_mode = NULL;
	p->hw_ref = NULL;
	p->init = NULL;
	p->is_bypass = 0;
	p->open = NULL;
	return p;
}

// -----------------------------------------------------------------------------
// private delete functions
// ------------------------------
/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void hv_delete(struct pl_hv *p){
	if (p != NULL){
		if (p->hvConfig != NULL) p->hvConfig->delete(p->hvConfig);
		if (p->hvTiming != NULL) p->hvTiming->delete(p->hvTiming);
		if (p->hvDriver != NULL) p->hvDriver->delete(p->hvDriver);
		if (p->vcomConfig != NULL) p->vcomConfig->delete(p->vcomConfig);
		if (p->vcomDriver != NULL) p->vcomDriver->delete(p->vcomDriver);
		if (p->vcomSwitch != NULL) p->vcomSwitch->delete(p->vcomSwitch);

		free(p);
		p = NULL;
	}
}

/**
 * Initializes all high voltage parts.
 *
 * @param p pointer to the hv structure
 * @see pl_hv
 * @return success flag: 0 if passed, <> 0 otherwise
 */
static int hv_init(struct pl_hv *p){
	assert(p != NULL);
	int stat = 0;

	// init hv driver
	if (p->hvDriver != NULL){
		if (p->hvDriver->init != NULL){
			stat |= p->hvDriver->init(p->hvDriver);
		}
	}

	// init hv config
	if (p->hvConfig != NULL){
		//LOG("%s: init hv config", __func__);
		if (p->hvConfig->init != NULL){
			//LOG("%s: init hv config", __func__);
			stat |= p->hvConfig->init(p->hvConfig);
			//LOG("%s: init hv config: stat = 0x%x", __func__, stat);
		}
	}

	// init hv timing
	if (p->hvTiming != NULL){
		if (p->hvTiming->init != NULL){
			stat |= p->hvTiming->init(p->hvTiming);
		}
	}

	// init vcom config
	if (p->vcomConfig != NULL){
		if (p->vcomConfig->init != NULL){
			stat |= p->vcomConfig->init(p->vcomConfig);
		}
	}

	// init vcom driver
	if (p->vcomDriver != NULL){
		if (p->vcomDriver->init != NULL){
			stat |= p->vcomDriver->init(p->vcomDriver);
		}
	}

	// init vcom switch
	if (p->vcomSwitch != NULL){
		if (p->vcomSwitch->init != NULL){
			stat |= p->vcomSwitch->init(p->vcomSwitch);
		}
	}

	return stat;
}

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void hv_driver_delete(struct pl_hv_driver *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void hv_config_delete(struct pl_hv_config *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void hv_timing_delete(struct pl_hv_timing *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void vcom_switch_delete(struct pl_vcom_switch *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}
/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void vcom_config_delete(struct pl_vcom_config *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}
/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void vcom_driver_delete(struct pl_vcom_driver *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}
