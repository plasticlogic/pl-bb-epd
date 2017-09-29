/*
 * generic_interface.h
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#ifndef GENERIC_INTERFACE_H_
#define GENERIC_INTERFACE_H_

#include <beaglebone/beaglebone-spi.h>
#include <beaglebone/beaglebone-parallel.h>

enum interfaceType{
	PARALLEL = 0,
	SPI,
};

typedef struct pl_generic_interface {
	void *hw_ref;		// hardware reference
	int fd;           // open file descriptor: /dev/spi-X.Y
	int cs_gpio; 		// chip select gpio

	int (*open)(struct pl_generic_interface * p);
	int (*close)(struct pl_generic_interface * p);
	int (*read_bytes)(struct pl_generic_interface *p, uint8_t *buff, size_t size);
	int (*write_bytes)(struct pl_generic_interface *p, uint8_t *buff, size_t size);
	int (*set_cs)(struct pl_generic_interface *p, uint8_t cs);
	void (*delete)(struct pl_generic_interface *p);

	struct spi_metadata *mSpi;
}pl_generic_interface_t;

pl_generic_interface_t* interface_new(uint8_t spi_channel, struct pl_gpio* hw_ref, uint8_t serial);

#endif /* GENERIC_INTERFACE_H_ */
