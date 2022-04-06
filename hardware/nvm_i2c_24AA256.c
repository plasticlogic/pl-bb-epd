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
 * nvm-i2c-24AA256.c
 *
 *  Created on: 12.12.2014
 *      Author: matti.haugwitz
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hardware/nvm-i2c-24AA256.h>
#include <pl/assert.h>

#define LOG_TAG "nvm_24AA256"
#include <pl/utils.h>

//remove static int nvm_24AA256_i2c_pgm(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len);
static int nvm_24AA256_i2c_read(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len);

int nvm_24AA256_i2c_init(struct pl_nvm * nvm, struct pl_i2c *i2c){
	nvm->hw_ref = i2c;
	nvm->read = &nvm_24AA256_i2c_read;
	//remove nvm->pgm = &nvm_24AA256_i2c_pgm;
	nvm->size = SIZE_24AA256;
	return 0;
}
/*//remove
static int nvm_24AA256_i2c_pgm(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len){

	assert(blob);

	int stat = -1;
	uint16_t register_address = addr;
	int chunkSize = 64;
	uint16_t byte_offset = 0;
	int bytes_to_transfer = len;
	uint8_t *data;
	uint8_t pgm_blob[chunkSize+2];
	uint8_t cmp_blob[chunkSize];

	struct pl_i2c *i2c = (struct pl_i2c *)nvm->hw_ref;

	while(bytes_to_transfer > 0){
		size_t transferChunkSize = (bytes_to_transfer >= chunkSize) ? chunkSize : bytes_to_transfer;

		uint8_t reg[2] = {(register_address >> 8) & 0xff, (uint8_t) register_address};

		data = &(blob[byte_offset]);
		pgm_blob[0] = (register_address >> 8) & 0xFF;
		pgm_blob[1] = register_address & 0xFF;
		memcpy(&pgm_blob[2], data, transferChunkSize);

		// set address in EEPROM
		stat = i2c->write(i2c, (uint8_t) I2C_24AA256, pgm_blob, transferChunkSize+2, 0);
		usleep(5000);

		stat = i2c->write(i2c, (uint8_t) I2C_24AA256, reg, 2, PL_I2C_NO_STOP);
		stat = i2c->read(i2c, (uint8_t) I2C_24AA256, cmp_blob, transferChunkSize, 0 );

		if (memcmp(data, cmp_blob, transferChunkSize) != 0){
			LOG("Verify after pgm failed...");
			return -1;
		}

		byte_offset += transferChunkSize;
		register_address += transferChunkSize;
		bytes_to_transfer -= transferChunkSize;
	}

	return stat;

}
//*/

static int nvm_24AA256_i2c_read(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len){

	assert(blob);
	struct pl_i2c *i2c = (struct pl_i2c *)nvm->hw_ref;

	int stat = -1;
	uint16_t register_address = addr;
	int chunkSize = 64;
	uint16_t byte_offset = 0;
	int bytes_to_transfer = len;
	uint8_t cmp_blob[chunkSize];

	while(bytes_to_transfer > 0){
		size_t transferChunkSize = (bytes_to_transfer >= chunkSize) ? chunkSize : bytes_to_transfer;
		uint8_t reg[2] = {(register_address >> 8) & 0xff, (uint8_t) register_address};

		stat = i2c->write(i2c, (uint8_t) I2C_24AA256, reg, 2, PL_I2C_NO_STOP);
		stat = i2c->read(i2c, (uint8_t) I2C_24AA256, cmp_blob, transferChunkSize, 0 );

		memcpy(&blob[byte_offset], cmp_blob, transferChunkSize);

		byte_offset += transferChunkSize;
		register_address += transferChunkSize;
		bytes_to_transfer -= transferChunkSize;
	}

  return stat;
}
