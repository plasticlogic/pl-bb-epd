/*
 * display.h
 *
 *  Created on: 10.03.2015
 *      Author: sebastian.friebe
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <pl/nvm.h>

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
