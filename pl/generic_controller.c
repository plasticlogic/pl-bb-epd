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
 * generic_controller.c
 *
 *  Created on: 23.03.2015
 *      Author: sebastian.friebe
 */

#include <stdlib.h>
#include <string.h>
#include <pl/generic_controller.h>
#define LOG_TAG "generic_controller"
#include <pl/utils.h>
#include <pl/assert.h>

static void generic_controller_delete(struct pl_generic_controller *p);

/**
 * allocates memory to hold a pl_generic_controller structure
 *
 * @return pointer to allocated memory
 */
struct pl_generic_controller *generic_controller_new(){
	struct pl_generic_controller *p = (struct pl_generic_controller *)malloc(sizeof(struct pl_generic_controller));;

	p->delete = generic_controller_delete;

	return p;
}

/**
 * maps the general waveform id/name to the controller specific waveform id
 *
 * @param p is a pl_generic_controller structure
 * @param wf_path general waveform id or name
 * @return controller specific waveform id
 */
int pl_generic_controller_get_wfid(pl_generic_controller_t *p, const char *wf_path)
{
	const struct pl_wfid *wfid;

	assert(p != NULL);
	assert(wf_path != NULL);

	/* Optimised string comparison first */
	for (wfid = p->wf_table; wfid->path != NULL; ++wfid)
		if (wfid->path == wf_path)
			return wfid->id;

	/* Full string compare now */
	for (wfid = p->wf_table; wfid->path != NULL; ++wfid)
		if (!strcmp(wfid->path, wf_path))
			return wfid->id;

	return -EINVAL;
}

// -----------------------------------------------------------------------------
// private functions
// ------------------------------
/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void generic_controller_delete(struct pl_generic_controller *p){

	if (p != NULL){
		free(p);
		p = NULL;
	}
}
