#ifndef SPI_HRDY_H_
#define SPI_HRDY_H_

struct spi_hrdy_metadata {
	uint8_t channel;  // SPI channel number
	uint8_t mode;     // current SPI mode
	uint8_t bpw;      // current SPI bits per word setting
	uint32_t msh;     // current SPI max speed setting in Hz
};

typedef struct pl_spi_hrdy
{
  void *hw_ref;		// hardware reference

  //parallel interface
  int fd;           // open file descriptor: /dev/spi-X.Y

  // control signal interface
  int cs_gpio; 		// chip select gpio
  int hrdy_gpio;
  int interfaceType;

  int (*open)(struct pl_spi_hrdy * p);
  int (*close)(struct pl_spi_hrdy * p);
  int (*read_bytes)(struct pl_spi_hrdy *p, uint8_t *buff, size_t size);
  int (*write_bytes)(struct pl_spi_hrdy *p, uint8_t *buff, size_t size);
  int (*set_cs)(struct pl_spi_hrdy *p, uint8_t cs);
  void (*delete)(struct pl_spi_hrdy *p);

  struct spi_hrdy_metadata *mSpi;

  /*uint8_t channel;  // SPI channel number
  uint8_t mode;     // current SPI mode
  uint8_t bpw;      // current SPI bits per word setting
  uint32_t msh;     // current SPI max speed setting in Hz*/
} pl_spi_hrdy_t;

#endif /* SPI_H_ */
