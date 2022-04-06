/*
 * it8951_i2c.h
 *
 *  Created on: 31 Mar 2021
 *      Author: matti.haugwitz
 */

#ifndef IT8951_I2C_H_
#define IT8951_I2C_H_

#include <stdint.h>
#include <errno.h>
#include <pl/i2c.h>
#include <ite/it8951.h>
#include "ite/it8951_controller.h"


extern int it8951_i2c_init(struct pl_generic_controller *controller, struct pl_i2c *i2c);

#endif /* IT8951_I2C_H_ */
