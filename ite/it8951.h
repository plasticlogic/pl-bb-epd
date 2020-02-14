/*
 * it8951.h
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#ifndef IT8951_H_
#define IT8951_H_

#include <pl/types.h>
#include <pl/gpio.h>
#include <pl/generic_interface.h>
#include <pl/i2c.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

struct it8951_pins {
	unsigned reset_n;
	unsigned busy_n;
	unsigned hirq;
	unsigned spi_cs;
	unsigned vcc_en;
} it8951_pins_t;

typedef struct it8951 {
	struct pl_gpio *gpio;
	struct pl_spi *spi;
	struct pl_generic_interface *interface;
	const struct it8951_pins *pins;

} it8951_t;

it8951_t *it8951_new(struct pl_gpio *gpios, struct pl_generic_interface *interface, struct pl_i2c *i2c, const struct it8951_pins *pins);

#endif /* IT8951_H_ */
