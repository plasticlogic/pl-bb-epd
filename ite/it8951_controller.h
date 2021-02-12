/*
  Plastic Logic EPD project on BeagleBone

  Copyright (C) 2020 Plastic Logic

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
 * it8951_controller.h
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#ifndef IT8951_CONTROLLER_H_
#define IT8951_CONTROLLER_H_

#include <ite/it8951.h>
#include <pl/generic_controller.h>


int it8951_controller_setup(struct pl_generic_controller *p, it8951_t *it8951);



#endif /* IT8951_CONTROLLER_H_ */
