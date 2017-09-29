/*
 * nvm-spi-MX25U4033E.h
 *
 *  Created on: 27 Jul 2015
 *      Author: matti.haugwitz
 */

#ifndef NVM_SPI_MX25U4033E_H_
#define NVM_SPI_MX25U4033E_H_


#include <pl/nvm.h>
#include <pl/spi.h>

// Size
#define MX25U4033E_SIZE		(0x80000)	// max size

// Register Address
#define MX25U4033E_PP		(0x02)	// page program
#define MX25U4033E_READ		(0x03)	// read
#define MX25U4033E_RDSR		(0x05)  // read status register
#define MX25U4033E_WREN		(0x06)	// write enable
#define MX25U4033E_SE		(0x20)	// sector erase
#define MX25U4033E_CE		(0x60)	// chip erase
#define MX25U4033E_RDID		(0x9f)	// read identification

// Status Register Bits
#define MX25U4033E_STATUS_WIP	(1	   ) // write in progress
#define MX25U4033E_STATUS_WEL	(1 << 1) // write enable latch

int nvm_MX25U4033E_spi_init(struct pl_nvm *nvm, struct pl_spi *spi);

#endif /* NVM_SPI_MX25U4033E_H_ */
