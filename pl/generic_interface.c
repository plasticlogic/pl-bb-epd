/*
 * generic_interface.c
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#include <pl/generic_interface.h>

struct pl_gpio;

pl_generic_interface_t* interface_new(uint8_t spi_channel, struct pl_gpio* hw_ref, uint8_t serial){
	pl_generic_interface_t* interface = malloc(sizeof(pl_generic_interface_t));
	if(serial){
		return (pl_generic_interface_t*) beaglebone_spi_new(spi_channel, hw_ref);
	}else{
		return (pl_generic_interface_t*) beaglebone_parallel_new(hw_ref);
	}
	return NULL;
}
