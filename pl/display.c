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
