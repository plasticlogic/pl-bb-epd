/*
 * it8951.c
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#include <stdlib.h>
#include <unistd.h>
#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#include <ite/it8951.h>
#define LOG_TAG "it8951"
#include "pl/utils.h"
#include <pl/spi.h>


it8951_t *it8951_new(struct pl_gpio *gpios, struct pl_generic_interface *interface, struct pl_i2c *i2c, const struct it8951_pins *pins)
{
	assert(gpios != NULL);
	assert(pins != NULL);
	assert(interface != NULL);

	it8951_t *p = (it8951_t *)malloc(sizeof(it8951_t));
	p->gpio = gpios;
	p->interface =  interface;
	p->pins =  pins;

	return p;
}
