/*
 * it8951.c
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#include <stdlib.h>
#include <unistd.h>
#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#define LOG_TAG "it8951"
#include "pl/utils.h"
#include <pl/spi.h>

#include "it8951.h"
