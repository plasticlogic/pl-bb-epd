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
 * it8951_nvm.c
 *
 *  Created on: 28.10.2020
 *      Author: oliver.lenz
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ite/it8951_nvm.h>
#include <pl/assert.h>

#define LOG_TAG "it8951-nvm"
#include <pl/utils.h>

static int nvm_it8951_read(struct pl_nvm * nvm, unsigned int addr,
		uint8_t * blob, int len);
static int wait_for_ack(struct it8951 * p, uint16_t status, uint16_t mask);

void itoa(int n, char s[]);
void reverse(char s[]);

it8951_nvm_t * it8951_nvm_new(pl_generic_controller_t *p, pl_spi_hrdy_t *spi,
		pl_hv_driver_t *pl_hv_driver, pl_hv_timing_t *pl_hv_timing) {
	it8951_nvm_t *nvm = (it8951_nvm_t *) malloc(sizeof(it8951_nvm_t));

	nvm->it8951 = p;
	nvm->spi = spi;
	nvm->pl_hv_driver = pl_hv_driver;
	nvm->pl_hv_timing = pl_hv_timing;

	return nvm;
}

pl_nvm_t *it8951_get_nvm(it8951_t * p, pl_spi_hrdy_t *spi,
		pl_hv_driver_t *pl_hv_driver, pl_hv_timing_t *pl_hv_timing) {
	assert(p != NULL);

	pl_nvm_t *nvm = pl_nvm_new();
	it8951_nvm_t *hw_ref = it8951_nvm_new(p, i2c, pl_hv_driver, pl_hv_timing);

	nvm->hw_ref = hw_ref;
	nvm->read = nvm_it8951_read;
//	nvm->pgm = nvm_s1d13541_pgm;

	nvm->size = 0x08;

	return nvm;
}

static int nvm_it8951_read(struct pl_nvm * nvm, unsigned int addr,
		uint8_t * blob, int len) {

	it8951_nvm_t * it8951_nvm = nvm->hw_ref;
	pl_generic_controller_t *p = it8951_nvm->it8951;
	int i = 0, j = 0;
	uint16_t data = 0;
	uint16_t addr_ = 0;
	regSetting_t regData;
	regData.valCount = 1;
	uint32_t bitmask = 0xFFFFFFFF;

	// wait for status: idle
	if (wait_for_ack(p, IT8951_PROM_STATUS_IDLE, 0xffff))
		return -ETIME;

	for (i = 0; i < len; i++) {
		for (j = 0; j < 2; j++) {
			// set read address
			addr_ = ((i * 2 + j) << 8) & 0x0f00;
			regData.addr = addr_;
			regData.val = IT8951_PROM_ADR_PGR_DATA;
//			p->write_register(p, IT8951_PROM_ADR_PGR_DATA, addr_);
			p->write_register(p, regData, bitmask);

			// set read operation start trigger
			//p->write_register(p, IT8951_PROM_CTRL, IT8951_PROM_READ_START);
			regData.addr = IT8951_PROM_CTRL;
			regData.val = IT8951_PROM_READ_START;
			p->write_register(p, regData, bitmask);

			//wait for status: read mode start
			if (wait_for_ack(p, IT8951_PROM_STATUS_READ_MODE,
			IT8951_PROM_STATUS_READ_MODE))
				return -ETIME;

			//wait for status: read operation finished
			if (wait_for_ack(p, 0x0000, IT8951_PROM_STATUS_READ_BUSY))
				return -ETIME;

			// set read operation start trigger
			data = p->read_register(p, IT8951_PROM_READ_DATA);
			if (j)
				blob[i] |= data & 0x0f;
			else
				blob[i] = data << 4 & 0xf0;

		}
	}

	// set read mode stop trigger
	regData.addr = IT8951_PROM_CTRL;
	regData.val = IT8951_PROM_READ_STOP;
	p->write_register(p, regData, bitmask);

	//wait for status: read mode stop
	if (wait_for_ack(p, 0x0000, IT8951_PROM_STATUS_READ_MODE))
		return -ETIME;

	return 0;
}

