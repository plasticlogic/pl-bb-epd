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
 * s1d13524_controller.h
 *
 *  Created on: 25.03.2015
 *      Author: sebastian.friebe
 */

#ifndef S1D13524_CONTROLLER_H_
#define S1D13524_CONTROLLER_H_

#include <stdlib.h>
#include <pl/generic_controller.h>
#include "epson-s1d135xx.h"
#include <errno.h>
int s1d13524_controller_setup(pl_generic_controller_t *p, s1d135xx_t *s1d135xx);
int s1d13541_controller_setup(pl_generic_controller_t *p, s1d135xx_t *s1d135xx);


#endif /* S1D13524_CONTROLLER_H_ */
