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
 * display.h
 *
 *  Created on: 10.03.2015
 *      Author: sebastian.friebe
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <pl/nvm.h>
#include <errno.h>

typedef struct pl_display{
	uint32_t gate_lines;
	uint32_t source_lines;
	float ppi;
	struct pl_nvm *nvm;

	void (*delete)(struct pl_display *p);

} pl_display_t;

/**
 * allocates memory to hold a pl_display structure
 *
 * @return pointer to allocated memory
 */
pl_display_t *pl_display_new();

#endif /* DISPLAY_H_ */
