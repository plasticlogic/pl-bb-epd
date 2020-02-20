/*
  Plastic Logic EPD project on BeagleBone

  Copyright (C) 2020 Plastic Logic

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
 * it8951_controller.c
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#include <unistd.h>

#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#include <pl/hv.h>
#include <pl/generic_interface.h>
#include <pl/i80.h>
#include <pl/gpio.h>
#define LOG_TAG "it8951_controller"
#include "pl/utils.h"

#include "ite/it8951_controller.h"

static const struct pl_wfid wf_table[] = {
	{ "default",	   2 },
	{ "0",             0 },
	{ "1",             1 },
	{ "2",             2 },
	{ "3",             3 },
	{ "4",             4 },
	{ "5",             5 },
	{ "6",             6 },
	{ "7",             7 },
	{ "8",             8 },
	{ "9",             9 },
	{ "10",           10 },
	{ "11",           11 },
	{ "12",           12 },
	{ "13",           13 },
	{ "14",           14 },
	{ "15",           15 },
	{ NULL,           -1 }
};

static int trigger_update(struct pl_generic_controller *controller);
static int clear_update(pl_generic_controller_t *p);
static int init_controller(struct pl_generic_controller *controller, int use_wf_from_nvm);
static int configure_update(struct pl_generic_controller *controller, int wfid, enum pl_update_mode mode, const struct pl_area *area);
static int fill(struct pl_generic_controller *controller, const struct pl_area *a, uint8_t grey);
static int load_wflib(struct pl_generic_controller *controller, const char *filename);
static int load_png_image(struct pl_generic_controller *controller, const char *path, const struct pl_area *area, int left,	int top);
static int wait_update_end(struct pl_generic_controller *controller);
static int read_register(struct pl_generic_controller *controller, const regSetting_t* setting);
static int write_register(struct pl_generic_controller *controller, const regSetting_t setting, const uint32_t bitmask);
static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting);
static int set_registers(struct pl_generic_controller *controller, const regSetting_t* map, int n);
static int set_temp_mode(struct pl_generic_controller *controller, enum pl_epdc_temp_mode mode);
static int update_temp(struct pl_generic_controller *controller);

int it8951_controller_setup(struct pl_generic_controller *controller, it8951_t *it8951)
{
	assert(controller != NULL);
	assert(it8951 != NULL);

	controller->wait_update_end = wait_update_end;
	controller->load_wflib = load_wflib;
	controller->fill = fill;
	controller->load_image = load_png_image;

	controller->configure_update = configure_update;
	controller->trigger_update = trigger_update;
	controller->clear_update = clear_update;
	controller->read_register = read_register;
	controller->write_register = write_register;
	controller->send_cmd = send_cmd;

	controller->init = init_controller;
	controller->wf_table = wf_table;
	controller->hw_ref = it8951;

	controller->set_temp_mode = set_temp_mode;
	controller->update_temp = update_temp;

	return 0;
}


static int trigger_update(struct pl_generic_controller *controller)
{
	return 0;
}

static int clear_update(pl_generic_controller_t *p)
{
	return 0;
}

static int init_controller(struct pl_generic_controller *controller, int use_wf_from_nvm)
{
	it8951_t *it8951 = controller->hw_ref;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;

	pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
	struct pl_gpio *gpio = (struct pl_gpio *) i80->hw_ref;

	uint8_t data_out [2];
	uint8_t data_in [40];

	data_out[0] = 0x03;
	data_out[1] = 0x02;

	gpio->set(i80->hdc_gpio, 0);

	bus->write_bytes(bus, data_out, sizeof(data_out));

	bus->read_bytes(bus, data_in, sizeof(data_in));

	// does the same again - just for confirmation

	I80IT8951DevInfo devInfo;
	GetIT8951SystemInfo(i80, &devInfo);

	return 0;
}

static int configure_update(struct pl_generic_controller *controller, int wfid, enum pl_update_mode mode, const struct pl_area *area)
{
	return 0;
}

static int fill(struct pl_generic_controller *controller, const struct pl_area *a, uint8_t grey)
{
	return 0;
}

static int load_wflib(struct pl_generic_controller *controller, const char *filename)
{
	return 0;
}

static int load_png_image(struct pl_generic_controller *controller, const char *path, const struct pl_area *area, int left,	int top)
{
	it8951_t *it8951 = controller->hw_ref;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;

	pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
	struct pl_gpio *gpio = (struct pl_gpio *) i80->hw_ref;

	TDWord gulImgBufAddr;
	TByte* gpFrameBuf;

    IT8951LdImgInfo stLdImgInfo;
    IT8951AreaImgInfo stAreaImgInfo;



	//Host Init
	//------------------------------------------------------------------
    //Get Device Info
	I80IT8951DevInfo devInfo;
	GetIT8951SystemInfo(i80, &devInfo);

    //Get Image Buffer Address of IT8951
    gulImgBufAddr = devInfo.usImgBufAddrL | (devInfo.usImgBufAddrH << 16);

    //Set to Enable I80 Packed mode
    IT8951WriteReg(i80, I80CPCR, 0x0001);
    //-------------------------------------------------------------------

    int width = 0;
    int height = 0;

    if(0)
    {
        //Host Frame Buffer allocation
        gpFrameBuf = malloc(devInfo.usPanelW * devInfo.usPanelH);
		//Write pixel 0xF0(White) to Frame Buffer
		memset(gpFrameBuf, 0xff, devInfo.usPanelW * devInfo.usPanelH);
    }
    else
    {
		if (read_png(path, &gpFrameBuf, &width, &height))
			return -ENOENT;
	}

    //Check TCon is free ? Wait TCon Ready (optional)
    IT8951WaitForDisplayReady(i80);

    //--------------------------------------------------------------------------------------------
    //      initial display - Display white only
    //--------------------------------------------------------------------------------------------
    //Load Image and Display
    //Setting Load image information
    stLdImgInfo.ulStartFBAddr    = (TDWord)gpFrameBuf;
    stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
    stLdImgInfo.usPixelFormat    = IT8951_8BPP;
    stLdImgInfo.usRotate         = IT8951_ROTATE_0;
    stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
    //Set Load Area
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = devInfo.usPanelW;
    stAreaImgInfo.usHeight = devInfo.usPanelH;

    //Load Image from Host to IT8951 Image Buffer
    IT8951HostAreaPackedPixelWrite(i80, &stLdImgInfo, &stAreaImgInfo);//Display function 2
    //Display Area ¡V (x,y,w,h) with mode 0 for initial White to clear Panel
    IT8951DisplayArea(i80, 0,0, devInfo.usPanelW, devInfo.usPanelH, 2);

    if(gpFrameBuf)
    	free(gpFrameBuf);

	return 0;
}

static int wait_update_end(struct pl_generic_controller *controller)
{
	return 0;
}

static int read_register(struct pl_generic_controller *controller, const regSetting_t* setting)
{
	return 0;
}

static int write_register(struct pl_generic_controller *controller, const regSetting_t setting, const uint32_t bitmask)
{
	return 0;
}

static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting)
{
	return 0;
}

static int set_registers(struct pl_generic_controller *controller, const regSetting_t* map, int n)
{
	return 0;
}

static int set_temp_mode(struct pl_generic_controller *controller, enum pl_epdc_temp_mode mode)
{
	return 0;
}

static int update_temp(struct pl_generic_controller *controller)
{
	return 0;
}

