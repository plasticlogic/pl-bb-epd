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
 * display.c
 *
 *  Created on: 24.03.2015
 *      Author: sebastian.friebe
 */


#include <stdlib.h>
#include <pl/display.h>

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void delete(pl_display_t *p);


// -----------------------------------------------------------------------------
// constructor
// ------------------------------
pl_display_t *pl_display_new(){
	pl_display_t *p = (pl_display_t *)malloc(sizeof(pl_display_t));;

	p->delete = delete;

	return p;
}

// -----------------------------------------------------------------------------
// private functions
// ------------------------------
static void delete(pl_display_t *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}
