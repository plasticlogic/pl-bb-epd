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
 * beaglebone_parallel.c
 *
 *  Created on: 23.07.2014
 *      Author: sebastian.friebe, matti.haugwitz, robert.pohlink
 */

#include <beaglebone/beaglebone_parallel.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <pindef.h>

// function prototypes
static int parallel_close(struct pl_parallel *psParallel);
static int parallel_read_bytes(struct pl_parallel *psParallel, uint8_t *buff, size_t size);
static int parallel_write_bytes(struct pl_parallel *psParallel, uint8_t *buff, size_t size);
static int parallel_set_cs(struct pl_parallel *psParallel, uint8_t cs);
static int parallel_init(struct pl_parallel *psParallel);
static void delete(struct pl_parallel *p);

/**
 * allocates, configures and returns a new pl_spi structure
 *
 * @param spi_channel number of the spi device /dev/spidev?.0
 * @return pl_spi structure
 */
pl_parallel_t *beaglebone_parallel_new(struct pl_gpio * hw_ref){
	pl_parallel_t *p = (pl_parallel_t *)malloc(sizeof(pl_parallel_t));
	p->mSPI = NULL;
	p->hw_ref = hw_ref;
	p->open = parallel_init;
	p->close = parallel_close;
	p->read_bytes = parallel_read_bytes;
	p->write_bytes = parallel_write_bytes;
	p->set_cs = parallel_set_cs;
	p->delete = delete;

	return p;
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
static int parallel_init(struct pl_parallel *psParallel)
{

	psParallel->fd = open("/dev/parallel", O_RDWR);
	if(psParallel->fd < 0){
		return FALSE;
	}

	return TRUE;
}

/**
 * closes the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int parallel_close(struct pl_parallel *psParallel){

	if(psParallel->fd)
		close(psParallel->fd);
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
static int parallel_read_bytes(struct pl_parallel *psParallel, uint8_t *buff, size_t size){

	  int iResult;

	  iResult = read(psParallel->fd, buff, size/2);

	  return iResult;
}

/**
 * write bytes from buffer to the spi bus
 *
 * @param psSPI pl_spi structure
 * @param buff pointer to the buffer
 * @param size size of the buffer
 * @return status
 */
static int parallel_write_bytes(struct pl_parallel *psParallel, uint8_t *buff, size_t size){

	  int iResult;

	  iResult = write(psParallel->fd, buff, size/2);

	  return iResult;
}

/**
 * closes the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int parallel_set_cs(struct pl_parallel *psParallel, uint8_t cs){

	struct pl_gpio * gpio = (struct pl_gpio *) psParallel->hw_ref;
	gpio->set(psParallel->cs_gpio, cs);

	return 0;
}
