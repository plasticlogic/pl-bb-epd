/*
 * beaglebone-hv.h
 *
 *  Created on: 16 Apr 2015
 *      Author: matti.haugwitz
 */

#ifndef BEAGLEBONE_HV_H_
#define BEAGLEBONE_HV_H_

#include <pl/hv.h>
#include <pl/gpio.h>


/**
 * creates a pl_hv_driver object based on the beaglebone gpios.
 *
 * @param gpio pointer to a pl_gpio structure
 * @return pointer to a pl_hv_driver interface
 */
pl_hv_driver_t *beaglebone_get_hv_driver(struct pl_gpio *gpio);


/**
 * creates a pl_vcom_config object based on the beaglebone gpios.
 *
 * @param gpio is pointer to a pl_gpio structure
 * @return pointer to a pl_vcom_switch interface
 */
pl_vcom_switch_t *beaglebone_get_vcom_switch(struct pl_gpio *gpio);


#endif /* BEAGLEBONE_HV_H_ */
