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
/*
 * beaglebone-parallel.h
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#ifndef BEAGLEBONE_PARALLEL_H_
#define BEAGLEBONE_PARALLEL_H_

#include <stddef.h>
#include <stdint.h>
#include <pl/global.h>
#include <pl/parallel.h>
#include <errno.h>

struct pl_gpio;

pl_parallel_t *beaglebone_parallel_new(struct pl_gpio * hw_ref);

#endif /* BEAGLEBONE_PARALLEL_H_ */
