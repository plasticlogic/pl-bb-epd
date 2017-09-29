/*
 * beaglebone-parallel.h
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#ifndef BEAGLEBONE_PARALLEL_H_
#define BEAGLEBONE_PARALLEL_H_

#include <stddef.h>
#include <stdint.h>
#include <pl/global.h>
#include <pl/parallel.h>


struct pl_gpio;

pl_parallel_t *beaglebone_parallel_new(struct pl_gpio * hw_ref);

#endif /* BEAGLEBONE_PARALLEL_H_ */
