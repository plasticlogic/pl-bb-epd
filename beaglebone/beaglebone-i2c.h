/*
 * beaglebone-i2c.h
 *
 *      Authors:
 *           Nick Terry <nick.terry@plasticlogic.com>
 *   		 Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *   		 sebastian.friebe
 *   		 matti.haugwitz
 */

#ifndef BEAGLEBONE_I2C_H
#define BEAGLEBONE_I2C_H 1

#include <stdint.h>

struct pl_gpio;
struct pl_i2c;

extern int beaglebone_i2c_init(uint8_t channel, struct pl_i2c *i2c);


#endif /* BEAGLEBONE_I2C_H */
