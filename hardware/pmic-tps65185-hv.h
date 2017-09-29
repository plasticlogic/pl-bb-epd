/*
 * pmic-tps65185-hv.h
 *
 *  Created on: 16.04.2015
 *      Author: sebastian.friebe
 */

#ifndef PMIC_TPS65185_HV_H_
#define PMIC_TPS65185_HV_H_

#include <hardware/pmic-tps65185.h>
#include <pl/hv.h>

/**
 * creates a pl_hv_driver object based on the Texas Instruments pmic TPS65185.
 *
 * @param tps65185 pointer to a pmic HW implementation
 * @return pointer to a pl_hv_driver interface for tps65185
 * @see pl_hv_driver
 */
pl_hv_driver_t *tps65185_get_hv_driver(pl_pmic_t *tps65185);

/**
 * creates a pl_hv_timing object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_hv_timing interface for tps65185
 * @see pl_hv_timing
 */
pl_hv_timing_t *tps65185_get_hv_timing(pl_pmic_t *tps65185);

/**
 * creates a pl_hv_config object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_hv_config interface for tps65185
 * @see pl_hv_config
 */
pl_hv_config_t *tps65185_get_hv_config(pl_pmic_t *tps65185);

/**
 * creates a pl_vcom_driver object based on the  Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_vcom_driver interface for tps65185
 * @see pl_vcom_driver
 */
pl_vcom_driver_t *tps65185_get_vcom_driver(pl_pmic_t *tps65185);

/**
 * creates a pl_vcom_config object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_vcom_config interface for tps65185
 * @see pl_vcom_config
 */
pl_vcom_config_t *tps65185_get_vcom_config(pl_pmic_t *tps65185);

/**
 * creates a pl_vcom_switch object based on the Texas Instruments pmic TPS65185
 *
 * @param tps65185 pointer to a tps65185 variable
 * @return pointer to a pl_vcom_switch interface for tps65185
 * @see pl_vcom_switch
 */
pl_vcom_switch_t *tps65185_get_vcom_switch(pl_pmic_t *tps65185);
#endif /* PMIC_TPS65185_HV_H_ */
