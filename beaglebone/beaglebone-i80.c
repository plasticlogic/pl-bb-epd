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
#include <beaglebone/beaglebone-parallel.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <src/pindef.h>

// function prototypes
static int i80_close(struct pl_parallel *p);
static int i80_read_bytes(struct pl_parallel *p, uint8_t *buff, size_t size);
static int i80_write_bytes(struct pl_parallel *p, uint8_t *buff, size_t size);
static int i80_set_cs(struct pl_parallel *p, uint8_t cs);
static int i80_init(struct pl_parallel *p);
static void delete(struct pl_parallel *p);
static int wait_for_ready(struct pl_i80 *p);
static void swap_data(uint8_t *buff, size_t size);

/**
 * allocates, configures and returns a new pl_spi structure
 *
 * @param spi_channel number of the spi device /dev/spidev?.0
 * @return pl_spi structure
 */
pl_parallel_t *beaglebone_i80_new(struct pl_gpio * hw_ref){
	pl_parallel_t *parellel_ref = (pl_parallel_t *)malloc(sizeof(pl_parallel_t));
	pl_i80_t *i80_ref = (pl_i80_t *)malloc(sizeof(pl_i80_t));

	i80_ref->hw_ref = hw_ref;
	parellel_ref->hw_ref = i80_ref;

	parellel_ref->open = i80_init;
	parellel_ref->close = i80_close;
	parellel_ref->read_bytes = i80_read_bytes;
	parellel_ref->write_bytes = i80_write_bytes;
	parellel_ref->set_cs = i80_set_cs;
	parellel_ref->delete = delete;

	return parellel_ref;
}

/**
 * free the memory of a pl_spi structure
 *
 * @param p pl_spi structure
 */
static void delete(struct pl_parallel *p){

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
static int i80_init(struct pl_parallel *p)
{
	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	i80_ref->fd = open("/dev/parallel", O_RDWR);
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
static int i80_close(struct pl_parallel *p)
{
	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	if(i80_ref->fd)
		close(i80_ref->fd);
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
static int i80_read_bytes(struct pl_parallel *p, uint8_t *buff, size_t size){

	int iResult = 0;

	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

	if(wait_for_ready(i80_ref))
	  return -1;

	//Switch C/D to Data - DATA - H
	//done outside

	//CS-L
	gpio->set(i80_ref->hcs_n_gpio, 0);

	//RD Enable -L
	gpio->set(i80_ref->hrd_n_gpio, 0);

	//Get 8-bits Bus Data (Collect 8 GPIO pins to Byte Data)
	iResult = read(i80_ref->fd, buff, size/2);

	//WR Disable -H
	gpio->set(i80_ref->hrd_n_gpio, 1);

	//CS-H
	gpio->set(i80_ref->hcs_n_gpio, 1);

	// necessary due to temporary reverse connection of parallel bus
	swap_data(buff, size);

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
static int i80_write_bytes(struct pl_parallel *p, uint8_t *buff, size_t size)
{
	int iResult = 0;

	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

	if(wait_for_ready(i80_ref))
	  return -1;

	// necessary due to temporary reverse connection of parallel bus
	swap_data(buff, size);

	//Switch C/D --> command = L / data = H
	//done outside

	//CS-L
	gpio->set(i80_ref->hcs_n_gpio, 0);

	//WR Enable -L
	gpio->set(i80_ref->hwe_n_gpio, 0);

	//Set Data output (Parallel output request)
	iResult = write(i80_ref->fd, buff, size/2);

	//WR Disable-H
	gpio->set(i80_ref->hwe_n_gpio, 1);

	//CS-H
	gpio->set(i80_ref->hcs_n_gpio, 1);

	return iResult;
}

/**
 * set chip select gpio
 *
 * @param p pl_i80 structure
 * @return status
 */
static int i80_set_cs(struct pl_parallel *p, uint8_t cs)
{
	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

	gpio->set(i80_ref->hcs_n_gpio, cs);

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

static void swap_data(uint8_t *buff, size_t size)
{
	int i = 0;
	uint8_t tmp[2];

	for(i=0; i< size/2; i++)
	{
		tmp[0] = buff[i*2];
		tmp[1] = buff[i*2+1];

		buff[i*2] = 		      tmp[1] >> 7 & 0x01;
		buff[i*2] = buff[i*2] | ( tmp[1] >> 5 & 0x02 );
		buff[i*2] = buff[i*2] | ( tmp[1] >> 3 & 0x04 );
		buff[i*2] = buff[i*2] | ( tmp[1] >> 1 & 0x08 );
		buff[i*2] = buff[i*2] | ( tmp[1] << 1 & 0x10 );
		buff[i*2] = buff[i*2] | ( tmp[1] << 3 & 0x20 );
		buff[i*2] = buff[i*2] | ( tmp[1] << 5 & 0x40 );
		buff[i*2] = buff[i*2] | ( tmp[1] << 7 & 0x80 );

		buff[i*2+1] = 		      tmp[0] >> 7 & 0x01;
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] >> 5 & 0x02 );
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] >> 3 & 0x04 );
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] >> 1 & 0x08 );
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] << 1 & 0x10 );
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] << 3 & 0x20 );
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] << 5 & 0x40 );
		buff[i*2+1] = buff[i*2+1] | ( tmp[0] << 7 & 0x80 );

		printf("swap: 0x%x | 0x%x --> 0x%x | 0x%x\n", tmp[1], tmp[0], buff[i*2+1], buff[i*2]);
	}
}
