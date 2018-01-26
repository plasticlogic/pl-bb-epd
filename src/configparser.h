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
