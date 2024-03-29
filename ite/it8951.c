/*
 * it8951.c
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#include <ite/it8951.h>
#define LOG_TAG "it8951"
#include <pl/spi.h>
#include <pl/spi_hrdy.h>
#include <sys/time.h>

//#define SWAPDATA

static void swap_data(TWord *buff, int size);
static TWord swap_data_in(TWord in);
I80IT8951DevInfo* pstDevInfo;

it8951_t *it8951_new(struct pl_gpio *gpios,
		struct pl_generic_interface *interface,
		enum interfaceType *sInterfaceType, struct pl_i2c *i2c,
		const struct it8951_pins *pins) {
	assert(gpios != NULL);
	assert(pins != NULL);
	assert(interface != NULL);

	it8951_t *p = (it8951_t *) malloc(sizeof(it8951_t));
	p->gpio = gpios;
	p->interface = interface;
	p->pins = pins;
	p->sInterfaceType = sInterfaceType;

	return p;
}

void GetIT8951SystemInfo(pl_generic_interface_t *bus, enum interfaceType *type,
		void* pBuf) {

	I80IT8951DevInfo* pBuf_ = (I80IT8951DevInfo*) pBuf;
	//Send I80 CMD
	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_GET_DEV_INFO);
	//IT8951WaitForReady(bus, type);
	if (*type == I80) {
		pstDevInfo = (I80IT8951DevInfo*) IT8951ReadData(bus, type,
				sizeof(I80IT8951DevInfo) * 2);
	} else {
		pstDevInfo = (I80IT8951DevInfo*) IT8951ReadData(bus, type,
				sizeof(I80IT8951DevInfo));
	}

	//IT8951WaitForReady(bus, type);

	//LOG("I'm here !");

	//transfer info between pointers
	pBuf_->usPanelW = pstDevInfo->usPanelW;
	pBuf_->usPanelH = pstDevInfo->usPanelH;
	pBuf_->usImgBufAddrH = pstDevInfo->usImgBufAddrH;
	pBuf_->usImgBufAddrL = pstDevInfo->usImgBufAddrL;

//// If using One Wire only these lines are Needed
//24BE28 Update Buffer Adress
////transfer info between pointers
//	pBuf_->usPanelW = 1280;
//	pBuf_->usPanelH = 960;
//	pBuf_->usImgBufAddrH = 0x11;
//	pBuf_->usImgBufAddrL = 0xfe20;
	//pBuf_->usImgBufAddrH = 0x24;
	//pBuf_->usImgBufAddrL = 0xbe28;

//	int i = 0;
//	int size = sizeof(pstDevInfo->usFWVersion) / 2;
//	for (i = 0; i < size; i++) {
//		pBuf_->usFWVersion[i] = pstDevInfo->usFWVersion[i];
//		pBuf_->usLUTVersion[i] = pstDevInfo->usLUTVersion[i];
//	}

//Show Device information of IT8951
	printf("Panel(W,H) = (%d,%d)\n", pBuf_->usPanelW, pBuf_->usPanelH);
	printf("Image Buffer Address = %X\r\n",
			pBuf_->usImgBufAddrL | (pBuf_->usImgBufAddrH << 16));
//	printf("Panel(W,H) = (%d,%d)\n", pBuf_->usPanelW,
//			pBuf_->usPanelH);
//		printf("Image Buffer Address = %X\r\n",
//				pBuf_->usImgBufAddrL | (pBuf_->usImgBufAddrH << 16));
	//Show Firmware and LUT Version
	//printf("FW Version = %s\r\n", stI80IT8951DevInfo.usFWVersion);
	//printf("LUT Version = %s\r\n", stI80IT8951DevInfo.usLUTVersion);
}

//-----------------------------------------------------------
//Initial function 2 �V Set Image buffer base address
//-----------------------------------------------------------
void IT8951SetImgBufBaseAddr(pl_generic_interface_t *bus,
		enum interfaceType *type, TDWord ulImgBufAddr) {
	TWord usWordH = (TWord) ((ulImgBufAddr >> 16) & 0x0000FFFF);
	TWord usWordL = (TWord) (ulImgBufAddr & 0x0000FFFF);
	//Write LISAR Reg
	IT8951WriteReg(bus, type, LISAR + 2, usWordH);
	//swap16(usWordL);
	IT8951WriteReg(bus, type, LISAR, usWordL);
}

//-----------------------------------------------------------
//Host Cmd 4 - REG_RD
//-----------------------------------------------------------
TWord IT8951ReadReg(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord usRegAddr) {
	TWord*usData;
	//----------I80 Mode-------------
	//Send Cmd and Register Address
	if (*type == I80) {
		TWord buf[2];
		buf[0] = IT8951_TCON_REG_RD;
		buf[1] = usRegAddr;
		IT8951WriteDataBurst(bus, type, buf, 4);
	} else {
		IT8951WriteCmdCode(bus, type, IT8951_TCON_REG_RD);
		IT8951WriteData(bus, type, usRegAddr);

	}
	//Read data from Host Data bus
	usData = IT8951ReadData(bus, type, 4);
	return *usData;
}

//-----------------------------------------------------------
//Host Cmd 5 - REG_WR
//-----------------------------------------------------------
void IT8951WriteReg(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord usRegAddr, TWord usValue) {

	if (*type == I80) {
		TWord buf[3];
		buf[0] = IT8951_TCON_REG_WR;
		buf[1] = usRegAddr;
		buf[2] = usValue;
		IT8951WriteDataBurst(bus, type, buf, 6);
	} else {
		//Send Cmd , Register Address and Write Value
		IT8951WriteCmdCode(bus, type, IT8951_TCON_REG_WR);
		IT8951WriteData(bus, type, usRegAddr);
		IT8951WriteData(bus, type, usValue);
	}
}

/**
 * update register
 *
 * @param bus it8951 bus structure
 * @param type it8951 interface tyoe variable
 * @param reg register address
 * @param val register value
 * @param bitmask register bitmask
 * @return status
 */
int IT8951_update_reg(pl_generic_interface_t *bus, enum interfaceType *type,
		uint16_t reg, uint16_t val, const uint32_t bitmask) {

	uint16_t current_val = IT8951ReadReg(bus, type, reg);
	uint16_t new_val = (val & bitmask) | (current_val & (~bitmask));

	IT8951WriteReg(bus, type, reg, new_val);

	return 0;
}
//-----------------------------------------------------------
//Display function 1 - Wait for LUT Engine Finish
//                     Polling Display Engine Ready by LUTNo
//-----------------------------------------------------------
void IT8951WaitForDisplayReady(pl_generic_interface_t *bus,
		enum interfaceType *type) {
	//Check IT8951 Register LUTAFSR => NonZero �V Busy, 0 - Free
	while (IT8951ReadReg(bus, type, DISPLAY_REG_BASE + 0x224))
		;
}

//-----------------------------------------------------------
//Display function 2 �V Load Image Area process
//-----------------------------------------------------------
void IT8951HostAreaPackedPixelWrite(pl_generic_interface_t *bus,
		enum interfaceType *type, IT8951LdImgInfo* pstLdImgInfo,
		IT8951AreaImgInfo* pstAreaImgInfo) {
	TDWord i, j;
	struct timeval tStop, tStart; // time variables
	float tTotal;

	gettimeofday(&tStart, NULL);

	//Source buffer address of Host
	uint8_t* pusFrameBuf = (uint8_t*) pstLdImgInfo->ulStartFBAddr;

	//Set Image buffer(IT8951) Base address

	IT8951SetImgBufBaseAddr(bus, type, pstLdImgInfo->ulImgBufBaseAddr);

	if (pstAreaImgInfo->usWidth > 2048) {

		pstAreaImgInfo->usWidth = pstAreaImgInfo->usWidth / 2;
		uint8_t *tempBuf = malloc(
				pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);
		memset(tempBuf, 0xFF,
				pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);
//
		int x, y = 0;
		for (y = 0; y < pstAreaImgInfo->usHeight; y++)
			for (x = 0; x < pstAreaImgInfo->usWidth; x++) {
				{
					tempBuf[x + pstAreaImgInfo->usWidth * y] = pusFrameBuf[x
							+ pstAreaImgInfo->usWidth * 2 * y];

				}
			}

		if (*type == SPI_HRDY) {
			struct pl_gpio * gpio = (struct pl_gpio *) bus->hw_ref;
			int b = 0;
			IT8951LoadImgAreaStart(bus, type, pstLdImgInfo, pstAreaImgInfo);
			for (b = 0; b < pstAreaImgInfo->usHeight; b++) {
				IT8951WriteDataBurst(bus, type, (TWord*) tempBuf,
						pstAreaImgInfo->usWidth / 2);
				tempBuf += pstAreaImgInfo->usWidth;
				j = b;
			}
		} else if (*type == I80) {
			int i, t = 0;
			TWord *usArg = malloc(
					(pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight + 12)
							* sizeof(TWord));
			//Setting Argument for Load image start
			usArg[0] = IT8951_TCON_LD_IMG_AREA;
			usArg[1] = (pstLdImgInfo->usEndianType << 8)
					| (pstLdImgInfo->usPixelFormat << 4)
					| (pstLdImgInfo->usRotate);
			usArg[2] = pstAreaImgInfo->usX;
			usArg[3] = pstAreaImgInfo->usY;
			usArg[4] = pstAreaImgInfo->usWidth;
			usArg[5] = pstAreaImgInfo->usHeight;

			for (i = 0;
					i < (pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);
					i++) {
				((TByte*) usArg)[12 + i] = tempBuf[i];
			}
			IT8951WriteDataBurst(bus, type, usArg,
					12 + pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);

//			IT8951WriteDataBurst(bus, type, (TWord*) tempBuf,
//					pstAreaImgInfo->usWidth / 2 * pstAreaImgInfo->usHeight);
			j = pstAreaImgInfo->usHeight;

			if (usArg)
				free(usArg);
		}
		IT8951LoadImgEnd(bus, type);

		//Second image Part
		pstAreaImgInfo->usX = pstAreaImgInfo->usWidth;
		uint8_t* tempBuf2 = malloc(
				pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);
		memset(tempBuf2, 0xFF,
				pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);

		for (y = 0; y < pstAreaImgInfo->usHeight; ++y) {
			for (x = 1280; x < pstAreaImgInfo->usWidth * 2; x++) {
				tempBuf2[x + pstAreaImgInfo->usWidth * y] = pusFrameBuf[y
						* (pstAreaImgInfo->usWidth * 2) + x];
			}
		}

		if (*type == SPI_HRDY) {
			struct pl_gpio * gpio = (struct pl_gpio *) bus->hw_ref;
			int b = 0;
			IT8951LoadImgAreaStart(bus, type, pstLdImgInfo, pstAreaImgInfo);
			for (b = 0; b < pstAreaImgInfo->usHeight; b++) {
				IT8951WriteDataBurst(bus, type, (TWord*) tempBuf2,
						pstAreaImgInfo->usWidth / 2);
				tempBuf2 += pstAreaImgInfo->usWidth;
				j = b;
			}
		} else if (*type == I80) {

			TWord *usArg = malloc(
					(pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight + 12)
							* sizeof(TWord));
			//Setting Argument for Load image start
			usArg[0] = IT8951_TCON_LD_IMG_AREA;
			usArg[1] = (pstLdImgInfo->usEndianType << 8)
					| (pstLdImgInfo->usPixelFormat << 4)
					| (pstLdImgInfo->usRotate);
			usArg[2] = pstAreaImgInfo->usX;
			usArg[3] = pstAreaImgInfo->usY;
			usArg[4] = pstAreaImgInfo->usWidth;
			usArg[5] = pstAreaImgInfo->usHeight;

			for (i = 0;
					i < (pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);
					i++) {
				((TByte*) usArg)[12 + i] = tempBuf2[i];
			}
			IT8951WriteDataBurst(bus, type, usArg,
					12 + pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);

//			IT8951WriteDataBurst(bus, type, (TWord*) tempBuf2,
//					pstAreaImgInfo->usWidth / 2 * pstAreaImgInfo->usHeight);
			j = pstAreaImgInfo->usHeight;

			if (usArg)
				free(usArg);
		}
		IT8951LoadImgEnd(bus, type);

		gettimeofday(&tStop, NULL);
		tTotal = (float) (tStop.tv_sec - tStart.tv_sec)
				+ ((float) (tStop.tv_usec - tStart.tv_usec) / 1000000);
		printf("Data Transmission --> Time: %f\n", tTotal);

		//if (tempBuf)
		//free(tempBuf);

		//if (tempBuf2)
		//free(tempBuf2);

	} else {

		//pstAreaImgInfo->usX = 400;
		//pstAreaImgInfo->usY = 300;
		//saveBufToPNG(pstAreaImgInfo->usWidth, pstAreaImgInfo->usHeight,
		//		pusFrameBuf);
		//IT8951LoadImgStart(bus, type, pstLdImgInfo);

		//Host Write Data
		//gettimeofday(&tStart, NULL);
		//
		if (*type == SPI_HRDY) {
			IT8951LoadImgAreaStart(bus, type, pstLdImgInfo, pstAreaImgInfo);
			struct pl_gpio * gpio = (struct pl_gpio *) bus->hw_ref;
			int b = 0;
			for (b = 0; b < pstAreaImgInfo->usHeight; b++) {

				IT8951WriteDataBurst(bus, type, (TWord*) pusFrameBuf,
						pstAreaImgInfo->usWidth / 2);
				pusFrameBuf += pstAreaImgInfo->usWidth;
				j = b;
			}
		} else if (*type == I80) {
			int i, t, test = 0;
			char burst_en = '1';

			int burst_var = open("/sys/class/pl_par/burst_en", O_RDWR);
			if (burst_var < 0) {
				printf("Could not enable Burst mode !");
			} else {
				test = write(burst_var, &burst_en, 1);
			}

			TWord *usArg = malloc(
					(12 + pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight)
							* sizeof(TWord));
			//Setting Argument for Load image start
			usArg[0] = IT8951_TCON_LD_IMG_AREA;
			usArg[1] = (pstLdImgInfo->usEndianType << 8)
					| (pstLdImgInfo->usPixelFormat << 4)
					| (pstLdImgInfo->usRotate);
			usArg[2] = pstAreaImgInfo->usX;
			usArg[3] = pstAreaImgInfo->usY;
			usArg[4] = pstAreaImgInfo->usWidth;
			usArg[5] = pstAreaImgInfo->usHeight;

			//IT8951WriteDataBurst(bus, type, usArg, 12);

//			TWord* buf = malloc(
//					sizeof(TWord)
//							* (pstAreaImgInfo->usWidth
//									* pstAreaImgInfo->usHeight * 1));
			//buf[0] = 0xFFFF;
			for (i = 0;
					i < (pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight);
					i++) {
				((TByte*) usArg)[12 + i] = pusFrameBuf[i];
			}
			IT8951WriteDataBurst(bus, type, usArg,
					pstAreaImgInfo->usWidth * pstAreaImgInfo->usHeight + 12);

//			IT8951WriteDataBurst(bus, type, (TWord*) pusFrameBuf,
//					pstAreaImgInfo->usWidth / 2 * pstAreaImgInfo->usHeight);
			j = pstAreaImgInfo->usHeight;

			if (burst_var < 0) {
				printf("Could not disable Burst mode !");
			} else {
				burst_en = '0';
				write(burst_var, &burst_en, 1);
			}

			if (usArg)
				free(usArg);
//			if (buf)
//				free(buf);
			close(burst_var);
		}

		gettimeofday(&tStop, NULL);
		tTotal = (float) (tStop.tv_sec - tStart.tv_sec)
				+ ((float) (tStop.tv_usec - tStart.tv_usec) / 1000000);
		//printf("Data Transmission --> Time: %f\n", tTotal);
		IT8951LoadImgEnd(bus, type);

	}

}

//-----------------------------------------------------------
//Display functions 3 - Application for Display panel Area
//-----------------------------------------------------------
void IT8951DisplayArea(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord usX, TWord usY, TWord usW, TWord usH, TWord usDpyMode) {
//Send I80 Display Command (User defined command of IT8951)
	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_DPY_AREA); //0x0034
//Write arguments
	IT8951WriteData(bus, type, usX);
	IT8951WriteData(bus, type, usY);
	IT8951WriteData(bus, type, usW);
	IT8951WriteData(bus, type, usH);
	IT8951WriteData(bus, type, usDpyMode);
}

//-----------------------------------------------------------
//Host controller function 1 �V Wait for host data Bus Ready
//-----------------------------------------------------------
int IT8951WaitForReady(pl_generic_interface_t *bus, enum interfaceType *type) {

	if (*type == SPI_HRDY) {
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;
		int i = 0;

		while (i++ < WAIT_FOR_READY_TIMEOUT_SPI_HRDY) {
			if (gpio->get(spi->hrdy_gpio) == 1) {
				return 0;
			}
		}
	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;
		int i = 0;

		while (i++ < WAIT_FOR_READY_TIMEOUT_I80) {

			if (gpio->get(i80->hrdy_gpio) == 1) {
				return 0;
			}
		}
	} else {
	}

	return 1;
}

void IT8951WriteCmdCode(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord usCmdCode) {
//wait for ready
	//IT8951WaitForReady(bus, type);
// swap data

#ifdef SWAPDATA
	{
		swap_data(&usCmdCode, 1);
	}
#endif

//write cmd code
	gpio_i80_16b_cmd_out(bus, type, usCmdCode);
}

//-----------------------------------------------------------
//Host controller function 3 �V Write Data to host data Bus
//-----------------------------------------------------------
void IT8951WriteData(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord usData) {
//wait for ready

//IT8951WaitForReady(bus, type);

//write data
	gpio_i80_16b_data_out(bus, type, usData);
}

//-----------------------------------------------------------
//Host controller function 3 �V Write Data to host data Bus
//-----------------------------------------------------------
void IT8951WriteDataBurst(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord *usData, int size) {
	int iResult = 0;

	if (*type == SPI_HRDY) {
//
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

		// Prepare SPI preamble to enable ITE Write Data mode via SPI
		uint8_t preamble_[2];
		preamble_[0] = (uint8_t) 0x00;
		preamble_[1] = (uint8_t) 0x00;

		int var, test;

		//Swap Data from little to big endian for spi
		swap16_array(usData, size);

		IT8951WaitForReady(bus, type);

		// open SPI Bus
		int stat = -EINVAL;
		stat = bus->open(spi);

		// Set CS to low
		gpio->set(spi->cs_gpio, 0);

		//send SPI write data preamble

		iResult = write(spi->fd, preamble_, 2);

		//usleep(250); //--> need this here ? Costs 0.7 to 0.8 seconds while transmitting data over spi
		IT8951WaitForReady(bus, type);

		//send SPI data
		iResult = write(spi->fd, usData, size * 2);

		// Set CS to high and end SPI communication
		gpio->set(spi->cs_gpio, 1);

	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

		// swap data
#ifdef SWAPDATA
		{
			swap_data(usData, size);
		}
#endif

		//int i = 0;

		//while (i++ < WAIT_FOR_READY_TIMEOUT_I80) {
		write(i80->fd, usData, size);
//			if (write(i80->fd, usData, size) > 0) {
//				return;
//			}
//		}
//			} else {
		//error
	}
	//return iResult;
}

//-----------------------------------------------------------
//Host controller function 4 �V Read Data from host data Bus
//-----------------------------------------------------------
TWord* IT8951ReadData(pl_generic_interface_t *bus, enum interfaceType *type,
		int size) {
	TWord* usData;

	usData = gpio_i80_16b_data_in(bus, type, size);

// swap data
#ifdef SWAPDATA
	{
		usData = swap_data_in(usData);
	}
#endif
	return usData;
}

//-----------------------------------------------------------
//Host controller function 4 �V Read Data from host data Bus
//-----------------------------------------------------------

void IT8951ReadDataBurst(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord *usData, int size) {
	int iResult = 0;

//wait for ready
	IT8951WaitForReady(bus, type);

	if (*type == SPI_HRDY) {
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

		//-------------------------real function start-------------------------------------

		//CS
		gpio->set(spi->cs_gpio, 0);

		int i = 0;

		//swap_endianess(usData);

		for (i = 0; i < size; i++) {
			// Executing read within the loop is necessary,
			// since executing read the does the HRD# pulse only once,
			// even with count >1 !
			iResult = read(spi->fd, usData + i, 1);
			// swap data
#ifdef SWAPDATA
			{
				usData[i] = swap_data_in(usData[i]);
			}
#endif
		}

		//CS
		gpio->set(spi->cs_gpio, 1);
		//-------------------------real function end-------------------------------------
	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

		//-------------------------real function start-------------------------------------

		//read data from host data bus
		gpio->set(i80->hdc_gpio, 1);

		//CS
		gpio->set(i80->hcs_n_gpio, 0);

		int i = 0;
		for (i = 0; i < size; i++) {
			// Executing read within the loop is necessary,
			// since executing read the does the HRD# pulse only once,
			// even with count >1 !
			iResult = read(i80->fd, usData + i, 1);
			// swap data
#ifdef SWAPDATA
			{
				usData[i] = swap_data_in(usData[i]);
			}
#endif
		}
		//CS
		gpio->set(i80->hcs_n_gpio, 1);
		//-------------------------real function end-------------------------------------
	} else {
		//error
	}
}

//-----------------------------------------------------------
//Host controller function 5 �V Write command to host data Bus with aruments
//-----------------------------------------------------------

void IT8951SendCmdArg(pl_generic_interface_t *bus, enum interfaceType *type,
		TWord usCmdCode, TWord* pArg, TWord usNumArg) {
	TWord i;

	if (*type == I80) {
		TWord *buf = malloc(usNumArg * 2);
		buf[0] = usCmdCode;
		for (i = 0; i < usNumArg; i++) {
			buf[i + 1] = pArg;
		}
		IT8951WriteDataBurst(bus, type, buf, usNumArg * 2);
		free(buf);

	} else {
		//Send Cmd code
		IT8951WriteCmdCode(bus, type, usCmdCode);
		//Send Data
		for (i = 0; i < usNumArg; i++) {
			IT8951WriteData(bus, type, pArg[i]);
		}
	}

}

//-----------------------------------------------------------
//Host Cmd 11 - LD_IMG_AREA
//-----------------------------------------------------------
void IT8951LoadImgAreaStart(pl_generic_interface_t *bus,
		enum interfaceType *type, IT8951LdImgInfo* pstLdImgInfo,
		IT8951AreaImgInfo* pstAreaImgInfo) {

//Send Cmd and Args
	if (*type == I80) {
		TWord usArg[6];
		//Setting Argument for Load image start
		usArg[0] = IT8951_TCON_LD_IMG_AREA;
		usArg[1] = (pstLdImgInfo->usEndianType << 8)
				| (pstLdImgInfo->usPixelFormat << 4) | (pstLdImgInfo->usRotate);
		usArg[2] = pstAreaImgInfo->usX;
		usArg[3] = pstAreaImgInfo->usY;
		usArg[4] = pstAreaImgInfo->usWidth;
		usArg[5] = pstAreaImgInfo->usHeight;
		IT8951WriteDataBurst(bus, type, usArg, 12);
	} else {
		TWord usArg[5];
		//Setting Argument for Load image start
		usArg[0] = (pstLdImgInfo->usEndianType << 8)
				| (pstLdImgInfo->usPixelFormat << 4) | (pstLdImgInfo->usRotate);
		usArg[1] = pstAreaImgInfo->usX;
		usArg[2] = pstAreaImgInfo->usY;
		usArg[3] = pstAreaImgInfo->usWidth;
		usArg[4] = pstAreaImgInfo->usHeight;
		IT8951SendCmdArg(bus, type, IT8951_TCON_LD_IMG_AREA, usArg, 5);
	}

}

//Host CMD IT8951_TCON_LD_IMG
void IT8951LoadImgStart(pl_generic_interface_t *bus, enum interfaceType *type,
		IT8951LdImgInfo* pstLdImgInfo) {
	TWord usArg[1];
//Setting Argument for Load image start
	usArg[0] = (pstLdImgInfo->usEndianType << 8)
			| (pstLdImgInfo->usPixelFormat << 4) | (pstLdImgInfo->usRotate);

//Send Cmd and Args
	IT8951SendCmdArg(bus, type, IT8951_TCON_LD_IMG, usArg, 1);
}

void IT8951MemBurstWriteProc(pl_generic_interface_t *bus,
		enum interfaceType *type, TDWord ulMemAddr, TDWord ulWriteSize,
		TWord *pSrcBuf) {

	TDWord i;

//Send Burst Write Start Cmd and Args
	IT8951MemBurstWrite(bus, type, ulMemAddr, ulWriteSize);
//
//	//Burst Write Data
	for (i = 0; i < ulWriteSize; i++) {
		gpio_i80_16b_data_out(bus, type, pSrcBuf[i]);
	}
//
//	//Send Burst End Cmd
	gpio_i80_16b_cmd_out(bus, type, IT8951_TCON_MEM_BST_END);
}

//-----------------------------------------------------------
//Host Cmd 8 - MEM_BST_WR
//-----------------------------------------------------------
void IT8951MemBurstWrite(pl_generic_interface_t *bus, enum interfaceType *type,
		TDWord ulMemAddr, TDWord ulWriteSize) {
	TWord usArg[4];
//Setting Arguments for Memory Burst Write
	usArg[0] = (TWord) (ulMemAddr & 0x0000FFFF); //addr[15:0]
	usArg[1] = (TWord) ((ulMemAddr >> 16) & 0x0000FFFF); //addr[25:16]
	usArg[2] = (TWord) (ulWriteSize & 0x0000FFFF); //Cnt[15:0]
	usArg[3] = (TWord) ((ulWriteSize >> 16) & 0x0000FFFF); //Cnt[25:16]
//Send Cmd and Arg
	IT8951SendCmdArg(bus, type, IT8951_TCON_MEM_BST_WR, usArg, 4);
}

//-----------------------------------------------------------
//Host Cmd 12 - LD_IMG_END
//-----------------------------------------------------------
void IT8951LoadImgEnd(pl_generic_interface_t *bus, enum interfaceType *type) {
	IT8951WriteCmdCode(bus, type, IT8951_TCON_LD_IMG_END);
}

//-------------------------------------------------------------------
//Host controller Write command code for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static void gpio_i80_16b_cmd_out(pl_generic_interface_t *bus,
		enum interfaceType *type, TWord usCmd) {
	int iResult = 0;

	if (*type == SPI_HRDY) {

		uint8_t usCmd_1[2];
		usCmd_1[0] = (uint8_t) 0x60;
		usCmd_1[1] = (uint8_t) 0x00;

		uint8_t usCmd_2[2];
		usCmd_2[0] = (uint8_t) (usCmd >> 8);
		usCmd_2[1] = (uint8_t) usCmd;

		int stat = -EINVAL;
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus;

		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

		// open spi
		stat = bus->open(bus);

		IT8951WaitForReady(bus, type);

		stat = bus->set_cs(bus, 0);
		stat = bus->write_bytes(bus, usCmd_1, 2);

		IT8951WaitForReady(bus, type);

		stat = bus->write_bytes(bus, usCmd_2, 2);
		stat = bus->set_cs(bus, 1);

	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

		int size = sizeof(usCmd);

		//IT8951WaitForReady(bus, type);
		//Set GPIO 0~7 to Output mode
		//See your host setting of GPIO
		//Switch C/D to CMD => CMD - L
		//GPIO_SET_L(CD);
		//gpio->set(i80->hdc_gpio, 0);
		//CS-L
		//GPIO_SET_L(CS);
		//gpio->set(i80->hcs_n_gpio, 0);
		//WR Enable
		//GPIO_SET_L(WEN);
		//gpio->set(i80->hwe_n_gpio, 0);
		//Set Data output (Parallel output request)
		//See your host setting of GPIO

		iResult = write(i80->fd, &usCmd, size);

		//gpio->set(i80->hwe_n_gpio, 1);
		//CS-H
		//GPIO_SET_H(CS);
		//gpio->set(i80->hcs_n_gpio, 1);
	} else {
		//error
	}

}
//-------------------------------------------------------------------
//Host controller Write Data for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static void gpio_i80_16b_data_out(pl_generic_interface_t *bus,
		enum interfaceType *type, TWord usData) {
	int iResult = 0;

	if (*type == SPI_HRDY) {
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

		// Prepare SPI preamble to enable ITE Write Data mode via SPI
		uint8_t preamble_[2];
		preamble_[0] = (uint8_t) 0x00;
		preamble_[1] = (uint8_t) 0x00;

		// Split TWord Data (2byte) into its uint8 (1byte) subunits
		uint8_t data_[2];
		data_[0] = (uint8_t) (usData >> 8);
		data_[1] = (uint8_t) usData;

		swap_endianess(data_);

		// open SPI Bus
		int stat = -EINVAL;
		stat = bus->open(bus);

		IT8951WaitForReady(bus, type);

		// Set CS to low
		gpio->set(spi->cs_gpio, 0);

		//send SPI write data preamble
		stat = bus->write_bytes(bus, preamble_, 2);

		IT8951WaitForReady(bus, type);

		//send SPI data
		stat = bus->write_bytes(bus, data_, 2);

		// Set CS to high and end SPI communication
		gpio->set(bus->cs_gpio, 1);

	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

		IT8951WaitForReady(bus, type);

		//Switch C/D to Data => Data - H
		//GPIO_SET_H(CD);
		gpio->set(i80->hdc_gpio, 1);

		//CS-L
		//GPIO_SET_L(CS);
		gpio->set(i80->hcs_n_gpio, 0);

		//WR Enable
		//GPIO_SET_L(WEN);
		//gpio->set(i80->hwe_n_gpio, 0);
		//Set 16 bits Bus Data
		//See your host setting of GPIO
		iResult = write(i80->fd, &usData, 1);

		//WR Enable - H
		//GPIO_SET_H(WEN);
		//gpio->set(i80->hwe_n_gpio, 1);
		//CS-H
		//GPIO_SET_H(CS);
		gpio->set(i80->hcs_n_gpio, 1);
	} else {
		//error
	}
}
//-------------------------------------------------------------------
//Host controller Read Data for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static TWord* gpio_i80_16b_data_in(pl_generic_interface_t *bus,
		enum interfaceType *type, int size) {

	TWord usData;
	TWord* iResult = (TWord*) malloc(size);

	if (*type == SPI_HRDY) {
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus;
		spi->mSpi = bus->mSpi;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

		TByte preamble_[2];
		preamble_[0] = (TByte) 0x10;
		preamble_[1] = (TByte) 0x00;

		// open SPI Bus
		int stat = -EINVAL;
		stat = bus->open(bus);

		//Set SPI-CS low
		gpio->set(spi->cs_gpio, 0);

		//send SPI read data preamble
		stat = bus->write_bytes(bus, preamble_, 2);

		// throw away first read, as this only contains rubbish (as documented by ITE)
		IT8951WaitForReady(bus, type);
		bus->read_bytes(spi, &usData, sizeof(TWord));

		//Loop through the various data
		for (int i = 0; i < size / sizeof(TWord); i++) {
			IT8951WaitForReady(bus, type);
			bus->read_bytes(spi, &usData, sizeof(TWord));
			iResult[i] = swap_endianess(usData);
			//usleep(250);
		}

		//Set SPI-CS high
		gpio->set(spi->cs_gpio, 1);

	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

		read(i80->fd, iResult, size);

	} else {
		//error
	}

	return iResult;
}

static void swap_data(TWord *buff, int size) {
	int i = 0;
	TWord tmp = 0;

	for (i = 0; i < size; i++) {
		tmp = buff[i];

		buff[i] = ((tmp & 0x0080) << 1);
		buff[i] = buff[i] | ((tmp & 0x0040) << 3);
		buff[i] = buff[i] | ((tmp & 0x0020) << 7);
		buff[i] = buff[i] | ((tmp & 0x0010) << 6);
		buff[i] = buff[i] | ((tmp & 0x0008) << 10);
		buff[i] = buff[i] | ((tmp & 0x0004) << 9);
		buff[i] = buff[i] | ((tmp & 0x0002) << 13);
		buff[i] = buff[i] | ((tmp & 0x0001) << 15);

		buff[i] = buff[i] | ((tmp & 0x8000) >> 15);
		buff[i] = buff[i] | ((tmp & 0x4000) >> 13);
		buff[i] = buff[i] | ((tmp & 0x2000) >> 11);
		buff[i] = buff[i] | ((tmp & 0x1000) >> 9);
		buff[i] = buff[i] | ((tmp & 0x0800) >> 7);
		buff[i] = buff[i] | ((tmp & 0x0400) >> 5);
		buff[i] = buff[i] | ((tmp & 0x0200) >> 3);
		buff[i] = buff[i] | ((tmp & 0x0100) >> 1);
	}

//printf("swap: 0x%x | 0x%x --> 0x%x | 0x%x\n", tmp[0], tmp[1], buff[i*2], buff[i*2+1]);
}

TWord swap_endianess(TWord in) {

	uint8_t buff[2];

	buff[0] = (uint8_t) in;
	buff[1] = (uint8_t) (in >> 8);

	TWord out;

	out = buff[1];
	out = out | (buff[0] << 8);

	return out;
}

static TWord swap_data_in(TWord in) {
	int i = 0;
	uint8_t buff[2];
	uint8_t tmp[2];

	buff[0] = (uint8_t) in;
	buff[1] = (uint8_t) (in >> 8);

	TWord out;

	tmp[0] = buff[0];
	tmp[1] = buff[1];

	tmp[0] = buff[i * 2];
	tmp[1] = buff[i * 2 + 1];

	buff[i * 2 + 1] = ((tmp[1] & 0x80) >> 7);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x40) >> 5);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x20) >> 2);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x10) << 1);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x08) >> 1);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x04) << 2);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x02) << 5);
	buff[i * 2 + 1] = buff[i * 2 + 1] | ((tmp[1] & 0x01) << 7);

	buff[i * 2] = ((tmp[0] & 0x80) >> 7);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x40) >> 5);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x20) >> 3);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x10) >> 1);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x08) << 1);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x04) << 3);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x02) << 5);
	buff[i * 2] = buff[i * 2] | ((tmp[0] & 0x01) << 7);

	TWord value = (buff[i * 2] << 8) | buff[i * 2 + 1];

	out = buff[1];
	out = out | (buff[0] << 8);

	printf("swap: 0x%x --> 0x%x --> %d\n", in, out, out);

	return out;
}

void saveBufToPNG(int width, int height, uint8_t *buf) {
	FILE *fp;

	fp = fopen("/tmp/imgBuf.png", "wb");

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
	NULL, NULL);

	png_infop info = png_create_info_struct(png);

	png_init_io(png, fp);

	png_set_IHDR(png, info, width, height, 8,
	PNG_COLOR_TYPE_GRAY,
	PNG_INTERLACE_NONE,
	PNG_COMPRESSION_TYPE_DEFAULT,
	PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

//	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
//	for (int y = 0; y < height; y++) {
//		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png, info));
	int i, hi, wi = 0;
	png_bytepp row_pointers = (png_bytepp) png_malloc(png,
			sizeof(png_bytepp) * height);
	for (i = 0; i < height; i++) {
		row_pointers[i] = (png_bytep) png_malloc(png, width);
	}
	for (hi = 0; hi < height; hi++) {
		for (wi = 0; wi < width; wi++) {
			// bmp_source is source data that we convert to png
			row_pointers[hi][wi] = buf[wi + width * hi];
		}
	}

	png_write_image(png, row_pointers);

	png_write_end(png, NULL);

	fclose(fp);

	png_destroy_write_struct(&png, &info);
}
