/*
 * nvm-i2c-24LC014H.h
 *
 *  Created on: 12.12.2014
 *      Author: matti.haugwitz
 */

#include <pl/nvm.h>
#include <pl/i2c.h>

#define I2C_24LC014H	0x52
#define SIZE_24LC014H	0x80

int nvm_24LC014H_i2c_init (struct pl_nvm * nvm, struct pl_i2c *i2c);
