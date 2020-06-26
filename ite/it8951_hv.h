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

pl_vcom_config_t *it8951_get_vcom_config(it8951_t *it8951);

#endif /* IT8951_HV_H_ */