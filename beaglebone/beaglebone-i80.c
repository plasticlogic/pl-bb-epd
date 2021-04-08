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
 * beaglebone_i80.c
 *
 *  Created on: 24.01.2020
 *      Author: matti.haugwitz
 */

#include <beaglebone/beaglebone-i80.h>
#include <beaglebone/beaglebone-parallel.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <src/pindef.h>

// function prototypes
static int i80_close(struct pl_parallel *p);
static int i80_read_bytes(struct pl_parallel *p, uint8_t *buff, size_t size);
static int i80_write_bytes(struct pl_parallel *p, uint8_t *buff, size_t size);
static int i80_set_cs(struct pl_parallel *p, uint8_t cs);
static int i80_init(struct pl_parallel *p);
static void delete(struct pl_parallel *p);
static int wait_for_ready(struct pl_i80 *p);
static void swap_data(uint8_t *buff, size_t size);
static void swap_data_in(uint8_t *buff, size_t size);

static void LCDWaitForReady(struct pl_i80 *p);
static void LCDWriteCmdCode(struct pl_i80 *p, TWord usCmdCode);
static void LCDWriteData(struct pl_i80 *p, TWord usData);
static TWord LCDReadData(struct pl_i80 *p);
static void gpio_i80_16b_cmd_out(struct pl_i80 *i80_ref, TWord usCmd);
static void gpio_i80_16b_data_out(struct pl_i80 *i80_ref, TWord usData);
static TWord gpio_i80_16b_data_in(struct pl_i80 *i80_ref);

/**
 * allocates, configures and returns a new pl_spi structure
 *
 * @param spi_channel number of the spi device /dev/spidev?.0
 * @return pl_spi structure
 */
pl_parallel_t *beaglebone_i80_new(struct pl_gpio * hw_ref){
	pl_parallel_t *parellel_ref = (pl_parallel_t *)malloc(sizeof(pl_parallel_t));
	pl_i80_t *i80_ref = (pl_i80_t *)malloc(sizeof(pl_i80_t));

	i80_ref->hw_ref = hw_ref;
	parellel_ref->hw_ref = i80_ref;

	parellel_ref->open = i80_init;
	parellel_ref->close = i80_close;
	parellel_ref->read_bytes = i80_read_bytes;
	parellel_ref->write_bytes = i80_write_bytes;
	parellel_ref->set_cs = i80_set_cs;
	parellel_ref->delete = delete;

	return parellel_ref;
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
static int i80_init(struct pl_parallel *p)
{
	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	i80_ref->fd = open("/dev/parallel", O_RDWR);
	if(p->fd < 0){
		return FALSE;
	}

	return TRUE;
}

/**
 * closes the parallel bus
 *
 * @param p pl_i80 structure
 * @return status
 */
static int i80_close(struct pl_parallel *p)
{
	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	if(i80_ref->fd)
		close(i80_ref->fd);
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
static int i80_read_bytes(struct pl_parallel *p, uint8_t *buff, size_t size){

	int iResult = 0;
	int i = 0;
	TWord buff_;

	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	for(i=0; i < size/2; i++)
	{
		printf("read word %d start ...", i);
		buff_ = LCDReadData(i80_ref);
		buff[i*2+1]   = (uint8_t) ((buff_ & 0xff00) >> 8);
		buff[i*2] = (uint8_t) ((buff_ & 0x00ff));
		printf("read word end\n");
	}

	swap_data_in(buff, size);

	return iResult;
}

//static int i80_read_bytes(struct pl_parallel *p, uint8_t *buff, size_t size){
//
//	int iResult = 0;
//
//	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;
//
//	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;
//
//	if(wait_for_ready(i80_ref))
//	  return -1;
//
//	//Switch C/D to Data - DATA - H
//	//done outside
//
//	//CS-L
//	gpio->set(i80_ref->hcs_n_gpio, 0);
//
//	//RD Enable -L
//	gpio->set(i80_ref->hrd_n_gpio, 0);
//
//	//Get 8-bits Bus Data (Collect 8 GPIO pins to Byte Data)
//	iResult = read(i80_ref->fd, buff, size/2);
//
//	//RD Disable -H
//	gpio->set(i80_ref->hrd_n_gpio, 1);
//
//	usleep(10);
//
//	//CS-H
//	gpio->set(i80_ref->hcs_n_gpio, 1);
//
//	// necessary due to temporary reverse connection of parallel bus
//	swap_data(buff, size);
//
//	return iResult;
//}

/**
 * write bytes from buffer to the i80 bus
 *
 * @param p pl_i80 structure
 * @param buff pointer to the buffer
 * @param size size of the buffer
 * @return status
 */
static int i80_write_bytes(struct pl_parallel *p, uint8_t *buff, size_t size)
{
	int iResult = 0;
	int i = 0;
	TWord buff_;

	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

	swap_data(buff, size);

	if(gpio->get(i80_ref->hdc_gpio) == 0)
	{
		buff_ = 		 buff[0];
		buff_ = buff_ | (buff[1] << 8);
		LCDWriteCmdCode(i80_ref, buff_);
	}
	else if(gpio->get(i80_ref->hdc_gpio) == 1)
	{
		for(i=0; i < size/2; i++)
		{

			buff_ = 		 buff[i*2];
			buff_ = buff_ | (buff[i*2+1] << 8);
			LCDWriteData(i80_ref, buff_);

		}
	}

	return iResult;
}

//static int i80_write_bytes(struct pl_parallel *p, uint8_t *buff, size_t size)
//{
//	int iResult = 0;
//
//	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;
//
//	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;
//
//	if(wait_for_ready(i80_ref))
//	  return -1;
//
//	// necessary due to temporary reverse connection of parallel bus
//	swap_data(buff, size);
//
//	//Switch C/D --> command = L / data = H
//	//done outside
//
//	//CS-L
//	gpio->set(i80_ref->hcs_n_gpio, 0);
//
//	if(wait_for_ready(i80_ref))
//	  return -1;
//
//	usleep(10);
//
//	//WR Enable -L
//	gpio->set(i80_ref->hwe_n_gpio, 0);
//
//	usleep(10);
//
//	//Set Data output (Parallel output request)
//	iResult = write(i80_ref->fd, buff, size/2);
//
//	if(wait_for_ready(i80_ref))
//	  return -1;
//
//	usleep(10);
//
//	//WR Disable-H
//	gpio->set(i80_ref->hwe_n_gpio, 1);
//
//	usleep(10);
//
//	//CS-H
//	gpio->set(i80_ref->hcs_n_gpio, 1);
//
//	return iResult;
//}

/**
 * set chip select gpio
 *
 * @param p pl_i80 structure
 * @return status
 */
static int i80_set_cs(struct pl_parallel *p, uint8_t cs)
{
	pl_i80_t * i80_ref = (pl_i80_t*) p->hw_ref;

	struct pl_gpio * gpio = (struct pl_gpio *) i80_ref->hw_ref;

	gpio->set(i80_ref->hcs_n_gpio, cs);

	return 0;
}

/**
 *
 */
static int wait_for_ready(struct pl_i80 *p)
{
	struct pl_gpio * gpio = (struct pl_gpio *) p->hw_ref;
	int i = 0;

	while(i++ < WAIT_FOR_READY_TIMEOUT_I80)
	{
			if(gpio->get(p->hrdy_gpio) == 1)
			{
				return 0;
			}
	}
	return -1;
}

static void swap_data(uint8_t *buff, size_t size)
{
	int i = 0;
	uint8_t tmp[2];

	for(i=0; i< size/2; i++)
	{
		tmp[0] = buff[i*2];
		tmp[1] = buff[i*2+1];

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

		//printf("swap: 0x%x | 0x%x --> 0x%x | 0x%x\n", tmp[0], tmp[1], buff[i*2], buff[i*2+1]);
	}
}

static void swap_data_in(uint8_t *buff, size_t size)
{
	int i = 0;
	uint8_t tmp[2];

	for(i=0; i< size/2; i++)
	{
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

		printf("swap: 0x%x | 0x%x --> 0x%x | 0x%x --> %d\n", tmp[0], tmp[1], buff[i*2], buff[i*2+1], value);
	}
}

//-----------------------------------------------------------
//Host controller function 1 ¡V Wait for host data Bus Ready
//-----------------------------------------------------------
static void LCDWaitForReady(struct pl_i80 *p)
{
	//Regarding to HRDY
	//you may need to use a GPIO pin connected to HRDY of IT8951
//    TDWord ulData = HRDY;
//    while(ulData == 0)
//    {
//        //Get status of HRDY
//        ulData = HRDY;
//    }

	wait_for_ready(p);
}

//-----------------------------------------------------------------
//Host controller function 2 ¡V Write command code to host data Bus
//-----------------------------------------------------------------
static void LCDWriteCmdCode(struct pl_i80 *p, TWord usCmdCode)
{
    //wait for ready
    LCDWaitForReady(p);
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
    //gpio->set(i80_ref->hwe_n_gpio, 0);
    //Set Data output (Parallel output request)
    //See your host setting of GPIO
    //GPIO_I80_Bus[16] = usCmd;
    iResult = write(i80_ref->fd, &usCmd, 1);

    //WR Enable - H
    //GPIO_SET_H(WEN);
    //gpio->set(i80_ref->hwe_n_gpio, 1);

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
    //gpio->set(i80_ref->hwe_n_gpio, 0);
    //Set 16 bits Bus Data
    //See your host setting of GPIO
    iResult = write(i80_ref->fd, &usData, 1);

    //WR Enable - H
    //GPIO_SET_H(WEN);
    //gpio->set(i80_ref->hwe_n_gpio, 1);
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
    //gpio->set(i80_ref->hrd_n_gpio, 0);
    //Get 8-bits Bus Data (Collect 8 GPIO pins to Byte Data)
    //See your host setting of GPIO
    //usData = GPIO_I80_Bus[16];
    iResult = read(i80_ref->fd, &usData, 1);

    //WR Enable - H
    //GPIO_SET_H(WEN);
    //gpio->set(i80_ref->hrd_n_gpio, 1);
    //CS-H
    //GPIO_SET_H(CS);
    gpio->set(i80_ref->hcs_n_gpio, 1);

    return usData;
}


