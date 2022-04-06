/*
  Plastic Logic EPD project on BeagleBone

  Copyright (C) 2018 Plastic Logic

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
#include <errno.h>

pl_pmic_t *tps65185_new(struct pl_i2c *i2c, uint8_t i2c_addr);

#endif /* INCLUDE_PMIC_TPS65185_H */
