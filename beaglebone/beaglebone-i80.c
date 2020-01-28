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
 * beaglebone_i80.c
 *
 *  Created on: 24.01.2020
 *      Author: matti.haugwitz
 */

#include <beaglebone/beaglebone-i80.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <src/pindef.h>

// function prototypes
static int i80_close(struct pl_i80 *p);
static int i80_read_bytes(struct pl_i80 *p, uint8_t *buff, size_t size);
static int i80_write_bytes(struct pl_i80 *p, uint8_t *buff, size_t size);
static int i80_set_cs(struct pl_i80 *p, uint8_t cs);
static int i80_init(struct pl_i80 *p);
static void delete(struct pl_i80 *p);
static int wait_for_ready(struct pl_i80 *p);

/**
 * allocates, configures and returns a new pl_spi structure
 *
 * @param spi_channel number of the spi device /dev/spidev?.0
 * @return pl_spi structure
 */
pl_i80_t *beaglebone_i80_new(struct pl_gpio * hw_ref){
	pl_i80_t *p = (pl_i80_t *)malloc(sizeof(pl_i80_t));

	p->hw_ref = hw_ref;
	p->open = i80_init;
	p->close = i80_close;
	p->read_bytes = i80_read_bytes;
	p->write_bytes = i80_write_bytes;
	p->set_cs = i80_set_cs;
	p->delete = delete;

	return p;
}

/**
 * free the memory of a pl_spi structure
 *
 * @param p pl_spi structure
 */
static void delete(struct pl_i80 *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * initialises the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int i80_init(struct pl_i80 *p)
{

	p->fd = open("/dev/parallel", O_RDWR);
	if(p->fd < 0){
		return FALSE;
	}

	return TRUE;
}

/**
 * closes the parallel bus
 *
 * @param p pl_i80 structure
 * @return status
 */
static int i80_close(struct pl_i80 *p){

	if(p->fd)
		close(p->fd);
	else
		return FALSE;
	return TRUE;
}

/**
 * reads bytes from spi bus into a buffer
 *
 * @param psSPI pl_spi structure
 * @param buff pointer to the buffer
 * @param size size of the buffer
 * @return status
 */
static int i80_read_bytes(struct pl_i80 *p, uint8_t *buff, size_t size){

	int iResult = 0;

	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;

	if(wait_for_ready(p))
	  return -1;

	//Switch C/D to Data - DATA - H
	//done outside

	//CS-L
	gpio->set(p->hcs_n_gpio, 0);

	//RD Enable -L
	gpio->set(p->hrd_n_gpio, 0);

	//Get 8-bits Bus Data (Collect 8 GPIO pins to Byte Data)
	iResult = read(p->fd, buff, size/2);

	//WR Disable -H
	gpio->set(p->hrd_n_gpio, 1);

	//CS-H
	gpio->set(p->hcs_n_gpio, 1);

	return iResult;
}

/**
 * write bytes from buffer to the i80 bus
 *
 * @param p pl_i80 structure
 * @param buff pointer to the buffer
 * @param size size of the buffer
 * @return status
 */
static int i80_write_bytes(struct pl_i80 *p, uint8_t *buff, size_t size)
{
	int iResult = 0;

	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;

	if(wait_for_ready(p))
	  return -1;

	//Switch C/D --> command = L / data = H
	//done outside

	//CS-L
	gpio->set(p->hcs_n_gpio, 0);

	//WR Enable -L
	gpio->set(p->hwe_n_gpio, 0);

	//Set Data output (Parallel output request)
	iResult = write(p->fd, buff, size/2);

	//WR Disable-H
	gpio->set(p->hwe_n_gpio, 1);

	//CS-H
	gpio->set(p->hcs_n_gpio, 1);

	return iResult;
}

/**
 * set chip select gpio
 *
 * @param p pl_i80 structure
 * @return status
 */
static int i80_set_cs(struct pl_i80 *p, uint8_t cs)
{
	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;
	gpio->set(p->hcs_n_gpio, cs);

	return 0;
}

/**
 *
 */
static int wait_for_ready(struct pl_i80 *p)
{
	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;
	int i = 0;

	while(i++ < WAIT_FOR_READY_TIMEOUT_I80)
	{
			if(gpio->get(p->hrdy_gpio) == 1)
			{
				return 0;
			}
	}
	return -1;
}
