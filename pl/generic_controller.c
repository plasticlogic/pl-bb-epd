/*
 * generic_controller.c
 *
 *  Created on: 23.03.2015
 *      Author: sebastian.friebe
 */

#include <stdlib.h>
#include <string.h>
#include <pl/generic_controller.h>
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

	return -1;
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
