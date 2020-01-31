/*
 * beaglebone_i80.h
 *
 *  Created on: 24.01.2020
 *      Author: matti.haugwitz
 */

#ifndef BEAGLEBONE_I80_H_
#define BEAGLEBONE_I80_H_

#include <stddef.h>
#include <stdint.h>
#include <pl/global.h>
#include <pl/i80.h>
#include <pl/parallel.h>
#include <errno.h>

#define WAIT_FOR_READY_TIMEOUT_I80 10000

struct pl_gpio;

pl_parallel_t *beaglebone_i80_new(struct pl_gpio * hw_ref);

#endif /* BEAGLEBONE_I80_H_ */
