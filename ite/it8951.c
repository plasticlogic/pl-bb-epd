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


static TWord swap_data(TWord in);
static TWord swap_data_in(TWord in);



it8951_t *it8951_new(struct pl_gpio *gpios, struct pl_generic_interface *interface, struct pl_i2c *i2c, const struct it8951_pins *pins)
{
	assert(gpios != NULL);
	assert(pins != NULL);
	assert(interface != NULL);

	it8951_t *p = (it8951_t *)malloc(sizeof(it8951_t));
	p->gpio = gpios;
	p->interface =  interface;
	p->pins =  pins;

	return p;
}

void GetIT8951SystemInfo(struct pl_i80 *p, void* pBuf)
{
	int i = 0;

    TWord* pusWord = (TWord*)pBuf;
    I80IT8951DevInfo* pstDevInfo;
    //Send I80 CMD
    LCDWriteCmdCode(p, USDEF_I80_CMD_GET_DEV_INFO);
//    #ifdef EN_SPI_2_I80
//
//    //Burst Read Request for SPI interface only
//    LCDReadNData(pusWord, sizeof(I80IT8951DevInfo)/2);//Polling HRDY for each words(2-bytes) if possible
//
//    #else
    //I80 interface - Single Read available
    for(i=0; i<sizeof(I80IT8951DevInfo)/2; i++)
    {
        pusWord[i] = LCDReadData(p);
    }

//    #endif

    //Show Device information of IT8951
    pstDevInfo = (I80IT8951DevInfo*)pBuf;
    printf("Panel(W,H) = (%d,%d)\r\n",
    pstDevInfo->usPanelW, pstDevInfo->usPanelH );
    printf("Image Buffer Address = %X\r\n",
    pstDevInfo->usImgBufAddrL | (pstDevInfo->usImgBufAddrH << 16));
    //Show Firmware and LUT Version
    //printf("FW Version = %s\r\n", stI80IT8951DevInfo.usFWVersion);
    //printf("LUT Version = %s\r\n", stI80IT8951DevInfo.usLUTVersion);
}

//-----------------------------------------------------------
//Initial function 2 ¡V Set Image buffer base address
//-----------------------------------------------------------
void IT8951SetImgBufBaseAddr(struct pl_i80 *p, TDWord ulImgBufAddr)
{
    TWord usWordH = (TWord)((ulImgBufAddr >> 16) & 0x0000FFFF);
    TWord usWordL = (TWord)( ulImgBufAddr & 0x0000FFFF);
    //Write LISAR Reg
    IT8951WriteReg(p, LISAR + 2 ,usWordH);
    IT8951WriteReg(p, LISAR ,usWordL);
}

//-----------------------------------------------------------
//Host Cmd 5 - REG_WR
//-----------------------------------------------------------
void IT8951WriteReg(struct pl_i80 *p, TWord usRegAddr,TWord usValue)
{
    //I80 Mode
    //Send Cmd , Register Address and Write Value
    LCDWriteCmdCode(p, IT8951_TCON_REG_WR);
    LCDWriteData(p, usRegAddr);
    LCDWriteData(p, usValue);
}

//-----------------------------------------------------------
//Display function 2 ¡V Load Image Area process
//-----------------------------------------------------------
void IT8951HostAreaPackedPixelWrite(struct pl_i80 *p, IT8951LdImgInfo* pstLdImgInfo, IT8951AreaImgInfo* pstAreaImgInfo)
{
    TDWord i,j;
    //Source buffer address of Host
    TWord* pusFrameBuf = (TWord*)pstLdImgInfo->ulStartFBAddr;

//	#if 0
//	//if width or height over than 2048 use memburst write instead, only allow 8bpp data
//    IT8951MemBurstWriteProc(pstLdImgInfo->ulImgBufBaseAddr,  pstAreaImgInfo->usWidth/2* pstAreaImgInfo->usHeight,   pusFrameBuf); //MemAddr, Size, Framebuffer address
//	#else
    //Set Image buffer(IT8951) Base address
    IT8951SetImgBufBaseAddr(p, pstLdImgInfo->ulImgBufBaseAddr);
    //Send Load Image start Cmd
    IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);
    //Host Write Data
    for(j=0;j< pstAreaImgInfo->usHeight;j++)
    {
//    	#ifdef __SPI_2_I80_INF__
//
//            //Write 1 Line for each SPI transfer
//            LCDWriteNData(pusFrameBuf, pstAreaImgInfo->usWidth/2);
//            pusFrameBuf += pstAreaImgInfo->usWidth/2;//Change to Next line of loaded image (supposed the Continuous image content in hsot frame buffer )
//
//        #else

        for(i=0;i< pstAreaImgInfo->usWidth/2;i++)
        {
            //Write a Word(2-Bytes) for each time
            LCDWriteData(p, *pusFrameBuf);
            pusFrameBuf++;
        }
        //#endif
    }
    //Send Load Img End Command
    IT8951LoadImgEnd();
//	#endif
}

//-----------------------------------------------------------
//Host controller function 1 ¡V Wait for host data Bus Ready
//-----------------------------------------------------------
void LCDWaitForReady(struct pl_i80 *p)
{
	//Regarding to HRDY
	//you may need to use a GPIO pin connected to HRDY of IT8951
//    TDWord ulData = HRDY;
//    while(ulData == 0)
//    {
//        //Get status of HRDY
//        ulData = HRDY;
//    }

	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;
	int i = 0;

	while(i++ < WAIT_FOR_READY_TIMEOUT_I80)
	{
			if(gpio->get(p->hrdy_gpio) == 1)
			{
				return;
			}
	}
}

//-----------------------------------------------------------------
//Host controller function 2 ¡V Write command code to host data Bus
//-----------------------------------------------------------------
static void LCDWriteCmdCode(struct pl_i80 *p, TWord usCmdCode)
{
    //wait for ready
    LCDWaitForReady(p);
    // swap data
    usCmdCode = swap_data(usCmdCode);
    //write cmd code
    gpio_i80_16b_cmd_out(p, usCmdCode);
}

//-----------------------------------------------------------
//Host controller function 3 ¡V Write Data to host data Bus
//-----------------------------------------------------------
static void LCDWriteData(struct pl_i80 *p, TWord usData)
{
    //wait for ready
    LCDWaitForReady(p);
    // swap data
    usData = swap_data(usData);
    //write data
    gpio_i80_16b_data_out(p, usData);
}

//-----------------------------------------------------------
//Host controller function 4 ¡V Read Data from host data Bus
//-----------------------------------------------------------
static TWord LCDReadData(struct pl_i80 *p)
{
    TWord usData;
    //wait for ready
    LCDWaitForReady(p);
    //read data from host data bus
    usData = gpio_i80_16b_data_in(p);
    // swap data
    usData = swap_data_in(usData);
    return usData;
}

//-------------------------------------------------------------------
//Host controller Write command code for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static void gpio_i80_16b_cmd_out(struct pl_i80 *i80_ref, TWord usCmd)
{
	int iResult = 0;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

    LCDWaitForReady(i80_ref);
    //Set GPIO 0~7 to Output mode
    //See your host setting of GPIO
    //Switch C/D to CMD => CMD - L
    //GPIO_SET_L(CD);
    gpio->set(i80_ref->hdc_gpio, 0);
    //CS-L
    //GPIO_SET_L(CS);
    gpio->set(i80_ref->hcs_n_gpio, 0);
    //WR Enable
    //GPIO_SET_L(WEN);
    gpio->set(i80_ref->hwe_n_gpio, 0);
    //Set Data output (Parallel output request)
    //See your host setting of GPIO
    //GPIO_I80_Bus[16] = usCmd;
    iResult = write(i80_ref->fd, &usCmd, 1);

    //WR Enable - H
    //GPIO_SET_H(WEN);
    gpio->set(i80_ref->hwe_n_gpio, 1);

    //CS-H
    //GPIO_SET_H(CS);
    gpio->set(i80_ref->hcs_n_gpio, 1);

}
//-------------------------------------------------------------------
//Host controller Write Data for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static void gpio_i80_16b_data_out(struct pl_i80 *i80_ref, TWord usData)
{
	int iResult = 0;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

    LCDWaitForReady(i80_ref);
    //e.g. - Set GPIO 0~7 to Output mode
    //See your host setting of GPIO
    //GPIO_I80_Bus[16] = usData;

    //Switch C/D to Data => Data - H
    //GPIO_SET_H(CD);
    gpio->set(i80_ref->hdc_gpio, 1);
    //CS-L
    //GPIO_SET_L(CS);
    gpio->set(i80_ref->hcs_n_gpio, 0);
    //WR Enable
    //GPIO_SET_L(WEN);
    gpio->set(i80_ref->hwe_n_gpio, 0);
    //Set 16 bits Bus Data
    //See your host setting of GPIO
    iResult = write(i80_ref->fd, &usData, 1);

    //WR Enable - H
    //GPIO_SET_H(WEN);
    gpio->set(i80_ref->hwe_n_gpio, 1);
    //CS-H
    //GPIO_SET_H(CS);
    gpio->set(i80_ref->hcs_n_gpio, 1);
}
//-------------------------------------------------------------------
//Host controller Read Data for 16 bits using GPIO simulation
//-------------------------------------------------------------------
static TWord gpio_i80_16b_data_in(struct pl_i80 *i80_ref)
{
    TWord usData;

	int iResult = 0;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

	// to go into read mode
	// iResult = read(i80_ref->fd, &usData, 1);

    LCDWaitForReady(i80_ref);
    //Set GPIO 0~7 to input mode
    //See your host setting of GPIO
    //Switch C/D to Data - DATA - H
    //GPIO_SET_H(CD);
    gpio->set(i80_ref->hdc_gpio, 1);
    //CS-L
    //GPIO_SET_L(CS);
    gpio->set(i80_ref->hcs_n_gpio, 0);
    //RD Enable
    //GPIO_SET_L(REN);
    gpio->set(i80_ref->hrd_n_gpio, 0);
    //Get 8-bits Bus Data (Collect 8 GPIO pins to Byte Data)
    //See your host setting of GPIO
    //usData = GPIO_I80_Bus[16];
    iResult = read(i80_ref->fd, &usData, 1);

    //WR Enable - H
    //GPIO_SET_H(WEN);
    gpio->set(i80_ref->hrd_n_gpio, 1);
    //CS-H
    //GPIO_SET_H(CS);
    gpio->set(i80_ref->hcs_n_gpio, 1);

    return usData;
}

static TWord swap_data(TWord in)
{
	int i = 0;
	uint8_t buff[2];
	uint8_t tmp[2];

	buff[1] = (uint8_t)  in;
	buff[0] = (uint8_t) (in >> 8);

	TWord out;

	tmp[0] = buff[0];
	tmp[1] = buff[1];

	buff[i*2+1] = 		    (( tmp[1] & 0x80 ) >> 7);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x40 ) >> 5);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x20 ) >> 1);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x10 ) >> 2);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x08 ) << 2);
	buff[i*2+1] = buff[i*2+1] | (( tmp[1] & 0x04 ) << 1);
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

	out =        buff[0];
	out = out | (buff[1] << 8);

	//printf("swap: 0x%x | 0x%x --> 0x%x | 0x%x\n", tmp[0], tmp[1], buff[i*2], buff[i*2+1]);

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
