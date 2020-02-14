/*
 * it8951.h
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#ifndef IT8951_H_
#define IT8951_H_

#include <stdint.h>

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
	const struct it8951_pins *pins;

} it8951_t;

#endif /* IT8951_H_ */
