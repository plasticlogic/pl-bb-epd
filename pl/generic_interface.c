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
 * generic_interface.c
 *
 *  Created on: 02.11.2016
 *      Author: robert.pohlink
 */

#include <pl/generic_interface.h>

struct pl_gpio;

pl_generic_interface_t* interface_new(uint8_t spi_channel, struct pl_gpio* hw_ref, uint8_t serial){
	//pl_generic_interface_t* interface = malloc(sizeof(pl_generic_interface_t));
	if(serial){
		return (pl_generic_interface_t*) beaglebone_spi_new(spi_channel, hw_ref);
	}else{
		return (pl_generic_interface_t*) beaglebone_parallel_new(hw_ref);
	}
	return NULL;
}
