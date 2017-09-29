/*
 * beaglebone-gpio.h
 *
 *  Created on: 21.07.2014
 *      Author: sebastian.friebe, matti.haugwitz
 */

#ifndef BEAGLEBONE_GPIO_H_
#define BEAGLEBONE_GPIO_H_

#include "pl/gpio.h"

#define BEAGLEBONE_GPIO(_port, _pin) ((_port) << 5 | _pin)

struct pl_gpio;

extern int beaglebone_gpio_init(struct pl_gpio *gpio);

#endif /* BEAGLEBONE_GPIO_H_ */
