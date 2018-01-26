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
 * hv.h
 *
 *  Created on: 19.03.2015
 *      Author: sebastian.friebe
 */

#ifndef HV_H_
#define HV_H_

/**
 * interface definition for a vcom switch hardware
 */
typedef struct pl_vcom_switch{
	void *hw_ref;
	int is_bypass;

	void (*enable_bypass_mode)(struct pl_vcom_switch *p, int switch_state);
	void (*disable_bypass_mode)(struct pl_vcom_switch *p);
	void (*open)(struct pl_vcom_switch *p);
	void (*close)(struct pl_vcom_switch *p);
	int (*init)(struct pl_vcom_switch *p);
	void (*delete)(struct pl_vcom_switch *p);
} pl_vcom_switch_t;


/**
 * interface definition for a vcom configuration device
 */
typedef struct pl_vcom_config{
	void *hw_ref;

	int (*set_vcom)(struct pl_vcom_config *p, double value);
	int (*get_vcom)(struct pl_vcom_config *p);
	int (*init)(struct pl_vcom_config *p);
	void (*delete)(struct pl_vcom_config *p);
} pl_vcom_config_t;

/**
 * interface definition for a vcom driving device
 */
typedef struct pl_vcom_driver{
	void *hw_ref;

	int (*switch_on)(struct pl_vcom_driver *p);
	int (*switch_off)(struct pl_vcom_driver *p);
	int (*init)(struct pl_vcom_driver *p);
	void (*delete)(struct pl_vcom_driver *p);
} pl_vcom_driver_t;


/**
 * interface definition for a HV configuration device
 */
typedef struct pl_hv_config{
	void *hw_ref;

	void (*set_vgh)(struct pl_hv_config *p, double value);
	void (*set_vgl)(struct pl_hv_config *p, double value);
	void (*set_vsh)(struct pl_hv_config *p, double value);
	void (*set_vsl)(struct pl_hv_config *p, double value);
	int (*init)(struct pl_hv_config *p);
	void (*delete)(struct pl_hv_config *p);
} pl_hv_config_t;

/**
 * interface definition for a HV driver device
 */
typedef struct pl_hv_driver{
	void *hw_ref;

	int (*switch_on)(struct pl_hv_driver *p);
	int (*switch_off)(struct pl_hv_driver *p);
	int (*init)(struct pl_hv_driver *p);
	void (*delete)(struct pl_hv_driver *p);
} pl_hv_driver_t;

/**
 * interface definition for a HV timing specification and setting device
 */
typedef struct pl_hv_timing{
	int toffset_vsh_on;
	int toffset_vsh_off;
	int toffset_vsl_on;
	int toffset_vsl_off;
	int toffset_vgh_on;
	int toffset_vgh_off;
	int toffset_vgl_on;
	int toffset_vgl_off;
	void *hw_ref;

	int (*set_timings)(struct pl_hv_timing *p);
	int (*get_timings)(struct pl_hv_timing *p);
	int (*init)(struct pl_hv_timing *p);
	void (*delete)(struct pl_hv_timing *p);
} pl_hv_timing_t;

/**
 * interface definition for a group of all required HV related interfaces
 */
typedef struct pl_hv{
	struct pl_vcom_switch *vcomSwitch;
	struct pl_vcom_config *vcomConfig;
	struct pl_vcom_driver *vcomDriver;
	struct pl_hv_config   *hvConfig;
	struct pl_hv_driver   *hvDriver;
	struct pl_hv_timing   *hvTiming;

	void (*delete)(struct pl_hv *p);
	int (*init)(struct pl_hv *p);
} pl_hv_t;

struct pl_hv *hv_new();
pl_hv_driver_t *hv_driver_new();
pl_hv_config_t *hv_config_new();
pl_hv_timing_t *hv_timing_new();
pl_vcom_config_t *vcom_config_new();
pl_vcom_driver_t *vcom_driver_new();
pl_vcom_switch_t *vcom_switch_new();
#endif /* HV_H_ */
