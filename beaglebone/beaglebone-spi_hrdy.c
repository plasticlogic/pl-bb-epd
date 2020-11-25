#include <beaglebone/beaglebone-spi_hrdy.h>
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
static int spi_hrdy_close(struct pl_spi_hrdy *psSPI);
static TWord spi_hrdy_read_bytes(struct pl_spi_hrdy *psSPI, TWord *buff,
		size_t size);
static int spi_hrdy_write_bytes(struct pl_spi_hrdy *psSPI, uint8_t *buff,
		size_t size);
static int spi_hrdy_set_cs(struct pl_spi_hrdy *psSPI, uint8_t cs);
static int spi_hrdy_init(pl_spi_hrdy_t *psSPI);
static void delete_spi_hrdy(pl_spi_hrdy_t *p);

int fd;
//int serial = 1;

/**
 * allocates, configures and returns a new pl_spi structure
 *
 * @param spi_channel number of the spi device /dev/spidev?.0
 * @return pl_spi structure
 */
pl_spi_hrdy_t *beaglebone_spi_hrdy_new(uint8_t spi_channel,
		struct pl_gpio* hw_ref) {
	pl_spi_hrdy_t *p = (pl_spi_hrdy_t *) malloc(sizeof(pl_spi_hrdy_t));

	struct p *meta = malloc(sizeof(struct spi_hrdy_metadata));
	p->mSpi = meta;

	p->hw_ref = hw_ref;
	p->mSpi->channel = spi_channel;
	p->fd = -1;

	p->open = spi_hrdy_init;
	p->close = spi_hrdy_close;
	p->read_bytes = spi_hrdy_read_bytes;
	p->write_bytes = spi_hrdy_write_bytes;
	p->set_cs = spi_hrdy_set_cs;
	p->delete = delete_spi_hrdy;

	return p;
}

/**
 * free the memory of a pl_spi structure
 *
 * @param p pl_spi structure
 */
static void delete_spi_hrdy(pl_spi_hrdy_t *p) {
	if (p != NULL) {
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
static int spi_hrdy_init(pl_spi_hrdy_t *psSPI) {

	//printf("into spi_hrdy_init\n");

	uint8_t tmp8;
	uint32_t tmp32;
	int bufferSize = 100;
	char userlandSpiDevice[bufferSize];
	//psSPI->fd = 0;

	snprintf(userlandSpiDevice, bufferSize, "/dev/spidev%d.0",
			psSPI->mSpi->channel);

	int cnt = 0;
	if(psSPI->fd <= 0)
	{
		while ((psSPI->fd = open(userlandSpiDevice, O_RDWR, 0)) == -1) {
			cnt++;
			printf("Try to open spi %d\n", cnt);
			usleep(100000);

			if (cnt >= 100)
				break;
		}
	}
	//printf("got fd %d\n", psSPI->fd);

	if (psSPI->fd == -1) {
		char errorStr[bufferSize];
		snprintf(errorStr, bufferSize,
				"Failed to open userland spi device (%s)\n", userlandSpiDevice);
		fprintf( stderr, errorStr);
		return FALSE;
	}
	//	if ( ioctl( psSPI->fd, SPI_IOC_RD_MODE, &tmp8 ) == -1 )
//  {
//		fprintf( stderr, "Failed to get SPI_IOC_RD_MODE\n" );
//		return FALSE;
//	}
//	psSPI->mSpi->mode = tmp8;
//	psSPI->mSpi->mode = SPI_HRDY_TRANSFER_MODE;

	if (ioctl(psSPI->fd, SPI_IOC_RD_BITS_PER_WORD, &tmp8) == -1) {
		fprintf( stderr, "Failed to get SPI_IOC_RD_BITS_PER_WORD\n");
		return FALSE;
	}
	psSPI->mSpi->bpw = tmp8;

//	if ( ioctl( psSPI->fd, SPI_hrdy_TRANSFER_RATE_IN_HZ, &tmp32 ) == -1)
//  {
//		fprintf( stderr, "Failed to get SPI_IOC_RD_MAX_SPEED_HZ\n" );
//		return FALSE;
//	}
//	psSPI->mSpi->msh = tmp32;
	//psSPI->mSpi->msh = 16000000; //SPI_HRDY_TRANSFER_RATE_IN_HZ;
	psSPI->mSpi->msh = 12000000; //SPI_HRDY_TRANSFER_RATE_IN_HZ;

	return TRUE;
}

/**
 * closes the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int spi_hrdy_close(struct pl_spi_hrdy *psSPI) {

	if ((psSPI->fd != -1) && (close(psSPI->fd) == -1)) {
		fprintf( stderr, "Failed to close SPI device, or not open\n");
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
 * Wait For the HRDY Pin to go high and signal the Ready State
 */
int wait_for_ready(struct pl_spi_hrdy *p) {
	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;
	int i = 0;

	while (i++ < WAIT_FOR_READY_TIMEOUT_SPI_HRDY) {
		if (gpio->get(p->hrdy_gpio) == 1) {
			return 0;
		}
	}
	return -1;
}

/**
 * reads bytes from spi bus into a buffer
 *
 * @param psSPI pl_spi structure
 * @param buff pointer to the buffer
 * @param size size of the buffer
 * @return status
 */
static TWord spi_hrdy_read_bytes(struct pl_spi_hrdy *psSPI, TWord *buff,
		size_t size) {

	TWord iResult;

	int i;
#if VERBOSE
	int s=size;
#endif
	// enough transfer buffers for 64 * 64 bytes or 4K
	pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) psSPI->hw_ref;
	struct spi_ioc_transfer asTrans[MAX_SPI_TRANSFER_BUFFERS_hrdy];
	struct spi_ioc_transfer *psTrans;
	TWord *rxBuffer = buff;
	bool boLast;

	memset(&asTrans, 0, sizeof(asTrans));

	// fill in the array of transfer buffers, limiting each one to transferring
	// MAX_SPI_PER_TRANSFER bytes.
	i = 0;
	while ((i < MAX_SPI_TRANSFER_BUFFERS_hrdy) && (size > 0)) {
		boLast = (size < MAX_SPI_BYTES_PER_TRANSFER_hrdy);
		psTrans = &asTrans[i];
		psTrans->tx_buf = (unsigned long) NULL;
		psTrans->rx_buf = (unsigned long) rxBuffer;
		// length is the number of bytes in the buffer, so for 9-bit mode it is
		// 2 bytes per word.
		psTrans->len = boLast ? size : MAX_SPI_BYTES_PER_TRANSFER_hrdy;
		psTrans->delay_usecs = 20;
		//psSPI->msh = 12000000;
		psTrans->speed_hz = psSPI->mSpi->msh; //SPI_TRANSFER_RATE_IN_HZ;
		psTrans->bits_per_word = psSPI->mSpi->bpw; //SPI_BITS_PER_WORD;
		psTrans->cs_change = boLast;

		size -= psTrans->len;
		rxBuffer += psTrans->len;
		++i;
	}
	iResult = ioctl(psSPI->fd, SPI_IOC_MESSAGE(i), asTrans);
	if (iResult < 1) {
		fprintf( stderr, "Can't write SPI transaction in %d parts (%d)\n", i,
				iResult);
		return FALSE;
	}
#if VERBOSE
	int tmp = rxBuffer;
	printf("spi\tread ");
	for(i=0;i<s;i++) {
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
static int spi_hrdy_write_bytes(struct pl_spi_hrdy *psSPI, uint8_t *buff,
		size_t size) {

	int iResult;
#if VERBOSE
	iResult = buff;
#endif
	int i;
	// enough transfer buffers for 64 * 64 bytes or 4K
	pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) psSPI;
	struct spi_ioc_transfer asTrans[MAX_SPI_TRANSFER_BUFFERS_hrdy];
	struct spi_ioc_transfer *psTrans;
	uint8_t *pbBuffer = buff;
	bool boLast;
#if VERBOSE
	printf("spi\twrite ");
	for(i=0;i<size;i++) {
		printf("0x%02X ", *buff++);
	}
	printf("\n");
	buff = iResult;
	iResult = 0;
#endif
	memset(&asTrans, 0, sizeof(asTrans));

	// fill in the array of transfer buffers, limiting each one to transferring
	// MAX_SPI_PER_TRANSFER bytes.
	i = 0;
	while ((i < MAX_SPI_TRANSFER_BUFFERS_hrdy) && (size > 0)) {
		wait_for_ready(spi);
		boLast = (size < MAX_SPI_BYTES_PER_TRANSFER_hrdy);
		psTrans = &asTrans[i];
		psTrans->tx_buf = (unsigned long) pbBuffer;
		psTrans->rx_buf = (unsigned long) NULL;
		// length is the number of bytes in the buffer, so for 9-bit mode it is
		// 2 bytes per word.
		psTrans->len = boLast ? size : MAX_SPI_BYTES_PER_TRANSFER_hrdy;
		psTrans->delay_usecs = 0;
		psTrans->speed_hz = psSPI->mSpi->msh; //SPI_TRANSFER_RATE_IN_HZ;
		psTrans->bits_per_word = psSPI->mSpi->bpw; //SPI_BITS_PER_WORD;
		psTrans->cs_change = boLast;

		size -= psTrans->len;
		pbBuffer += psTrans->len;
		++i;
	}

	iResult = ioctl(psSPI->fd, SPI_IOC_MESSAGE(i), asTrans);
	if (iResult < 1) {
		fprintf( stderr, "Can't write SPI transaction in %d parts (%d)\n", i,
				iResult);
		return -EIO;
	}
	return iResult;
}

/**
 * closes the spi bus
 *
 * @param psSPI pl_spi structure
 * @return status
 */
static int spi_hrdy_set_cs(struct pl_spi_hrdy *psSPI, uint8_t cs) {

	struct pl_gpio * gpio = (struct pl_gpio *) psSPI->hw_ref;
	gpio->set(psSPI->cs_gpio, cs);
	return 0;
}

