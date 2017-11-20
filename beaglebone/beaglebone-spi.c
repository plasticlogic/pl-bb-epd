/*
 * beaglebone_spi.c
 *
 *  Created on: 23.07.2014
 *      Author: sebastian.friebe, matti.haugwitz
 */

#include <beaglebone/beaglebone-spi.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <src/pindef.h>

#define VERBOSE 0

// function prototypes
static int spi_close(struct pl_spi *psSPI);
static int spi_read_bytes(struct pl_spi *psSPI, uint8_t *buff, size_t size);
static int spi_write_bytes(struct pl_spi *psSPI, uint8_t *buff, size_t size);
static int spi_set_cs(struct pl_spi *psSPI, uint8_t cs);
static int spi_init(pl_spi_t *psSPI);
static void delete(pl_spi_t *p);

int fd, serial = 1;

/**
 * allocates, configures and returns a new pl_spi structure
 *
 * @param spi_channel number of the spi device /dev/spidev?.0
 * @return pl_spi structure
 */
pl_spi_t *beaglebone_spi_new(uint8_t spi_channel, struct pl_gpio * hw_ref){
	pl_spi_t *p = (pl_spi_t *)malloc(sizeof(pl_spi_t));
	struct spi_metadata *meta = malloc(sizeof(struct spi_metadata));
	p->mSpi = meta;
	p->hw_ref = hw_ref;
	p->mSpi->channel = spi_channel;
	p->open = spi_init;
	p->close = spi_close;
	p->read_bytes = spi_read_bytes;
	p->write_bytes = spi_write_bytes;
	p->set_cs = spi_set_cs;
	p->delete = delete;

	return p;
}

/**
 * free the memory of a pl_spi structure
 *
 * @param p pl_spi structure
 */
static void delete(pl_spi_t *p){
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
static int spi_init(pl_spi_t *psSPI)
{

	uint8_t tmp8;
	uint32_t tmp32;
	int bufferSize = 100;
	char userlandSpiDevice[bufferSize];

	snprintf(userlandSpiDevice, bufferSize, "/dev/spidev%d.0", psSPI->mSpi->channel);
	if ( ( psSPI->fd = open(userlandSpiDevice , O_RDWR, 0 ) ) == -1 )
  {
		char errorStr[bufferSize];
		snprintf(errorStr, bufferSize, "Failed to open userland spi device (%s)\n", userlandSpiDevice);
		fprintf( stderr,  errorStr);
		return FALSE;
	}

	if ( ioctl( psSPI->fd, SPI_IOC_RD_MODE, &tmp8 ) == -1 )
  {
		fprintf( stderr, "Failed to get SPI_IOC_RD_MODE\n" );
		return FALSE;
	}
	psSPI->mSpi->mode = tmp8;

	if ( ioctl( psSPI->fd, SPI_IOC_RD_BITS_PER_WORD, &tmp8 ) == -1 )
  {
		fprintf( stderr, "Failed to get SPI_IOC_RD_BITS_PER_WORD\n" );
		return FALSE;
	}
	psSPI->mSpi->bpw = tmp8;

	if ( ioctl( psSPI->fd, SPI_IOC_RD_MAX_SPEED_HZ, &tmp32 ) == -1)
  {
		fprintf( stderr, "Failed to get SPI_IOC_RD_MAX_SPEED_HZ\n" );
		return FALSE;
	}
	psSPI->mSpi->msh = tmp32;

	return TRUE;
}

/**
 * closes the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int spi_close(struct pl_spi *psSPI){

	if ( ( psSPI->fd != -1 ) &&
	   ( close( psSPI->fd ) == -1 ) )
	{
		fprintf( stderr, "Failed to close SPI device, or not open\n" );
		return FALSE;
	}

	psSPI->mSpi->channel = -1;
	psSPI->fd = -1;
	psSPI->mSpi->mode = 0;
	psSPI->mSpi->bpw = 0;
	psSPI->mSpi->msh = 0;
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
static int spi_read_bytes(struct pl_spi *psSPI, uint8_t *buff, size_t size){

	  int iResult;

	  int i;
#if VERBOSE
	  int s=size;
#endif
	  // enough transfer buffers for 64 * 64 bytes or 4K
	  struct spi_ioc_transfer asTrans[ MAX_SPI_TRANSFER_BUFFERS ];
	  struct spi_ioc_transfer *psTrans;
	  uint8_t *rxBuffer = buff;
	  bool boLast;

	  memset( &asTrans, 0, sizeof( asTrans ) );

	  // fill in the array of transfer buffers, limiting each one to transferring
	  // MAX_SPI_PER_TRANSFER bytes.
	  i = 0;
	  while ( ( i < MAX_SPI_TRANSFER_BUFFERS ) && ( size > 0 ) )
	  {
		boLast = ( size < MAX_SPI_BYTES_PER_TRANSFER );
		psTrans = &asTrans[ i ];
		psTrans->tx_buf = (unsigned long)NULL;
		psTrans->rx_buf = (unsigned long)rxBuffer;
		// length is the number of bytes in the buffer, so for 9-bit mode it is
		// 2 bytes per word.
		psTrans->len = boLast ? size : MAX_SPI_BYTES_PER_TRANSFER;
		psTrans->delay_usecs = 20;
		//psSPI->msh = 12000000;
		psTrans->speed_hz = psSPI->mSpi->msh; //SPI_TRANSFER_RATE_IN_HZ;
		psTrans->bits_per_word =  psSPI->mSpi->bpw; //SPI_BITS_PER_WORD;
		psTrans->cs_change = boLast;

		size -= psTrans->len;
		rxBuffer += psTrans->len;
		++i;
	  }
	  iResult = ioctl( psSPI->fd, SPI_IOC_MESSAGE( i ), asTrans );
	  if ( iResult < 1 )
	  {
		fprintf( stderr, "Can't write SPI transaction in %d parts (%d)\n", i, iResult );
		return FALSE;
	  }
#if VERBOSE
	  int tmp = rxBuffer;
	  printf("spi\tread ");
	  for(i=0;i<s;i++){
		  printf("0x%02X ", *buff++);
	  }
	  printf("\n");
	  rxBuffer=tmp;
#endif
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
static int spi_write_bytes(struct pl_spi *psSPI, uint8_t *buff, size_t size){

	  int iResult;
#if VERBOSE
	  iResult = buff;
#endif
	  int i;
	  // enough transfer buffers for 64 * 64 bytes or 4K
	  struct spi_ioc_transfer asTrans[ MAX_SPI_TRANSFER_BUFFERS ];
	  struct spi_ioc_transfer *psTrans;
	  uint8_t *pbBuffer = buff;
	  bool boLast;
#if VERBOSE
	  printf("spi\twrite ");
	  for(i=0;i<size;i++){
		  printf("0x%02X ", *buff++);
	  }
	  printf("\n");
	  buff = iResult;
	  iResult = 0;
#endif
	  memset( &asTrans, 0, sizeof( asTrans ) );

	  // fill in the array of transfer buffers, limiting each one to transferring
	  // MAX_SPI_PER_TRANSFER bytes.
	  i = 0;
	  while ( ( i < MAX_SPI_TRANSFER_BUFFERS ) && ( size > 0 ) )
	  {
		boLast = ( size < MAX_SPI_BYTES_PER_TRANSFER );
		psTrans = &asTrans[ i ];
		psTrans->tx_buf = (unsigned long)pbBuffer;
		psTrans->rx_buf = (unsigned long)NULL;
		// length is the number of bytes in the buffer, so for 9-bit mode it is
		// 2 bytes per word.
		psTrans->len = boLast ? size : MAX_SPI_BYTES_PER_TRANSFER;
		psTrans->delay_usecs = 0;
		psTrans->speed_hz = psSPI->mSpi->msh; //SPI_TRANSFER_RATE_IN_HZ;
		psTrans->bits_per_word =  psSPI->mSpi->bpw; //SPI_BITS_PER_WORD;
		psTrans->cs_change = boLast;

		size -= psTrans->len;
		pbBuffer += psTrans->len;
		++i;
	  }

	  iResult = ioctl( psSPI->fd, SPI_IOC_MESSAGE( i ), asTrans );
	  if ( iResult < 1 )
	  {
		fprintf( stderr, "Can't write SPI transaction in %d parts (%d)\n", i, iResult );
		return -1;
	  }
	  return iResult;
}

/**
 * closes the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int spi_set_cs(struct pl_spi *psSPI, uint8_t cs){

	struct pl_gpio * gpio = (struct pl_gpio *) psSPI->hw_ref;
	gpio->set(psSPI->cs_gpio, cs);
	return 0;
}
