/*
 * dac-max5820-hv.h
 *
 *  Created on: 14.04.2015
 *      Author: matti.haugwitz
 */


#ifndef DAC_MAX5820_HV_H_
#define DAC_MAX5820_HV_H_

#include <hardware/dac-max5820.h>
#include <pl/hv.h>


/**
 * creates a pl_vcom_config object based on the maxim dac max5820.
 *
 * @param dac_max5820 pointer to a dac_max5820 variable
 * @return pointer to a pl_hv_config interface for dac_max5820
 * @see pl_vcom_config
 */
pl_vcom_config_t *dac_max5820_get_vcom_config(dac_max5820_t *dac_max5820);


#endif /* DAC_MAX5820_HV_H_ */
