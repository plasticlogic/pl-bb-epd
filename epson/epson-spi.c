/*
 * beaglebone_spi.c
 *
 *  Created on: 23.07.2014
 *      Authors: sebastian.friebe, matti.haugwitz
 */

#include <epson/epson-spi.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

// function prototypes
static int spi_open(struct pl_spi *psSPI);
static int spi_close(struct pl_spi *psSPI);
static int spi_chip_select(struct pl_spi *psSPI, uint8_t cs);
static int spi_read_bytes(struct pl_spi *psSPI, uint8_t *buff, size_t size);
static int spi_write_bytes(struct pl_spi *psSPI, uint8_t *buff, size_t size);

static void delete(struct pl_spi *p);


/**
 * allocates, configures and returns a new pl_spi structure based on epson spi host interface
 *
 * @param c s1d135xx structure
 * @return pl_spi structure
 */
pl_spi_t *epson_spi_new(s1d135xx_t *c){
	pl_spi_t *p = (pl_spi_t *)malloc(sizeof(pl_spi_t));

	p->hw_ref = c;
	p->open = spi_open;
	p->close = spi_close;
	p->read_bytes = spi_read_bytes;
	p->write_bytes = spi_write_bytes;
	p->set_cs = spi_chip_select;
	p->delete = delete;

	return p;
}

/**
 * release memory of pl_spi structure
 *
 * @param p pl_spi structure
 */
static void delete(struct pl_spi *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * spi chip select
 *
 * @param c pl_spi structure
 * @param cs set level of the chip select bin 0=low, 1=high
 * @return status
 */
static int spi_chip_select(struct pl_spi *c, uint8_t cs)
{
	s1d135xx_t *p = c->hw_ref;
	if(cs)
	{
		p->write_reg(p, 0x0208, 0x0000);	// set chip select output to high
	}
	else
	{
		p->write_reg(p, 0x0208, 0x0001);	// set chip select output to low
	}

	return 0;
}


/**
 * sets the spi configuration register 0x0204
 *
 * @param c pl_spi structure
 * @return true
 */
static int spi_open(struct pl_spi *c)
{
	s1d135xx_t *p = c->hw_ref;
	uint16_t config = 0x0000;

	// set spi clck frequency to 9:1
	config |= 0x38;

	// enable spi
	config |= 0x01;

	p->write_reg(p, 0x0204, config); // write spi configuration register

	return TRUE;
}


/**
 * does nothing yet
 *
 * @return true
 */
static int spi_close(struct pl_spi *c){

	return TRUE;
}

/**
 * read bytes from spi bus
 *
 * @param c pl_spi structure
 * @param buff pointer to data buffer
 * @param size buffer size
 *
 * @return true
 */
static int spi_read_bytes(struct pl_spi *c, uint8_t *buff, size_t size){

	s1d135xx_t *p = c->hw_ref;

	p->read_spi(p, buff, size);

	return TRUE;
}

/**
 * write bytes to spi bus
 *
 * @param c pl_spi structure
 * @param buff pointer to data buffer
 * @param size buffer size
 *
 * @return true
 */
static int spi_write_bytes(struct pl_spi *c, uint8_t *buff, size_t size){

	s1d135xx_t *p = c->hw_ref;

	p->write_spi(p, buff, size);

	return TRUE;
}
