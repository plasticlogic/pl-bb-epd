/*
 * config_defs.h
 *
 *  Created on: 06.10.2014
 *      Author: sebastian.friebe
 */

#ifndef CONFIG_DEFS_H_
#define CONFIG_DEFS_H_

#include "pl/types.h"

enum epd_controller_type {
	S1D13524,
	S1D13541,
	SSD1606,
};

enum control_system {
	BEAGLEBONE_WHITE,
	BEAGLEBONE_BLACK,
};

enum driving_board {
	RUDDOCK,
	HBZ5,
	HBZ1_3,
	HBZ6_3,	
	FALCON_SERIAL,
	FALCON_PARALLEL,
	RUDDOCK_PARALLEL,
};

enum nvm_version {
	NVM_NULL,
	NVM_MICROCHIP_24AA256,
	NVM_MICROCHIP_24LC014H,
};

enum vcom_switch_choises {
	VCOM_SWITCH_NULL,
	VCOM_SWITCH_GPIO,
	VCOM_SWITCH_TPS65185,
};

enum vcom_driver_choises {
	VCOM_DRIVER_NULL,
	VCOM_DRIVER_MAX17135,
	VCOM_DRIVER_MAX8520,
	VCOM_DRIVER_TPS65185,
};

enum vcom_config_choises {
	VCOM_CONFIG_NULL,
	VCOM_CONFIG_MAX17135,
	VCOM_CONFIG_MAX8520,
	VCOM_CONFIG_TPS65185,
};

enum hv_driver_choises {
	HV_DRIVER_NULL,
	HV_DRIVER_MAX17135,
	HV_DRIVER_TPS65185,
};

enum hv_config_choises {
	HV_CONFIG_NULL,
	HV_CONFIG_MAX17135,
	HV_CONFIG_TPS65185,
};

enum hv_timing_choises {
	HV_TIMING_NULL,
	HV_TIMING_MAX17135,
	HV_TIMING_TPS65185,
};

extern regSetting_t *epdc_register_defaults;
extern int epdc_register_defaults_count;

#endif /* CONFIG_DEFS_H_ */
