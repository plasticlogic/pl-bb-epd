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
#include <string.h>

#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#include <pl/hv.h>
#include <pl/generic_interface.h>
#include <pl/types.h>
#include <pl/i80.h>
#include <pl/gpio.h>
#define LOG_TAG "it8951_controller"
#include "pl/utils.h"

#include "ite/it8951_controller.h"

static const struct pl_wfid wf_table[] = { { "default", 2 }, { "0", 0 }, { "1",
		1 }, { "2", 2 }, { "3", 3 }, { "4", 4 }, { "5", 5 }, { "6", 6 }, { "7",
		7 }, { "8", 8 }, { "9", 9 }, { "10", 10 }, { "11", 11 }, { "12", 12 }, {
		"13", 13 }, { "14", 14 }, { "15", 15 }, { NULL, -1 } };

TByte* fillBuffer;
int modeToUse = 0;
int wfidToUse = 2;
int partialX = 0;
int partialY = 0;
bool clear = false;
I80IT8951DevInfo devInfo;

static int trigger_update(struct pl_generic_controller *controller);
static int clear_update(pl_generic_controller_t *p);
static int init_controller(struct pl_generic_controller *controller,
		int use_wf_from_nvm);
static int configure_update(struct pl_generic_controller *controller, int wfid,
		enum pl_update_mode mode, const struct pl_area *area);
static int fill(struct pl_generic_controller *controller,
		const struct pl_area *a, uint8_t grey);
static int load_wflib(struct pl_generic_controller *controller,
		const char *filename);
static int load_png_image(struct pl_generic_controller *controller,
		const char *path, struct pl_area *area, int left, int top);
static int wait_update_end(struct pl_generic_controller *controller);
static int read_register(struct pl_generic_controller *controller,
		const regSetting_t* setting);
static int write_register(struct pl_generic_controller *controller,
		const regSetting_t setting, const uint32_t bitmask);
static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting);
static int set_registers(struct pl_generic_controller *controller,
		const regSetting_t* map, int n);
static int set_temp_mode(struct pl_generic_controller *controller,
		enum pl_epdc_temp_mode mode);
static int update_temp(struct pl_generic_controller *controller);
static int get_resolution(pl_generic_controller_t *p, int* xres, int* yres);

int it8951_controller_setup(struct pl_generic_controller *controller,
		it8951_t *it8951) {
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
	controller->get_resolution = get_resolution;

	return 0;
}

static int trigger_update(struct pl_generic_controller *controller) {
	//initialize communication structure
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;
	//pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;

	if (modeToUse == 0) {
		controller->imageWidth = controller->xres;
		controller->imageHeight = controller->yres;
		partialX = 0;
		partialY = 0;
	}

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0B); // Power Down Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //DE ????

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0C); // Power Down Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0A); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

//Check if Frame Buffer configuration Mode, when only 1BPP (Bit per Pixel), configure for Black and white update
	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_DPY_AREA);
	IT8951WriteData(bus, type, (TWord) partialX);     				// Display X
	IT8951WriteData(bus, type, (TWord) partialY); 					// Display Y
	IT8951WriteData(bus, type, (TWord) controller->imageWidth); // Display W devInfo.usPanelW 1200
	IT8951WriteData(bus, type, (TWord) controller->imageHeight); // Display H devInfo.usPanelH 960
	IT8951WriteData(bus, type, (TWord) wfidToUse); 				// Display Mode

	//Wait until the Update has ended
	IT8951WaitForDisplayReady(bus, type);

	printf("PMIC Register 7 after update: ");

	regSetting_t reg;
	reg.addr = (int) IT8951_TCON_BYPASS_I2C;
	reg.valCount = (int) 4;
	uint16_t *reg7;
	reg7 = malloc(reg.valCount * sizeof(uint16_t));
	reg7[0] = 0x00;
	reg7[1] = 0x68;
	reg7[2] = 0x07;
	reg7[3] = 0x01;
	reg.val = reg7;

	send_cmd(controller, reg);

	printf("PMIC Register 8 after update: ");

	regSetting_t reg2;
	reg2.addr = IT8951_TCON_BYPASS_I2C;
	reg2.valCount = 4;
	uint16_t *reg8;
	reg8 = malloc(reg.valCount * sizeof(uint16_t));
	reg8[0] = 0x00;
	reg8[1] = 0x68;
	reg8[2] = 0x08;
	reg8[3] = 0x01;
	reg2.val = reg8;

	send_cmd(controller, reg2);

//	printf("PMIC Register B after update: ");
//
//	regSetting_t reg3;
//	reg3.addr = IT8951_TCON_BYPASS_I2C;
//	reg3.valCount = 4;
//	uint16_t *regb;
//	regb = malloc(reg3.valCount * sizeof(uint16_t));
//	regb[0] = 0x00;
//	regb[1] = 0x68;
//	regb[2] = 0x0B;
//	regb[3] = 0x01;
//	reg3.val = regb;
//
//	send_cmd(controller, reg3);
//
//	printf("PMIC Register C after update: ");
//
//	regSetting_t reg4;
//	reg4.addr = IT8951_TCON_BYPASS_I2C;
//	reg4.valCount = 4;
//	uint16_t *regc;
//	regc = malloc(reg4.valCount * sizeof(uint16_t));
//	regc[0] = 0x00;
//	regc[1] = 0x68;
//	regc[2] = 0x0C;
//	regc[3] = 0x01;
//	reg4.val = regc;
//
//	send_cmd(controller, reg4);
//
//	IT8951WaitForReady(bus, type);

	return 0;
}

static int clear_update(pl_generic_controller_t *p) {

	it8951_t *it8951 = p->hw_ref;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	TDWord gulImgBufAddr;
	TByte* gpFrameBuf;
	rgbw_pixel_t *pngBuffer;

	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;

	//Host Init
	//------------------------------------------------------------------
	//Get Device Info

	if (devInfo.usImgBufAddrH == NULL)
		GetIT8951SystemInfo(bus, type, &devInfo);

	//Get Image Buffer Address of IT8951
	gulImgBufAddr = devInfo.usImgBufAddrL | (devInfo.usImgBufAddrH << 16);

	//Set to Enable I80 Packed mode
	IT8951WriteReg(bus, type, I80CPCR, 0x0001);
	//-------------------------------------------------------------------

	p->yres = devInfo.usPanelH;
	p->xres = devInfo.usPanelW;

	int v_yres = 0;
	int v_xres = 0;

	if (!p->display_scrambling) {
		v_xres = p->xres - (2 * p->xoffset);
		v_yres = p->yres - p->yoffset;
	}

	int width = 0;
	int height = 0;

	//Host Frame Buffer allocation
	configure_update(p, 0, 0, NULL);
//	gpFrameBuf = malloc(devInfo.usPanelW * devInfo.usPanelH);
//	//Write pixel 0xF0(White) to Frame Buffer
//	memset(gpFrameBuf, 0xff, devInfo.usPanelW * devInfo.usPanelH);
	width = devInfo.usPanelW;
	height = devInfo.usPanelH;
	p->imageWidth = devInfo.usPanelW;
	p->imageHeight = devInfo.usPanelH;

	stLdImgInfo.ulStartFBAddr = (TDWord) fillBuffer;
	stLdImgInfo.usEndianType = IT8951_LDIMG_L_ENDIAN;
	stLdImgInfo.usPixelFormat = IT8951_8BPP;
	stLdImgInfo.usRotate = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area

	stAreaImgInfo.usX = partialX;
	stAreaImgInfo.usY = partialY;
	stAreaImgInfo.usWidth = width;
	stAreaImgInfo.usHeight = height;

	//Load Image from Host to IT8951 Image Buffer
	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo); //Display function 2
	//Display Area ¡V (x,y,w,h) with mode 0 for initial White to clear Panel

	if (fillBuffer)
		free(fillBuffer);

	trigger_update(p);
	return 0;
}

static int init_controller(struct pl_generic_controller *controller,
		int use_wf_from_nvm) {
	it8951_t *it8951 = controller->hw_ref;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	uint8_t data_out[2];
	uint8_t data_in[40];

	data_out[0] = 0x03;
	data_out[1] = 0x02;

	IT8951WriteData(bus, type, (int) data_out);

	IT8951ReadData(bus, type, (int) data_in);

	//does the same again - just for confirmation

	I80IT8951DevInfo devInfo;
	GetIT8951SystemInfo(bus, type, &devInfo);

	return 0;
}

static int configure_update(struct pl_generic_controller *controller, int wfid,
		enum pl_update_mode mode, const struct pl_area *area) {
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	wfidToUse = wfid;
	modeToUse = mode;

	//Set to Enable I80 Packed mode
	IT8951WriteReg(bus, type, I80CPCR, 0x0001);

	return 0;
}

static int load_wflib(struct pl_generic_controller *controller,
		const char *filename) {
	LOG(
			"Setting Waveform is not supported by EPDC, please contact PL Germany GmbH !");
	return 0;
}

static void memory_padding(uint8_t *source, uint8_t *target,
		int source_gatelines, int source_sourcelines, int target_gatelines,
		int target_sourcelines, int gate_offset, int source_offset) {
	int sourceline, gateline;
	int _gateline_offset = 0;
	int _sourceline_offset = 0;
	uint8_t tempTarget;
	uint8_t tempSource;

	if (gate_offset > 0)
		_gateline_offset = gate_offset;
	else
		_gateline_offset = target_gatelines - source_gatelines;

	if (source_offset > 0)
		_sourceline_offset = source_offset;
	else
		_sourceline_offset = target_sourcelines - source_sourcelines;

	for (gateline = 0; gateline < source_gatelines; gateline++)
		for (sourceline = 0; sourceline < source_sourcelines; sourceline++) {
			int source_index = gateline * source_sourcelines + sourceline;
			int target_index = (gateline + _gateline_offset)
					* target_sourcelines + (sourceline + _sourceline_offset);
			if (!(source_index < 0 || target_index < 0)) {
				target[target_index] = source[source_index];
				source[source_index] = 0x00;
			}
		}
}

static void memory_padding_area(uint8_t *source, uint8_t *target,
		int source_gatelines, int source_sourcelines, int gate_offset,
		int source_offset, struct pl_area* source_area, int top, int left) {
	int sourceline, gateline;
#if VERBOSE
	LOG("%s: source_gatelines %i, source_sourcelines %i, gate_offset %i, source_offset %i, source_area %p, top %i, left %i", __func__, source_gatelines, source_sourcelines, gate_offset, source_offset, source_area, top, left);
	LOG("%s: AREA: L: %i, T: %i, H: %i, W: %i", __func__, source_area->left, source_area->top, source_area->height, source_area->width);
#endif
	for (gateline = source_area->top;
			gateline < source_area->top + source_area->height; gateline++) {
		for (sourceline = source_area->left;
				sourceline < source_area->left + source_area->width;
				sourceline++) {
			int source_index = (gateline/*+source_area->top*/)
					* (source_sourcelines/*+source_area->left*/) + sourceline;
			int target_index = (gateline - source_area->top)
					* source_area->width + (sourceline - source_area->left);
			if (!(source_index < 0 || target_index < 0)) {
				target[target_index] = source[source_index];
				source[source_index] = 0x80;
			}
		}
	}
}

static int load_png_image(struct pl_generic_controller *controller,
		const char *path, struct pl_area *area, int left, int top) {
	it8951_t *it8951 = controller->hw_ref;

	partialX = left;
	partialY = top;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	TDWord gulImgBufAddr;
	TByte* gpFrameBuf;
	rgbw_pixel_t *pngBuffer;

	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;

	//Host Init
	//------------------------------------------------------------------
	//Get Device Info

	if (devInfo.usImgBufAddrH == NULL)
		GetIT8951SystemInfo(bus, type, &devInfo);

	//Get Image Buffer Address of IT8951
	gulImgBufAddr = devInfo.usImgBufAddrL | (devInfo.usImgBufAddrH << 16);

	//Set to Enable I80 Packed mode
	IT8951WriteReg(bus, type, I80CPCR, 0x0001);
	//-------------------------------------------------------------------

	controller->yres = devInfo.usPanelH;
	controller->xres = devInfo.usPanelW;

	int v_yres = 0;
	int v_xres = 0;

	if (!controller->display_scrambling) {
		v_xres = controller->xres - (2 * controller->xoffset);
		v_yres = controller->yres - controller->yoffset;
	}

	int width = 0;
	int height = 0;

	if (clear) {
		//Host Frame Buffer allocation
		//configure_update(controller, 0, 0, area);
		gpFrameBuf = malloc(devInfo.usPanelW * devInfo.usPanelH);
		//Write pixel 0xF0(White) to Frame Buffer
		//memset(gpFrameBuf, 0xff, devInfo.usPanelW * devInfo.usPanelH);
		gpFrameBuf = fillBuffer;
		width = devInfo.usPanelW;
		height = devInfo.usPanelH;
		controller->imageWidth = devInfo.usPanelW;
		controller->imageHeight = devInfo.usPanelH;

	} else if (controller->cfa_overlay.r_position == -1) {
		LOG("BW");

		if (read_png(path, &gpFrameBuf, &width, &height))
			return -ENOENT;
		controller->imageWidth = width;
		controller->imageHeight = height;
		if (area != NULL) {
			area->width = width;
			area->height = height;
		}
		if (height == controller->xres && width == controller->yres) {
			rotate_8bit_image(&height, &width, gpFrameBuf);
		}

	} else {

		LOG("CFA");
//				 read png image
		if (read_rgbw_png(path, &pngBuffer, &width, &height))
			return -ENOENT;

		// apply cfa filter to resolution
		v_xres = (controller->xres - (2 * controller->xoffset)) / 2;
		v_yres = (controller->yres - controller->yoffset) / 2;

		if (!controller->display_scrambling) {
			//*
			if (area) {
				area->left *= 2;
				area->top *= 2;
				area->width *= 2;
				area->height *= 2;
			}
			//*/
			if (height == v_xres && width == v_yres && (height != width)) {
				rotate_rgbw_image(&height, &width, pngBuffer);
				LOG("CFA %ix%i -> %ix%i", height, width, controller->yres,
						controller->xres);
			}
		} else {
			if (controller->display_scrambling & SCRAMBLING_GATE_SCRAMBLE_MASK) {
				v_xres = v_xres * 2;
				v_yres = v_yres / 2;
				if (area) {
					area->top *= 4;
					area->height *= 4;
				}
				if (height == (v_xres) && width == (v_yres)
						&& (height != width)) {
					rotate_rgbw_image(&height, &width, pngBuffer);
				}
			} else if (controller->display_scrambling
					& SCRAMBLING_SOURCE_SCRAMBLE_MASK) {
				v_xres = v_xres / 2;
				v_yres = v_yres * 2;
				if (area) {
					area->left *= 4;
					//area->top /= 2;
					area->width *= 4;
					//area->height /= 2;

				}
				if (height == (v_xres) && width == (v_yres)
						&& (height != width)) {
					rotate_rgbw_image(&height, &width, pngBuffer);
				}
			}
		}
	}

	// scramble image
	TByte* scrambledPNG;
	if (controller->cfa_overlay.r_position == -1 || clear) {
		scrambledPNG = malloc(width * height);
		scramble_array(gpFrameBuf, scrambledPNG, &height, &width,
				controller->display_scrambling);
	} else {
		scrambledPNG =
				malloc(
						4
								* max(height,
										controller->yres) * max(width, controller->xres));

		uint8_t *colorBuffer =
				malloc(
						4
								* max(height,
										controller->yres) * max(width, controller->xres));
		rgbw_processing((uint32_t*) &width, (uint32_t*) &height, pngBuffer,
				colorBuffer, (struct pl_area*) (area) ? NULL : area,
				controller->cfa_overlay);
		scramble_array(colorBuffer, scrambledPNG, &height, &width,
				controller->display_scrambling);
		free(colorBuffer);
		if (pngBuffer)
			free(pngBuffer);
	}

	TByte* targetBuf = malloc(width * height);

//	if (width < controller->xres || height < controller->yres){
//		if (controller->display_scrambling == 0){
//			TByte* targetBufArea = malloc (width * height);
//		}
//	}

	if (controller->display_scrambling == 0) {
		memcpy(targetBuf, scrambledPNG, width * height);

	} else {

		memory_padding(scrambledPNG, targetBuf, height, width, controller->yres,
				controller->xres, controller->yoffset, controller->xoffset);
	}

	//Check TCon is free ? Wait TCon Ready (optional)
	//IT8951WaitForDisplayReady(bus, type);

	//--------------------------------------------------------------------------------------------
	//      initial display - Display white only
	//--------------------------------------------------------------------------------------------
	//Load Image and Display
	//Setting Load image information
	stLdImgInfo.ulStartFBAddr = (TDWord) targetBuf;
	stLdImgInfo.usEndianType = IT8951_LDIMG_L_ENDIAN;
	stLdImgInfo.usPixelFormat = IT8951_8BPP;
	stLdImgInfo.usRotate = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	//ToDo: Pipe x/y position through from console
	stAreaImgInfo.usX = partialX;
	stAreaImgInfo.usY = partialY;
	stAreaImgInfo.usWidth = width;
	stAreaImgInfo.usHeight = height;

	//Load Image from Host to IT8951 Image Buffer
	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo); //Display function 2
	//Display Area ¡V (x,y,w,h) with mode 0 for initial White to clear Panel
	//IT8951DisplayArea(i80, 0,0, devInfo.usPanelW, devInfo.usPanelH, 2);

	if (scrambledPNG)
		free(scrambledPNG);

	if (gpFrameBuf)
		free(gpFrameBuf);

//	if (fillBuffer)
//		free(fillBuffer);

	return 0;
}

static int wait_update_end(struct pl_generic_controller *controller) {
	//initialize communication structure
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	//Poll the TCON Register, to know when the update has finished
	IT8951WaitForDisplayReady(bus, type);
	return 0;
}

static int read_register(struct pl_generic_controller *controller,
		const regSetting_t* setting) {
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	*(setting->val) = IT8951ReadReg(bus, type, setting->addr);

	return 0;
}

static int write_register(struct pl_generic_controller *controller,
		const regSetting_t setting, const uint32_t bitmask) {
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	IT8951_update_reg(bus, type, setting.addr, setting.val, bitmask);

	return 0;
}

static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting) {
	int i = 0;
	it8951_t *it8951 = p->hw_ref;
	struct pl_generic_interface *interface = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	IT8951WriteCmdCode(interface, type, setting.addr);

	for (i = 0; i < setting.valCount; i++)
		IT8951WriteData(interface, type, setting.val[i]);

	//sleep(1);
	usleep(8000);

	if (setting.val[0] == 0x00) {
		TWord* value = IT8951ReadData(interface, type, 1);  //read data
		printf("Data: 0x%x\n", *value);
	}

	return 0;
}

static int set_registers(struct pl_generic_controller *controller,
		const regSetting_t* map, int n) {
	return 0;
}

static int set_temp_mode(struct pl_generic_controller *p,
		enum pl_epdc_temp_mode mode) {

	it8951_t *it8951 = p->hw_ref;
	assert(it8951 != NULL);
	struct pl_generic_interface *interface = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	int stat = 0;

	switch (mode) {
	case PL_EPDC_TEMP_MANUAL:
		// Force Set of Temperature to 37 Degree Celcius, cause acutal Waveform in the Firmware only supports 37 Degree
		IT8951WriteCmdCode(interface, type, USDEF_I80_CMD_FORCE_SET_TEMP);

		TWord dataTemp[2];
		dataTemp[0] = 0x0001;
		dataTemp[1] = p->manual_temp;

		//usleep(250);

		IT8951WriteDataBurst(interface, type, dataTemp, 2);
		IT8951WaitForReady(interface, type);
		stat = 1;
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		LOG(
				"Selected External Temp Mode, if no external Sensor read is available internal Temp will be used.");
		break;
	case PL_EPDC_TEMP_INTERNAL:
		LOG("Selected Internal Temp Mode");
		stat = 1;
		break;
	default:
		assert_fail("Invalid temperature mode");
	}

	p->temp_mode = mode;

	return stat;
}

static int update_temp(struct pl_generic_controller *controller) {
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	struct pl_generic_interface *interface = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;
	int stat = -1;
	int newTemp = 37;

	//LOG("Set Temperature to %i ", decimalNumber + " to %i " + hexadecimalNumber);
	if (controller->temp_mode == PL_EPDC_TEMP_MANUAL) {
		newTemp = controller->manual_temp;
	} else if (controller->temp_mode == PL_EPDC_TEMP_EXTERNAL) {

		IT8951WaitForReady(interface, type);

		IT8951WriteCmdCode(interface, type, IT8951_TCON_BYPASS_I2C);
		IT8951WriteData(interface, type, 0x01); // I2C write command
		IT8951WriteData(interface, type, 0x68); // TPS65815 Chip Address0
		IT8951WriteData(interface, type, 0x0D); // Power Up Sequence Register
		IT8951WriteData(interface, type, 0x01); // Write Size
		IT8951WriteData(interface, type, 0x80);

		IT8951WaitForReady(interface, type);

		TWord pmicTemp;
		IT8951WriteCmdCode(interface, type, IT8951_TCON_BYPASS_I2C);
		IT8951WriteData(interface, type, 0x00); // I2C write command
		IT8951WriteData(interface, type, 0x68); // TPS65815 Chip Address0
		IT8951WriteData(interface, type, 0x00); // Power Up Sequence Register
		IT8951WriteData(interface, type, 0x01); // Write Size
		pmicTemp = (int) IT8951ReadData(interface, type, 1);  //read data
		newTemp = pmicTemp >> 8;
		printf("PMIC Temp is %x \n", pmicTemp >> 8);
		//printf("Not yet implemented ! \n");
	}

	IT8951WaitForReady(interface, type);
	// Force Set of Temperature to 37 Degree Celcius, cause acutal Waveform in the Firmware only supports 37 Degree
	IT8951WriteCmdCode(interface, type, USDEF_I80_CMD_FORCE_SET_TEMP);

	IT8951WaitForReady(interface, type);

	TWord dataTemp[2];
	dataTemp[0] = 0x01;
	dataTemp[1] = newTemp;  //37   //controller->manual_temp;

	IT8951WriteDataBurst(interface, type, dataTemp, 2);
	IT8951WaitForReady(interface, type);

	stat = 1;

	return stat;
}

static int get_resolution(pl_generic_controller_t *p, int* xres, int* yres) {
	it8951_t *it8951 = p->hw_ref;
	I80IT8951DevInfo* pstDevInfo;
	assert(it8951 != NULL);
	IT8951WriteCmdCode(it8951->interface, it8951->sInterfaceType,
	USDEF_I80_CMD_GET_DEV_INFO);
	if (xres && yres) {
		// TODO: Check if scrambled!!!
		pstDevInfo = (I80IT8951DevInfo*) IT8951ReadData(it8951->interface,
				it8951->sInterfaceType, sizeof(I80IT8951DevInfo) / 2);

		*xres = pstDevInfo->usPanelW;
		*yres = pstDevInfo->usPanelH;
		return 0;
	}
	return -EINVAL;
}

static int fill(struct pl_generic_controller *controller,
		const struct pl_area *a, uint8_t grey) {

	clear = true;

	it8951_t *it8951 = controller->hw_ref;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	TDWord gulImgBufAddr;
	TByte* gpFrameBuf;
	rgbw_pixel_t *pngBuffer;

	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;

	//Host Init
	//------------------------------------------------------------------
	//Get Device Info

	if (devInfo.usImgBufAddrH == NULL)
		GetIT8951SystemInfo(bus, type, &devInfo);

	fillBuffer = malloc(devInfo.usPanelW * devInfo.usPanelH);
	//Write pixel 0xF0(White) to Frame Buffer
	memset(fillBuffer, grey, devInfo.usPanelW * devInfo.usPanelH);

	return 1;
}

