/**
 * pmic-tps65185.h -- Driver for TI TPS65185 PMIC
 *
 * Authors:
 *  Sebastian Friebe <sebastian.friebe@plasticlogic.com>
 *  Nick Terry <nick.terry@plasticlogic.com>
 *  Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PMIC_TPS65185_H
#define INCLUDE_PMIC_TPS65185_H 1

#include <stdint.h>
#include <pl/pmic.h>

pl_pmic_t *tps65185_new(struct pl_i2c *i2c, uint8_t i2c_addr);

#endif /* INCLUDE_PMIC_TPS65185_H */
