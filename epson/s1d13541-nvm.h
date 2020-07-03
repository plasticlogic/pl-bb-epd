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
 * s1d13541-nvm.h
 *
 *  Created on: 26 Apr 2016
 *      Author: matti.haugwitz
 */

#ifndef S1D13541_NVM_H_
#define S1D13541_NVM_H_

#include "pl/nvm.h"
#include "pl/hv.h"
#include "pl/pmic.h"
#include <epson/epson-s1d135xx.h>
#include <pl/i2c.h>
#include <errno.h>
#define S1D13541_PROM_STATUS		0x0500
#define S1D13541_PROM_CTRL			0x0502
#define S1D13541_PROM_ADR_PGR_DATA	0x0504
#define S1D13541_PROM_READ_DATA		0x0506

#define S1D13541_PROM_STATUS_IDLE 			0x0
#define S1D13541_PROM_STATUS_READ_BUSY 		(1 << 8)
#define S1D13541_PROM_STATUS_PGM_BUSY 		(1 << 9)
#define S1D13541_PROM_STATUS_ERASE_BUSY 	(1 << 10)
#define S1D13541_PROM_STATUS_READ_MODE 		(1 << 12)
#define S1D13541_PROM_STATUS_PGM_MODE 		(1 << 13)
#define S1D13541_PROM_STATUS_ERASE_ALL_MODE	(1 << 14)

#define S1D13541_PROM_READ_START 			(1 << 0)
#define S1D13541_PROM_READ_STOP 			(1 << 1)
#define S1D13541_PROM_VCOM_READ 			(1 << 2)
#define S1D13541_PROM_PGM_MODE_START 		(1 << 4)
#define S1D13541_PROM_PGM_OP_START 			(1 << 5)
#define S1D13541_PROM_PGM_OP_STOP 			(1 << 6)
#define S1D13541_PROM_PGM_MODE_STOP 		(1 << 7)
#define S1D13541_PROM_ERASE_ALL_MODE_START 	(1 << 8)
#define S1D13541_PROM_ERASE_ALL_OP_START 	(1 << 9)
#define S1D13541_PROM_ERASE_ALL_OP_STOP 	(1 << 10)
#define S1D13541_PROM_ERASE_ALL_MODE_STOP 	(1 << 11)

// PCA9536DGKR I2C GPIO to control VME1 and VME2
#define HV_GPIO_EXPANDER_I2C_ADDR 		0x41
#define HV_GPIO_EXPANDER_OUTPUT_REG 	0x01
#define HV_GPIO_EXPANDER_CTRL_REG 		0x03
#define HV_GPIO_EXPANDER_ALL_PORTS_IN	0xff
#define HV_GPIO_EXPANDER_ALL_PORTS_OUT	0x00
#define HV_GPIO_EXPANDER_VME1_ON		0x01
#define HV_GPIO_EXPANDER_VME2_ON		0x02

#define HV_DIGIPOT_I2C_ADDR 			0x2b


pl_nvm_t *s1d13541_get_nvm(s1d135xx_t * p, pl_i2c_t * i2c, pl_hv_driver_t *pl_hv_driver, pl_hv_timing_t *pl_hv_timing);
int s1d13541_generate_eeprom_blob(struct pl_nvm *nvm, uint8_t *data);
int s1d13541_extract_eeprom_blob(struct pl_nvm *nvm, uint8_t *data);


typedef struct s1d13541_nvm {

	s1d135xx_t *s1d13541;
	pl_i2c_t *i2c;
	pl_hv_driver_t *pl_hv_driver;
	pl_hv_timing_t *pl_hv_timing;

} s1d13541_nvm_t;

#endif /* S1D13541_NVM_H_ */
