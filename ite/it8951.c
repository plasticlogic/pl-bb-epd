/*
 * it8951.c
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#include <stdlib.h>
#include <unistd.h>
#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#include <ite/it8951.h>
#define LOG_TAG "it8951"
#include "pl/utils.h"
#include <pl/spi.h>
#include <pl/spi_hrdy.h>
#include <sys/time.h>

//#define SWAPDATA


static void swap_data(TWord *buff, int size);
static TWord swap_data_in(TWord in);



it8951_t *it8951_new(struct pl_gpio *gpios, struct pl_generic_interface *interface, enum interfaceType *sInterfaceType, struct pl_i2c *i2c, const struct it8951_pins *pins)
{
	assert(gpios != NULL);
	assert(pins != NULL);
	assert(interface != NULL);

	it8951_t *p = (it8951_t *)malloc(sizeof(it8951_t));
	p->gpio = gpios;
	p->interface =  interface;
	p->pins =  pins;
	p->sInterfaceType = sInterfaceType;

	return p;
}

//void GetIT8951SystemInfo(struct pl_i80 *p, void* pBuf)
//{
//    TWord* pusWord = (TWord*)pBuf;
//    I80IT8951DevInfo* pstDevInfo;
//    //Send I80 CMD
//    IT8951WriteCmdCode(p, USDEF_I80_CMD_GET_DEV_INFO);
////    #ifdef EN_SPI_2_I80
////
////    //Burst Read Request for SPI interface only
////    LCDReadNData(pusWord, sizeof(I80IT8951DevInfo)/2);//Polling HRDY for each words(2-bytes) if possible
////
////    #else
//    //I80 interface - Single Read availabl
//    int i;
//    for(i=0; i<sizeof(I80IT8951DevInfo)/2; i++)
//    {
//        pusWord[i] = IT8951ReadData(p);
//      }
//
////    IT8951ReadDataBurst(p, pusWord, sizeof(I80IT8951DevInfo)/2);
//
////    #endif
//
//    //Show Device information of IT8951
//    pstDevInfo = (I80IT8951DevInfo*)pBuf;
//    printf("Panel(W,H) = (%d,%d)\n", pstDevInfo->usPanelW, pstDevInfo->usPanelH );
//    printf("Image Buffer Address = %X\r\n",
//    pstDevInfo->usImgBufAddrL | (pstDevInfo->usImgBufAddrH << 16));
//    //Show Firmware and LUT Version
//    //printf("FW Version = %s\r\n", stI80IT8951DevInfo.usFWVersion);
//    //printf("LUT Version = %s\r\n", stI80IT8951DevInfo.usLUTVersion);
//}

void GetIT8951SystemInfo(pl_generic_interface_t *bus, enum interfaceType *type  , void* pBuf)
{
    //TWord* pusWord = (TWord*)pBuf;
    I80IT8951DevInfo* pstDevInfo;
    //Send I80 CMD
    IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_GET_DEV_INFO);
//    #ifdef EN_SPI_2_I80
//
//    //Burst Read Request for SPI interface only
//    LCDReadNData(pusWord, sizeof(I80IT8951DevInfo)/2);//Polling HRDY for each words(2-bytes) if possible
//
//    #else
    //I80 interface - Single Read availabl
    //int i;
    //for(i=0; i<sizeof(I80IT8951DevInfo)/2; i++)
    //{
    //    pusWord[i] = IT8951ReadData(bus, type);
    //  }

    pstDevInfo = IT8951ReadData(bus, type, sizeof(I80IT8951DevInfo)/2);

//    IT8951ReadDataBurst(p, pusWord, sizeof(I80IT8951DevInfo)/2);

//    #endif

    //Show Device information of IT8951
    //pstDevInfo = (I80IT8951DevInfo*)pusWord;
    printf("Panel(W,H) = (%d,%d)\n", pstDevInfo->usPanelW, pstDevInfo->usPanelH );
    printf("Image Buffer Address = %X\r\n",
    pstDevInfo->usImgBufAddrL | (pstDevInfo->usImgBufAddrH << 16));
    //Show Firmware and LUT Version
    //printf("FW Version = %s\r\n", stI80IT8951DevInfo.usFWVersion);
    //printf("LUT Version = %s\r\n", stI80IT8951DevInfo.usLUTVersion);
}

//-----------------------------------------------------------
//Initial function 2 ¡V Set Image buffer base address
//-----------------------------------------------------------
void IT8951SetImgBufBaseAddr(pl_generic_interface_t *bus, enum interfaceType *type, TDWord ulImgBufAddr)
{
    TWord usWordH = (TWord)((ulImgBufAddr >> 16) & 0x0000FFFF);
    TWord usWordL = (TWord)( ulImgBufAddr & 0x0000FFFF);
    //Write LISAR Reg
    IT8951WriteReg(bus, type, LISAR + 2 ,usWordH);
    IT8951WriteReg(bus, type, LISAR ,usWordL);
}

//-----------------------------------------------------------
//Host Cmd 4 - REG_RD
//-----------------------------------------------------------
TWord IT8951ReadReg(pl_generic_interface_t *bus, enum interfaceType *type, TWord usRegAddr)
{
    TWord usData;
    //----------I80 Mode-------------
    //Send Cmd and Register Address
    IT8951WriteCmdCode(bus, type, IT8951_TCON_REG_RD);
    IT8951WriteData(bus, type, usRegAddr);
    //Read data from Host Data bus
    usData = IT8951ReadData(bus, type, 1);
    return usData;
}

//-----------------------------------------------------------
//Host Cmd 5 - REG_WR
//-----------------------------------------------------------
void IT8951WriteReg(pl_generic_interface_t *bus, enum interfaceType *type, TWord usRegAddr,TWord usValue)
{
    //I80 Mode
    //Send Cmd , Register Address and Write Value
	IT8951WriteCmdCode(bus, type, IT8951_TCON_REG_WR);
	IT8951WriteData(bus, type, usRegAddr);
	IT8951WriteData(bus, type, usValue);
}

//-----------------------------------------------------------
//Display function 1 - Wait for LUT Engine Finish
//                     Polling Display Engine Ready by LUTNo
//-----------------------------------------------------------
void IT8951WaitForDisplayReady(pl_generic_interface_t *bus, enum interfaceType *type)
{
    //Check IT8951 Register LUTAFSR => NonZero ¡V Busy, 0 - Free
    while(IT8951ReadReg(bus, type, LUTAFSR));
}

//-----------------------------------------------------------
//Display function 2 ¡V Load Image Area process
//-----------------------------------------------------------
void IT8951HostAreaPackedPixelWrite(pl_generic_interface_t *bus, enum interfaceType *type, IT8951LdImgInfo* pstLdImgInfo, IT8951AreaImgInfo* pstAreaImgInfo)
{
    TDWord i,j;
    struct timeval tStop, tStart; // time variables
    float tTotal;

    //Source buffer address of Host
    TWord* pusFrameBuf = (TWord*)pstLdImgInfo->ulStartFBAddr;

//	#if 0
//	//if width or height over than 2048 use memburst write instead, only allow 8bpp data
//    IT8951MemBurstWriteProc(pstLdImgInfo->ulImgBufBaseAddr,  pstAreaImgInfo->usWidth/2* pstAreaImgInfo->usHeight,   pusFrameBuf); //MemAddr, Size, Framebuffer address
//	#else
    //Set Image buffer(IT8951) Base address
    IT8951SetImgBufBaseAddr(bus, type, pstLdImgInfo->ulImgBufBaseAddr);
    //Send Load Image start Cmd
    IT8951LoadImgAreaStart(bus, type, pstLdImgInfo , pstAreaImgInfo);
    //Host Write Data
    gettimeofday(&tStart, NULL);
//    for(j=0;j< pstAreaImgInfo->usHeight;j++)
//    {
//    	#ifdef __SPI_2_I80_INF__
//
//            //Write 1 Line for each SPI transfer
//            LCDWriteNData(pusFrameBuf, pstAreaImgInfo->usWidth/2);
//            pusFrameBuf += pstAreaImgInfo->usWidth/2;//Change to Next line of loaded image (supposed the Continuous image content in hsot frame buffer )
//
//        #else

//    	gettimeofday(&tStart, NULL);

//        for(i=0;i< pstAreaImgInfo->usWidth/2;i++)
//        {
//            //Write a Word(2-Bytes) for each time
//        	LCDWriteData_NoSwap(p, *pusFrameBuf);
//            pusFrameBuf++;
//        }
        //#endif

    	//LCDWriteDataBurst(p, pusFrameBuf + (j * pstAreaImgInfo->usWidth/2), pstAreaImgInfo->usWidth/2);

//        gettimeofday(&tStop, NULL);
//
//        tTotal = (float)(tStop.tv_sec - tStart.tv_sec) + ((float)(tStop.tv_usec - tStart.tv_usec)/1000000);

        //printf("Height: %d --> Time: %f\n", j, tTotal);

    //}
    IT8951WriteDataBurst(bus, type, pusFrameBuf, pstAreaImgInfo->usWidth/2 * pstAreaImgInfo->usHeight);
    gettimeofday(&tStop, NULL);

    tTotal = (float)(tStop.tv_sec - tStart.tv_sec) + ((float)(tStop.tv_usec - tStart.tv_usec)/1000000);
    printf("Height: %d --> Time: %f\n", j, tTotal);

    //Send Load Img End Command
    IT8951LoadImgEnd(bus, type);
//	#endif
}

//-----------------------------------------------------------
//Display functions 3 - Application for Display panel Area
//-----------------------------------------------------------
void IT8951DisplayArea(pl_generic_interface_t *bus, enum interfaceType *type, TWord usX, TWord usY, TWord usW, TWord usH, TWord usDpyMode)
{
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
//Host controller function 1 ¡V Wait for host data Bus Ready
//-----------------------------------------------------------
int IT8951WaitForReady(pl_generic_interface_t *bus, enum interfaceType *type)
{
	//Regarding to HRDY
	//you may need to use a GPIO pin connected to HRDY of IT8951
//    TDWord ulData = HRDY;
//    while(ulData == 0)
//    {
//        //Get status of HRDY
//        ulData = HRDY;
//    }

	if(*type == SPI_HRDY ){
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus ->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;
			int i = 0;

			while(i++ < WAIT_FOR_READY_TIMEOUT_SPI_HRDY)
			{
					if(gpio->get(spi->hrdy_gpio) == 1)
					{
						return 0;
					}
			}
	}
	else if(*type == I80){
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;
		int i = 0;

		while(i++ < WAIT_FOR_READY_TIMEOUT_I80)
		{

				if(gpio->get(i80->hrdy_gpio) == 1)
				{
					return 0;
				}
		}
	}
	else{
		//error
	}

	return 1;
}

//-----------------------------------------------------------------
//Host controller function 2 ¡V Write command code to host data Bus
//-----------------------------------------------------------------
//void IT8951WriteCmdCode(struct pl_i80 *p, TWord usCmdCode)
//{
//    //wait for ready
//	IT8951WaitForReady(p);
//    // swap data
//
//#ifdef SWAPDATA
//    {
//    swap_data(&usCmdCode, 1);
//    }
//#endif
//
//    //write cmd code
//    gpio_i80_16b_cmd_out(p, usCmdCode);
//}

void IT8951WriteCmdCode(pl_generic_interface_t *bus, enum interfaceType *type,  TWord usCmdCode)
{
    //wait for ready
	IT8951WaitForReady(bus, type);
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
//Host controller function 3 ¡V Write Data to host data Bus
//-----------------------------------------------------------
void IT8951WriteData(pl_generic_interface_t *bus, enum interfaceType *type, TWord usData)
{
    //wait for ready
	IT8951WaitForReady(bus, type);
    // swap data
#ifdef SWAPDATA
    {
    swap_data(&usData, 1);
    }
#endif
    //write data
    gpio_i80_16b_data_out(bus, type, usData);
}

//-----------------------------------------------------------
//Host controller function 3 ¡V Write Data to host data Bus
//-----------------------------------------------------------
void IT8951WriteDataBurst(pl_generic_interface_t *bus, enum interfaceType *type, TWord *usData, int size)
{
	int iResult = 0;

	//int i = 0;
	//TWord usData_;

	if(*type == SPI_HRDY){
		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus ->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

	    // swap data
	#ifdef SWAPDATA
	    {
	    swap_data(usData, size);
	    }
	#endif

	    //wait for ready
	    IT8951WaitForReady(bus, type);

	    //Switch C/D to Data => Data - H
	    //GPIO_SET_H(CD);

	    //CS
	    gpio->set(spi->cs_gpio, 0);

	    struct timeval tStop, tStart; // time variables
	    float tTotal;

	    gettimeofday(&tStart, NULL);
	    //for(i=0; i<size; i++)
	//    {
	////    	usData_ = usData[i];
	////        // swap data
	////    	usData_ = swap_data(usData_);
	////		//Set 16 bits Bus Data
	////		//See your host setting of GPIO
	////		iResult = write(p->fd, &usData_, 1);
	//
	//		// unswaped data
	//		iResult = write(p->fd, usData[i], 1);
	//    }

	    iResult = write(spi->fd, usData, size/2);
	    IT8951WaitForReady(bus, type);
	    iResult = write(spi->fd, usData + size/2, size/2);


	    gettimeofday(&tStop, NULL);
	    tTotal = (float)(tStop.tv_sec - tStart.tv_sec) + ((float)(tStop.tv_usec - tStart.tv_usec)/1000000);
	    printf("Data Transmission --> Time: %f\n", tTotal);

	    //wait for ready
	    IT8951WaitForReady(bus, type);

	    //CS
	    gpio->set(spi->cs_gpio, 1);

		}
	else if(*type == I80){
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

	    // swap data
	#ifdef SWAPDATA
	    {
	    swap_data(usData, size);
	    }
	#endif

	    //wait for ready
	    IT8951WaitForReady(bus, type);

	    //Switch C/D to Data => Data - H
	    //GPIO_SET_H(CD);
	    gpio->set(i80->hdc_gpio, 1);

	    //CS
	    gpio->set(i80->hcs_n_gpio, 0);

	    struct timeval tStop, tStart; // time variables
	    float tTotal;

	    gettimeofday(&tStart, NULL);
	    //for(i=0; i<size; i++)
	//    {
	////    	usData_ = usData[i];
	////        // swap data
	////    	usData_ = swap_data(usData_);
	////		//Set 16 bits Bus Data
	////		//See your host setting of GPIO
	////		iResult = write(p->fd, &usData_, 1);
	//
	//		// unswaped data
	//		iResult = write(p->fd, usData[i], 1);
	//    }

	    iResult = write(i80->fd, usData, size/2);
	    IT8951WaitForReady(bus, type);
	    iResult = write(i80->fd, usData + size/2, size/2);


	    gettimeofday(&tStop, NULL);
	    tTotal = (float)(tStop.tv_sec - tStart.tv_sec) + ((float)(tStop.tv_usec - tStart.tv_usec)/1000000);
	    printf("Data Transmission --> Time: %f\n", tTotal);

	    //wait for ready
	    IT8951WaitForReady(bus, type);

	    //CS
	    gpio->set(i80->hcs_n_gpio, 1);
		}
	else{
			//error
		}
}

//-----------------------------------------------------------
//Host controller function 4 ¡V Read Data from host data Bus
//-----------------------------------------------------------
TWord* IT8951ReadData(pl_generic_interface_t *bus, enum interfaceType *type, int size)
{
    TWord* usData;
    //wait for ready
    IT8951WaitForReady(bus, type);
    //read data from host data bus
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
//Host controller function 4 ¡V Read Data from host data Bus
//-----------------------------------------------------------

void IT8951ReadDataBurst(pl_generic_interface_t *bus, enum interfaceType *type, TWord *usData, int size)
{
	int iResult = 0;

    //wait for ready
	IT8951WaitForReady(bus, type);

    if(*type == SPI_HRDY){
    		pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus ->hw_ref;
    		struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

    		//-------------------------real function start-------------------------------------

    		//CS
    		gpio->set(spi->cs_gpio, 0);

    		int i = 0;
    		for(i=0; i<size; i++)
    		{
    			// Executing read within the loop is necessary,
    			// since executing read the does the HRD# pulse only once,
    			// even with count >1 !
    			iResult = read(spi->fd, usData+i, 1);
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
    		}
    	else if(*type == I80){
    		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
    		struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

    		//-------------------------real function start-------------------------------------

    			//read data from host data bus
    		    gpio->set(i80->hdc_gpio, 1);

    		    //CS
    		    gpio->set(i80->hcs_n_gpio, 0);

    		    int i = 0;
    		    for(i=0; i<size; i++)
    		    {
    		    	// Executing read within the loop is necessary,
    		    	// since executing read the does the HRD# pulse only once,
    		    	// even with count >1 !
    		    	iResult = read(i80->fd, usData+i, 1);
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
    		}
    	else{
    			//error
    		}
}

//-----------------------------------------------------------
//Host controller function 5 ¡V Write command to host data Bus with aruments
//-----------------------------------------------------------

void IT8951SendCmdArg(pl_generic_interface_t *bus, enum interfaceType *type, TWord usCmdCode,TWord* pArg, TWord usNumArg)
{
     TWord i;
     //Send Cmd code
     IT8951WriteCmdCode(bus, type, usCmdCode);
     //Send Data
     for(i=0;i<usNumArg;i++)
     {
    	 IT8951WriteData(bus, type, pArg[i]);
     }
}

//-----------------------------------------------------------
//Host Cmd 11 - LD_IMG_AREA
//-----------------------------------------------------------
void IT8951LoadImgAreaStart(pl_generic_interface_t *bus, enum interfaceType *type, IT8951LdImgInfo* pstLdImgInfo ,IT8951AreaImgInfo* pstAreaImgInfo)
{
    TWord usArg[5];
    //Setting Argument for Load image start
    usArg[0] = (pstLdImgInfo->usEndianType << 8 )
    |(pstLdImgInfo->usPixelFormat << 4)
    |(pstLdImgInfo->usRotate);
    usArg[1] = pstAreaImgInfo->usX;
    usArg[2] = pstAreaImgInfo->usY;
    usArg[3] = pstAreaImgInfo->usWidth;
    usArg[4] = pstAreaImgInfo->usHeight;
    //Send Cmd and Args
    IT8951SendCmdArg(bus, type, IT8951_TCON_LD_IMG_AREA , usArg , 5);
}
//-----------------------------------------------------------
//Host Cmd 12 - LD_IMG_END
//-----------------------------------------------------------
void IT8951LoadImgEnd(pl_generic_interface_t *bus, enum interfaceType *type)
{
	IT8951WriteCmdCode(bus, type, IT8951_TCON_LD_IMG_END);
}


//-------------------------------------------------------------------
//Host controller Write command code for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static void gpio_i80_16b_cmd_out(pl_generic_interface_t *bus, enum interfaceType *type, TWord usCmd)
{
	int iResult = 0;


	if(*type == SPI_HRDY){

			uint8_t usCmd_[4];
			usCmd_[0] = (uint8_t) 0x60;
			usCmd_[1] = (uint8_t) 0x00;
			usCmd_[2] = (uint8_t) (usCmd >> 8);
			usCmd_[3] = (uint8_t)  usCmd;


			int stat = -EINVAL;
			pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus ->hw_ref;
//			spi->close 			= (pl_spi_hrdy_t*) bus->close;
//			spi->open 			= (pl_spi_hrdy_t*) bus->open;
//			spi->read_bytes 	= (pl_spi_hrdy_t*) bus->read_bytes;
//			spi->write_bytes 	= (pl_spi_hrdy_t*) bus->write_bytes;
//			spi->set_cs 		= (pl_spi_hrdy_t*) bus->set_cs;
//			spi->delete 		= (pl_spi_hrdy_t*) bus->delete;
//			spi->mSpi 			= (pl_spi_hrdy_t*) bus->mSpi;
//			spi->fd 			= (pl_spi_hrdy_t*) bus->fd;
			struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

			// open spi
			stat = bus->open(bus);

			IT8951WaitForReady(bus, type);
			//Set GPIO 0~7 to Output mode
			//See your host setting of GPIO
			//Switch C/D to CMD => CMD - L
			//GPIO_SET_L(CD);
			//CS-L
			//GPIO_SET_L(CS);

			//WR Enable
			//GPIO_SET_L(WEN);
			//gpio->set(i80_ref->hwe_n_gpio, 0);
			//Set Data output (Parallel output request)
			//See your host setting of GPIO
			//GPIO_I80_Bus[16] = usCmd;


			//gpio->set(spi->cs_gpio, 0);
			//iResult = write(spi->fd, &usCmd, 1);

			stat = bus->set_cs(spi, 0);
			stat = bus->write_bytes(bus, usCmd_, 4);
			//stat = send_cmd(spi, &usCmd);			// read command
			//stat = spi.write_bytes(&spi, reg, 3);			// write 3-byte address
			//stat = stat = spi.read_bytes(&spi, data, transferChunkSize);		// read data
			stat = bus->set_cs(spi, 1);

			//gpio->set(spi->cs_gpio, 1);

			//WR Enable - H
			//GPIO_SET_H(WEN);
			//gpio->set(i80_ref->hwe_n_gpio, 1);

			//CS-H
			//GPIO_SET_H(CS);


			}
	else if(*type == I80){
			pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
			struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

			IT8951WaitForReady(bus, type);
			//Set GPIO 0~7 to Output mode
			//See your host setting of GPIO
			//Switch C/D to CMD => CMD - L
			//GPIO_SET_L(CD);
			gpio->set(i80->hdc_gpio, 0);
			//CS-L
			//GPIO_SET_L(CS);
			gpio->set(i80->hcs_n_gpio, 0);
			//WR Enable
			//GPIO_SET_L(WEN);
			//gpio->set(i80_ref->hwe_n_gpio, 0);
			//Set Data output (Parallel output request)
			//See your host setting of GPIO
			//GPIO_I80_Bus[16] = usCmd;
			iResult = write(i80->fd, &usCmd, 1);

			//WR Enable - H
			//GPIO_SET_H(WEN);
			//gpio->set(i80_ref->hwe_n_gpio, 1);

			//CS-H
			//GPIO_SET_H(CS);
			gpio->set(i80->hcs_n_gpio, 1);
			}
		else{
				//error
			}



}
//-------------------------------------------------------------------
//Host controller Write Data for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static void gpio_i80_16b_data_out(pl_generic_interface_t *bus, enum interfaceType *type, TWord usData)
{
	int iResult = 0;

	if(*type == SPI_HRDY){
			pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus ->hw_ref;
			struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

			uint8_t preamble_[4];
			preamble_[0] = (uint8_t) 0x00;
			preamble_[1] = (uint8_t) 0x00;

			IT8951WaitForReady(bus, type);
			//e.g. - Set GPIO 0~7 to Output mode
			//See your host setting of GPIO
			//GPIO_I80_Bus[16] = usData;

		    //Switch C/D to Data => Data - H
			//GPIO_SET_H(CD);
		    //CS-L
		    //GPIO_SET_L(CS);
		    gpio->set(spi->cs_gpio, 0);
		    //WR Enable
		    //GPIO_SET_L(WEN);
		    //gpio->set(i80_ref->hwe_n_gpio, 0);
		    //Set 16 bits Bus Data
		    //See your host setting of GPIO
		    iResult = write(spi->fd, &usData, 1);

		    //WR Enable - H
		    //GPIO_SET_H(WEN);
		    //gpio->set(i80_ref->hwe_n_gpio, 1);
		    //CS-H
		    //GPIO_SET_H(CS);
		    gpio->set(spi->cs_gpio, 1);

			}
		else if(*type == I80){
			pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
			struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

			IT8951WaitForReady(bus, type);
			//e.g. - Set GPIO 0~7 to Output mode
			//See your host setting of GPIO
			//GPIO_I80_Bus[16] = usData;

		    //Switch C/D to Data => Data - H
			//GPIO_SET_H(CD);
			gpio->set(i80->hdc_gpio, 1);
		    //CS-L
		    //GPIO_SET_L(CS);
		    gpio->set(i80->hcs_n_gpio, 0);
		    //WR Enable
		    //GPIO_SET_L(WEN);
		    //gpio->set(i80_ref->hwe_n_gpio, 0);
		    //Set 16 bits Bus Data
		    //See your host setting of GPIO
		    iResult = write(i80->fd, &usData, 1);

		    //WR Enable - H
		    //GPIO_SET_H(WEN);
		    //gpio->set(i80_ref->hwe_n_gpio, 1);
		    //CS-H
		    //GPIO_SET_H(CS);
		    gpio->set(i80->hcs_n_gpio, 1);
			}
		else{
				//error
			}
}
//-------------------------------------------------------------------
//Host controller Read Data for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static TWord* gpio_i80_16b_data_in(pl_generic_interface_t *bus, enum interfaceType *type, int size)
{

	TWord usData;
	//int iResult = 0;
	TWord* iResult = (TWord*) malloc(size*sizeof(TWord));


	if(*type == SPI_HRDY){
			pl_spi_hrdy_t *spi = (pl_spi_hrdy_t*) bus ->hw_ref;
			spi->mSpi = bus->mSpi;
			struct pl_gpio * gpio = (struct pl_gpio *) spi->hw_ref;

			TByte preamble_[2];
			preamble_[0] = (TByte) 0x10;
			preamble_[1] = (TByte) 0x00;

			// open SPI Bus
			int stat = -EINVAL;
			stat = bus->open(spi);

			//Set SPI-CS low
			gpio->set(spi->cs_gpio, 0);

			//send SPI read data preamble
			stat = bus->write_bytes(bus, preamble_, 2);

			//Loop through the various data
			int i = 0;
			for(i=0; i<(size+1); i++){
				// throw away first read, as this only contains rubbish (as documented by ITE)
				if(i==0){
					IT8951WaitForReady(bus, type);
					bus->read_bytes(spi, &usData, sizeof(TWord));
				}
				//next read will give the value that you want to receive
				else{
					IT8951WaitForReady(bus, type);
					bus->read_bytes(spi, &usData, sizeof(TWord));
					iResult[i-1] = swap_endianess(usData);

				}
				//iResult = read(spi->fd, &usData, );
			}

			//Set SPI-CS high
			gpio->set(spi->cs_gpio, 1);
			}
		else if(*type == I80){
			pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
			struct pl_gpio * gpio = (struct pl_gpio *) i80->hw_ref;

			// to go into read mode
			// iResult = read(i80_ref->fd, &usData, 1);

			IT8951WaitForReady(bus, type);
			//Set GPIO 0~7 to input mode
			//See your host setting of GPIO
			//Switch C/D to Data - DATA - H
			//GPIO_SET_H(CD);
			gpio->set(i80->hdc_gpio, 1);
			//CS-L
			//GPIO_SET_L(CS);
			gpio->set(i80->hcs_n_gpio, 0);
			//RD Enable
			//GPIO_SET_L(REN);
			//gpio->set(i80_ref->hrd_n_gpio, 0);
			//Get 8-bits Bus Data (Collect 8 GPIO pins to Byte Data)
			//See your host setting of GPIO
			//usData = GPIO_I80_Bus[16];

			//
			int i = 0;
			for(i=0; i<size; i++){
				read(i80->fd, &usData, 1);
				iResult [i] = usData;
			}

			//WR Enable - H
			//GPIO_SET_H(WEN);
			//gpio->set(i80_ref->hrd_n_gpio, 1);
			//CS-H
			//GPIO_SET_H(CS);
			gpio->set(i80->hcs_n_gpio, 1);
			}
		else{
				//error
			}

    //return usData;
	return iResult;
}

static void swap_data(TWord *buff, int size)
{
	int i = 0;
	TWord tmp = 0;

	for(i = 0; i< size; i++)
	{
		tmp = buff[i];

		buff[i] = 		    (( tmp & 0x0080 ) << 1 );
		buff[i] = buff[i] | (( tmp & 0x0040 ) << 3);
		buff[i] = buff[i] | (( tmp & 0x0020 ) << 7);
		buff[i] = buff[i] | (( tmp & 0x0010 ) << 6);
		buff[i] = buff[i] | (( tmp & 0x0008 ) << 10);
		buff[i] = buff[i] | (( tmp & 0x0004 ) << 9);
		buff[i] = buff[i] | (( tmp & 0x0002 ) << 13);
		buff[i] = buff[i] | (( tmp & 0x0001 ) << 15);

		buff[i] = buff[i] | (( tmp & 0x8000 ) >> 15);
		buff[i] = buff[i] | (( tmp & 0x4000 ) >> 13);
		buff[i] = buff[i] | (( tmp & 0x2000 ) >> 11);
		buff[i] = buff[i] | (( tmp & 0x1000 ) >> 9);
		buff[i] = buff[i] | (( tmp & 0x0800 ) >> 7);
		buff[i] = buff[i] | (( tmp & 0x0400 ) >> 5);
		buff[i] = buff[i] | (( tmp & 0x0200 ) >> 3);
		buff[i] = buff[i] | (( tmp & 0x0100 ) >> 1);
	}

	//printf("swap: 0x%x | 0x%x --> 0x%x | 0x%x\n", tmp[0], tmp[1], buff[i*2], buff[i*2+1]);
}

TWord swap_endianess(TWord in){

	uint8_t buff[2];

	buff[0] = (uint8_t)  in;
	buff[1] = (uint8_t) (in >> 8);

	TWord out;

	out =        buff[1];
	out = out | (buff[0] << 8);

	return out;
}

static TWord swap_data_in(TWord in)
{
	int i = 0;
	uint8_t buff[2];
	uint8_t tmp[2];

	buff[0] = (uint8_t)  in;
	buff[1] = (uint8_t) (in >> 8);

	TWord out;

	tmp[0] = buff[0];
	tmp[1] = buff[1];

	tmp[0] = buff[i*2];
	tmp[1] = buff[i*2+1];

	buff[i*2+1] = 		    (( tmp[1] & 0x80 ) >> 7);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x40 ) >> 5);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x20 ) >> 2);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x10 ) << 1);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x08 ) >> 1);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x04 ) << 2);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x02 ) << 5);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x01 ) << 7);

	buff[i*2] = 		        (( tmp[0] & 0x80 ) >> 7);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x40 ) >> 5);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x20 ) >> 3);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x10 ) >> 1);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x08 ) << 1);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x04 ) << 3);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x02 ) << 5);
	buff[i*2] = buff[i*2] | (( tmp[0] & 0x01 ) << 7);

	TWord value = (buff[i*2] << 8) | buff[i*2+1];

	out =        buff[1];
	out = out | (buff[0] << 8);

	printf("swap: 0x%x --> 0x%x --> %d\n", in, out, out);

	return out;
}
