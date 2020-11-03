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
 * it8951_nvm.h
 *
 *  Created on: 28.10.2020
 *      Author: oliver.lenz
 */

#ifndef IT8951_NVM_H_
#define IT8951_NVM_H_

#include "pl/nvm.h"
#include "pl/hv.h"
#include "pl/pmic.h"
#include <ite/it8951.h>
#include <ite/it8951_controller.h>
#include <pl/spi_hrdy.h>
#include <errno.h>

#define IT8951_PROM_STATUS			0x0500
#define IT8951_PROM_CTRL			0x0502
#define IT8951_PROM_ADR_PGR_DATA	0x0504
#define IT8951_PROM_READ_DATA		0x0506

#define IT8951_PROM_STATUS_IDLE 			 0x0
#define IT8951_PROM_STATUS_READ_BUSY 		(1 << 8)
#define IT8951_PROM_STATUS_PGM_BUSY 		(1 << 9)
#define IT8951_PROM_STATUS_ERASE_BUSY 		(1 << 10)
#define IT8951_PROM_STATUS_READ_MODE 		(1 << 12)
#define IT8951_PROM_STATUS_PGM_MODE 		(1 << 13)
#define IT8951_PROM_STATUS_ERASE_ALL_MODE	(1 << 14)

#define IT8951_PROM_READ_START 				(1 << 0)
#define IT8951_PROM_READ_STOP 				(1 << 1)
#define IT8951_PROM_VCOM_READ 				(1 << 2)
#define IT8951_PROM_PGM_MODE_START 			(1 << 4)
#define IT8951_PROM_PGM_OP_START 			(1 << 5)
#define IT8951_PROM_PGM_OP_STOP 			(1 << 6)
#define IT8951_PROM_PGM_MODE_STOP 			(1 << 7)
#define IT8951_PROM_ERASE_ALL_MODE_START 	(1 << 8)
#define IT8951_PROM_ERASE_ALL_OP_START 		(1 << 9)
#define IT8951_PROM_ERASE_ALL_OP_STOP 		(1 << 10)
#define IT8951_PROM_ERASE_ALL_MODE_STOP 	(1 << 11)

pl_nvm_t *it8951_get_nvm(it8951_t * p, pl_spi_hrdy_t *spi,
		pl_hv_driver_t *pl_hv_driver, pl_hv_timing_t *pl_hv_timing);
int it8951_generate_eeprom_blob(struct pl_nvm *nvm, uint8_t *data);
int it8951_extract_eeprom_blob(struct pl_nvm *nvm, uint8_t *data);

typedef struct it8951_nvm {

	pl_generic_controller_t *it8951;
	pl_spi_hrdy_t *spi;
	pl_hv_driver_t *pl_hv_driver;
	pl_hv_timing_t *pl_hv_timing;

} it8951_nvm_t;

#endif /* IT8951_NVM_H_ */
