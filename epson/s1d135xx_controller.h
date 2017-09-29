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

int s1d13524_controller_setup(pl_generic_controller_t *p, s1d135xx_t *s1d135xx);
int s1d13541_controller_setup(pl_generic_controller_t *p, s1d135xx_t *s1d135xx);


#endif /* S1D13524_CONTROLLER_H_ */
