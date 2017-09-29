/*
 * nvm-i2c-24LC014H.c
 *
 *  Created on: 12.12.2014
 *      Author: matti.haugwitz
 */

#include <stdio.h>
#include <unistd.h>
#include <hardware/nvm-i2c-24LC014H.h>
#include <pl/assert.h>

static int nvm_24LC014H_i2c_read (struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len);
//remove static int nvm_24LC014H_i2c_pgm (struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len);

int nvm_24LC014H_i2c_init (struct pl_nvm * nvm, struct pl_i2c *i2c){
	nvm->hw_ref = i2c;
	//remove nvm->pgm = &nvm_24LC014H_i2c_pgm;
	nvm->read = &nvm_24LC014H_i2c_read;
	nvm->size = SIZE_24LC014H;

	return 0;
}
/*//remove
static int nvm_24LC014H_i2c_pgm (struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len){

	assert(blob);
	struct pl_i2c *i2c = (struct pl_i2c *)nvm->hw_ref;

	int byte_offset;
	int stat = -1;
	uint8_t tmp;
	uint8_t cmp_blob[len];
	int i;

	//write blob to nvm, try it 5 times
	for(byte_offset=0; byte_offset<len; byte_offset++){
		for(i=1; i<= 5; i++)
		{
			stat = pl_i2c_reg_write_8(i2c, (uint8_t) I2C_24LC014H, (uint8_t) (addr + byte_offset), (uint8_t) blob[byte_offset]);
			usleep(1000);
			if (stat == 0)
				break;
		}
	}

	//read back from nvm
	for(byte_offset=0; byte_offset<len; byte_offset++){
		stat = pl_i2c_reg_read_8(i2c, (uint8_t) I2C_24LC014H, (uint8_t) (addr + byte_offset), &tmp);
		cmp_blob[byte_offset] = tmp;
	}

	//blob verification
	for(byte_offset=0; byte_offset<len; byte_offset++){
		if(blob[byte_offset]!=cmp_blob[byte_offset]){
			printf("Binary verification from nvm-i2c-24LC014H failed\n");
			return -1;
		}
	}


  return stat;
}
//*/

static int nvm_24LC014H_i2c_read (struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len){

	assert(blob);
	struct pl_i2c *i2c = (struct pl_i2c *)nvm->hw_ref;

	int byte_offset;
	int stat = -1;
	uint8_t tmp;

	//read back from nvm
	for(byte_offset=0; byte_offset<len; byte_offset++){
		stat = pl_i2c_reg_read_8(i2c, (uint8_t) I2C_24LC014H, (uint8_t) (addr + byte_offset), &tmp);
		blob[byte_offset] = tmp;
	}

  return stat;
}
