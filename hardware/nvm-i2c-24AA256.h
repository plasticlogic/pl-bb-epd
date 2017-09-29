/*
 * nvm-i2c-24AA256.h
 *
 *  Created on: 12.12.2014
 *      Author: matti.haugwitz
 */

#include <pl/nvm.h>
#include <pl/i2c.h>

#define I2C_24AA256		0x54
#define SIZE_24AA256	0x8000

int nvm_24AA256_i2c_init(struct pl_nvm * nvm, struct pl_i2c *i2c);




