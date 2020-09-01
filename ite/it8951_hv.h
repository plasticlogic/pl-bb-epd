/*
 * it8951_hv.h
 *
 *  Created on: 25 Jun 2020
 *      Author: matti.haugwitz
 */

#ifndef IT8951_HV_H_
#define IT8951_HV_H_

#include <pl/hv.h>
#include <ite/it8951.h>
#include <pl/vcom.h>

#define	IT8951_HVPMIC_DAC_MAX		5000
#define	IT8951_HVPMIC_DAC_MIN		0

pl_vcom_config_t *it8951_get_vcom_config(pl_pmic_t *it8951_pmic);
pl_hv_driver_t *it8951_get_hv_driver(it8951_t *it8951);
static int it8951_hv_driver_on(struct pl_hv_driver *p);
static int it8951_hv_driver_off(struct pl_hv_driver *p);
static int it8951_hv_init(pl_vcom_config_t *p);

#endif /* IT8951_HV_H_ */
