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
 *      Author: robert.pohlink, matti.haugwitz
 */

#include <pl/generic_interface.h>
#include <beaglebone/beaglebone-i80.h>
#include <beaglebone/beaglebone-spi_hrdy.h>
#include <pl/i80.h>
#include <pl/spi_hrdy.h>
#include <src/pindef.h>

struct pl_gpio;

pl_generic_interface_t* interface_new(uint8_t spi_channel, struct pl_gpio* p_gpio, enum interfaceType type){
	pl_generic_interface_t* interface;
	interface = NULL;

	if(type == SPI){
		interface = (pl_generic_interface_t*) beaglebone_spi_new(spi_channel, p_gpio);
	}
	else if(type == PARALLEL)
	{
		interface = (pl_generic_interface_t*) beaglebone_parallel_new(p_gpio);
	}
	else if(type == I80)
	{
		interface = (pl_generic_interface_t*) beaglebone_i80_new(p_gpio);
		pl_i80_t* i80_ref = (pl_i80_t*) interface->hw_ref;
		i80_ref->hwe_n_gpio =  NULL; //FALCON_I80_HWE_N;
		i80_ref->hrd_n_gpio =  NULL; //FALCON_I80_HRD_N;
		i80_ref->hcs_n_gpio =  FALCON_I80_HCS_N;
		i80_ref->hdc_gpio =  FALCON_I80_HDC;
		i80_ref->hrdy_gpio =  FALCON_I80_HRDY;
	}
	else if(type == SPI_HRDY)
	{
		interface = (pl_generic_interface_t*) beaglebone_spi_hrdy_new(spi_channel, p_gpio);
		pl_spi_hrdy_t* spi_ref = (pl_spi_hrdy_t*) interface->hw_ref;
		interface->hrdy_gpio = FALCON_I80_HRDY;
		interface->cs_gpio = FALCON_SPI_CS_ITE;
	}

	return interface;
}
