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
 * config_defs.h
 *
 *  Created on: 06.10.2014
 *      Author: sebastian.friebe
 */

#ifndef CONFIG_DEFS_H_
#define CONFIG_DEFS_H_

#include "pl/types.h"
#include <errno.h>

enum epd_controller_type {
	S1D13524,
	S1D13541,
	SSD1606,
	IT8951,
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
	FALCON_I80,
	FALCON_SPI,
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
	VCOM_CONFIG_IT8951,
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
