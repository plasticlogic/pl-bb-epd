/*
 * configparser.h
 *
 *  Created on: 27.04.2015
 *      Author: sebastian.friebe
 */

#ifndef CONFIGPARSER_H_
#define CONFIGPARSER_H_


#include "config_defs.h"
#include "hw_setup.h"

int parse_config(hw_setup_t *setup, const char *filename);

#endif /* CONFIGPARSER_H_ */
