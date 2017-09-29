/*
 * spi.h
 *
 *  Created on: 28.04.2015
 *      Author: sebastian.friebe
 */

#ifndef SPI_H_
#define SPI_H_

struct spi_metadata {
	uint8_t channel;  // SPI channel number
	uint8_t mode;     // current SPI mode
	uint8_t bpw;      // current SPI bits per word setting
	uint32_t msh;     // current SPI max speed setting in Hz
};

typedef struct pl_spi
{
  void *hw_ref;		// hardware reference
  int fd;           // open file descriptor: /dev/spi-X.Y
  int cs_gpio; 		// chip select gpio

  int (*open)(struct pl_spi * p);
  int (*close)(struct pl_spi * p);
  int (*read_bytes)(struct pl_spi *p, uint8_t *buff, size_t size);
  int (*write_bytes)(struct pl_spi *p, uint8_t *buff, size_t size);
  int (*set_cs)(struct pl_spi *p, uint8_t cs);
  void (*delete)(struct pl_spi *p);

  struct spi_metadata *mSpi;
  /*uint8_t channel;  // SPI channel number
  uint8_t mode;     // current SPI mode
  uint8_t bpw;      // current SPI bits per word setting
  uint32_t msh;     // current SPI max speed setting in Hz*/
} pl_spi_t;

#endif /* SPI_H_ */
