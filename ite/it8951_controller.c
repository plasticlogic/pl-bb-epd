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
#include <pl/utils.h>

#include <ite/it8951_controller.h>

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
uint16_t reg7[4];
uint16_t reg8[4];
int verbose = 0;

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
static int load_buffer(struct pl_generic_controller *controller, uint8_t *buf,
		struct pl_area *area, int binary);
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
static int get_temperature(pl_generic_controller_t *p, int* temperature);

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
	controller->get_temp = get_temperature;
	controller->get_resolution = get_resolution;
	controller->load_buffer = load_buffer;

	return 0;
}

static int trigger_update(struct pl_generic_controller *controller) {

	//initialize communication structure
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	if (modeToUse == 0) {
		controller->imageWidth = controller->xres;
		controller->imageHeight = controller->yres;
		partialX = 0;
		partialY = 0;
	}

	if (*type == I80) {
		TWord buf[8];
		buf[0] = (TWord) USDEF_I80_CMD_DPY_AREA_BUFFER;
		buf[1] = (TWord) partialX;
		buf[2] = (TWord) partialY;
		buf[3] = (TWord) controller->imageWidth;
		buf[4] = (TWord) controller->imageHeight;
		buf[5] = (TWord) wfidToUse;
		buf[6] = (TWord) devInfo.usImgBufAddrL;
		buf[7] = (TWord) devInfo.usImgBufAddrH;
		//struct timespec t;
		//start_stopwatch(&t);
		IT8951WriteDataBurst(bus, type, buf, 16);
		//read_stopwatch(&t, "Trigger Update", 1);
	} else {
		IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_DPY_AREA_BUFFER);
		IT8951WriteData(bus, type, (TWord) partialX);     		// Display X
		IT8951WriteData(bus, type, (TWord) partialY); 			// Display Y
		IT8951WriteData(bus, type, (TWord) controller->imageWidth); // Display W devInfo.usPanelW 1200
		IT8951WriteData(bus, type, (TWord) controller->imageHeight); // Display H devInfo.usPanelH 960
		IT8951WriteData(bus, type, (TWord) wfidToUse); 		// Display Mode
		IT8951WriteData(bus, type, (TWord) devInfo.usImgBufAddrL); // Display H devInfo.usPanelH 960
		IT8951WriteData(bus, type, (TWord) devInfo.usImgBufAddrH);
	}

	if (controller->animationMode == 1) {
		if (controller->bufferNumber == 0) {
			controller->bufferNumber = 1;
		} else if (controller->bufferNumber == 1) {
			controller->bufferNumber = 0;
		}
	}

//Check if Frame Buffer configuration Mode, when only 1BPP (Bit per Pixel), configure for Black and white update

//	regSetting_t regUpdate;
//	regUpdate.addr = (int) USDEF_I80_CMD_DPY_AREA;
//	regUpdate.valCount = (int) 5;
//	TWord data[5];
//	data[0] = (TWord) partialX;
//	data[1] = (TWord) partialY;
//	data[2] = (TWord) controller->imageWidth;
//	data[3] = (TWord) controller->imageHeight;
//	data[4] = (TWord) wfidToUse;
//	regUpdate.val = data;
//	send_cmd(controller, regUpdate);
//	IT8951WriteDataBurst(bus, type, data, 5);
//

	if (!controller->animationMode) {

		IT8951WaitForReady(bus, type);

		printf("PMIC Register 7 after update: ");

		regSetting_t reg;
		reg.addr = (int) IT8951_TCON_BYPASS_I2C;
		reg.valCount = (int) 4;
		reg7[0] = 0x00;
		reg7[1] = 0x68;
		reg7[2] = 0x07;
		reg7[3] = 0x01;
		reg.val = reg7;

		if (*type == I80) {
			TWord buf[5];
			buf[0] = IT8951_TCON_BYPASS_I2C;
			buf[1] = 0x00;
			buf[2] = 0x68;
			buf[3] = 0x07;
			buf[4] = 0x01;
			IT8951WaitForReady(bus, type);
			IT8951WriteDataBurst(bus, type, buf, 10);
			IT8951WaitForReady(bus, type);
			TWord* value = IT8951ReadData(bus, type, 2);  //read data
			printf("0x%x\n", *value);

		} else {
			send_cmd(controller, reg);
			TWord* value = IT8951ReadData(bus, type, 2);  //read data
			printf("0x%x\n", *value);
		}

		printf("PMIC Register 8 after update: ");

		regSetting_t reg2;
		reg2.addr = IT8951_TCON_BYPASS_I2C;
		reg2.valCount = 4;
		reg8[0] = 0x00;
		reg8[1] = 0x68;
		reg8[2] = 0x08;
		reg8[3] = 0x01;
		reg2.val = reg8;

		if (*type == I80) {
			TWord buf[5];
			buf[0] = IT8951_TCON_BYPASS_I2C;
			buf[1] = 0x00;
			buf[2] = 0x68;
			buf[3] = 0x08;
			buf[4] = 0x01;
			IT8951WaitForReady(bus, type);
			IT8951WriteDataBurst(bus, type, buf, 10);
			IT8951WaitForReady(bus, type);
			TWord* value = IT8951ReadData(bus, type, 2);  //read data
			printf("0x%x\n", *value);
		} else {
			send_cmd(controller, reg2);
			TWord* value = IT8951ReadData(bus, type, 2);  //read data
			printf("0x%x\n", *value);
		}
		IT8951WaitForDisplayReady(bus, type);
	}
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

	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo);

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

	//I80IT8951DevInfo devInfo;
	GetIT8951SystemInfo(bus, type, &devInfo);

	if (*type == I80) {
		TWord buf[5];
		buf[0] = USDEF_I80_CMD_SET_PWR_SEQ;
		buf[1] = 0x54;
		buf[2] = 0x00;
		buf[3] = 0xC9;
		buf[4] = 0x00;
		IT8951WriteDataBurst(bus, type, buf, 10);

	} else {
		TWord buf[4];
		buf[0] = 0x0054;
		buf[1] = 0x0000;
		buf[2] = 0x00C9;
		buf[3] = 0x0000;
		IT8951SendCmdArg(bus, type, USDEF_I80_CMD_SET_PWR_SEQ, buf, 4);
	}

	return 0;
}

static int configure_update(struct pl_generic_controller *controller, int wfid,
		enum pl_update_mode mode, const struct pl_area *area) {
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);

	wfidToUse = wfid;
	modeToUse = mode;

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

	if (target_sourcelines != 1156) {

		for (gateline = 0; gateline < source_gatelines; gateline++)
			for (sourceline = 0; sourceline < source_sourcelines;
					sourceline++) {
				int source_index = gateline * source_sourcelines + sourceline;
				int target_index = (gateline + _gateline_offset)
						* target_sourcelines
						+ (sourceline + _sourceline_offset);
				if (!(source_index < 0 || target_index < 0)) {
					target[target_index] = source[source_index];
					source[source_index] = 0x00;
				}
			}

//Comment in in case of 4x11.7"
	} else {

		for (gateline = 0; gateline < 412; gateline++) {
			for (sourceline = 0; sourceline < source_sourcelines;
					sourceline++) {
				int source_index = gateline * source_sourcelines + sourceline;
				target[source_index] = source[source_index];
			}
		}

		for (gateline = 412; gateline < 824; gateline++)
			for (sourceline = 0; sourceline < source_sourcelines;
					sourceline++) {
				int source_index = gateline * source_sourcelines + sourceline;
				int target_index = (gateline + _gateline_offset)
						* target_sourcelines
						+ (sourceline + _sourceline_offset);
				if (!(source_index < 0 || target_index < 0)) {
					target[target_index] = source[source_index];
					//source[source_index] = 0x00;
				}
			}
		for (gateline = 824; gateline < 1236; gateline++)
			for (sourceline = 0; sourceline < source_sourcelines;
					sourceline++) {
				int source_index = gateline * source_sourcelines + sourceline;
				int target_index = (gateline + _gateline_offset * 2)
						* target_sourcelines
						+ (sourceline + _sourceline_offset);
				if (!(source_index < 0 || target_index < 0)) {
					target[target_index] = source[source_index];
					//source[source_index] = 0x00;
				}
			}
		for (gateline = 1236; gateline < 1648; gateline++)
			for (sourceline = 0; sourceline < source_sourcelines;
					sourceline++) {
				int source_index = gateline * source_sourcelines + sourceline;
				int target_index = (gateline + _gateline_offset * 3)
						* target_sourcelines
						+ (sourceline + _sourceline_offset);
				if (!(source_index < 0 || target_index < 0)) {
					target[target_index] = source[source_index];
					//source[source_index] = 0x00;
				}
			}
		//saveBufToPNG(target_sourcelines, target_gatelines, target);
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

static int load_buffer(struct pl_generic_controller *controller, uint8_t *buf,
		struct pl_area *area, int binary) {

	it8951_t *it8951 = controller->hw_ref;

	int width, height = 0;

	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	TDWord gulImgBufAddr;
	TByte* gpFrameBuf;
	rgbw_pixel_t *pngBuffer;

	IT8951LdImgInfo stLdImgInfo;
	IT8951AreaImgInfo stAreaImgInfo;

	int fd, stat = 0;

	struct timeval tStop, tStart; // time variables
	float tTotal;

	gettimeofday(&tStart, NULL);

	if (devInfo.usImgBufAddrH == NULL)
		GetIT8951SystemInfo(bus, type, &devInfo);

	if (binary == 1) {
		devInfo.usImgBufAddrL = 0xBE28;
		devInfo.usImgBufAddrH = 0x24;
	}

//	if (controller->animationMode == 1) {
//		if (controller->bufferNumber == 0) {
//			devInfo.usImgBufAddrL = 0xFF50;
//			devInfo.usImgBufAddrH = 0x11;
//		} else if (controller->bufferNumber == 1) {
//			devInfo.usImgBufAddrL = 0x7F60;
//			devInfo.usImgBufAddrH = 0x37;
//		}
//	}

	//Get Image Buffer Address of IT8951
	gulImgBufAddr = devInfo.usImgBufAddrL | (devInfo.usImgBufAddrH << 16);

	//Set to Enable I80 Packed mode
	IT8951WriteReg(bus, type, I80CPCR, 0x0001);
	//-------------------------------------------------------------------

	if (area != NULL) {
		controller->yres = area->height;
		controller->xres = area->width;
	} else {
		controller->yres = devInfo.usPanelH;
		controller->xres = devInfo.usPanelW;
	}

	uint8_t *img_buf;
	rgbw_pixel_t *png_buffer;

	fd = open(buf, O_RDONLY);

	width = controller->xres;
	height = controller->yres;

	img_buf = malloc(sizeof(uint8_t) * width * height);
	stat = read(fd, img_buf, width * height);
	stat = close(fd);
	stLdImgInfo.ulStartFBAddr = (TDWord) img_buf;

	//--------------------------------------------------------------------------------------------
	//      initial display - Display white only
	//--------------------------------------------------------------------------------------------
	//Load Image and Display
	//Setting Load image information
	stLdImgInfo.usEndianType = IT8951_LDIMG_L_ENDIAN;
	stLdImgInfo.usPixelFormat = IT8951_8BPP;
	stLdImgInfo.usRotate = IT8951_ROTATE_0;
	stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
	//Set Load Area
	//ToDo: Pipe x/y position through from console
	if (area != NULL) {
		stAreaImgInfo.usX = area->left;
		stAreaImgInfo.usY = area->top;
		partialX = area->left;
		partialY = area->top;
	} else {
		stAreaImgInfo.usX = partialX;
		stAreaImgInfo.usY = partialY;
	}

	stAreaImgInfo.usWidth = controller->xres;
	stAreaImgInfo.usHeight = controller->yres;

	gettimeofday(&tStop, NULL);
	tTotal = (float) (tStop.tv_sec - tStart.tv_sec)
			+ ((float) (tStop.tv_usec - tStart.tv_usec) / 1000000);
	if (!controller->animationMode)
		printf("Time Load and Scramble: %f\n", tTotal);

	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo);

	//if (img_buf)
	//free(img_buf);
	//if (png_buffer)
	//free(png_buffer);

	return 0;

}

static int load_png_image(struct pl_generic_controller *controller,
		const char *path, struct pl_area *area, int left, int top) {

	it8951_t *it8951 = controller->hw_ref;

	if (left == 99999 && top == 99999) {
		partialX = 0;
		partialY = 0;
	} else {
		partialX = left;
		partialY = top;
	}

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

	struct timeval tStop, tStart; // time variables
	float tTotal;
	gettimeofday(&tStart, NULL);

	if (left == 99999 && top == 99999) {
		GetIT8951SystemInfo(bus, type, &devInfo);
		//devInfo.usImgBufAddrL = 0x2a70;
		//devInfo.usImgBufAddrH = 0x12;
		devInfo.usImgBufAddrL = 0xBE28;
		devInfo.usImgBufAddrH = 0x24;
	} else {
		if (devInfo.usImgBufAddrH == NULL)
			GetIT8951SystemInfo(bus, type, &devInfo);
	}

//	devInfo.usPanelH = 960;
//	devInfo.usPanelW = 1280;
//	devInfo.usImgBufAddrL = 0x2a70;
//	devInfo.usImgBufAddrH = 0x12;

	//Get Image Buffer Address of IT8951
	gulImgBufAddr = devInfo.usImgBufAddrL | (devInfo.usImgBufAddrH << 16);

	//Set to Enable I80 Packed mode
	IT8951WriteReg(bus, type, I80CPCR, 0x0001);
	//-------------------------------------------------------------------
	controller->yres = devInfo.usPanelH;
	controller->xres = devInfo.usPanelW;

	int v_yres = 0;
	int v_xres = 0;

	if (controller->display_scrambling == 0) {
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

//		FILE *fp;
//		fp = fopen("/tmp/img.txt", "w");
//
//		fwrite(gpFrameBuf, sizeof(gpFrameBuf[0]), 1280 * 960, fp);
//
//		fclose(fp);

		controller->imageWidth = width;
		controller->imageHeight = height;
		if (area != NULL && controller->display_scrambling == 0) {
			area->width = width;
			area->height = height;
		} else if (area != NULL && controller->display_scrambling != 0) {
			area->width = controller->xres;
			area->height = controller->yres;
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
		if (controller->display_scrambling) {
			scrambledPNG = malloc(
			max(height,
					controller->yres) * max(width, controller->xres));
			scramble_array(gpFrameBuf, scrambledPNG, &height, &width,
					controller->display_scrambling);

		} else {
			scrambledPNG = malloc(width * height);

			scramble_array(gpFrameBuf, scrambledPNG, &height, &width,
					controller->display_scrambling);
		}

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

		controller->imageWidth = width;
		controller->imageHeight = height;

		scramble_array(colorBuffer, scrambledPNG, &height, &width,
				controller->display_scrambling);
		free(colorBuffer);
		if (pngBuffer)
			free(pngBuffer);
	}

	TByte* targetBuf;
	if (controller->display_scrambling && clear) {
		targetBuf = malloc(controller->xres * controller->yres);
		memory_padding(scrambledPNG, targetBuf, controller->yres,
				controller->xres, controller->yres, controller->xres,
				controller->yoffset, controller->xoffset);
	} else if (controller->display_scrambling) {
		targetBuf = malloc(
		max(height,
				controller->yres) * max(width, controller->xres));

		if (area == NULL) {
			memory_padding(scrambledPNG, targetBuf, height, width,
					controller->yres, controller->xres, controller->yoffset,
					controller->xoffset);
		} else {
			area->width = width;
			area->height = height;
			memory_padding_area(scrambledPNG, targetBuf, height, width,
					controller->yoffset, controller->xoffset, area, partialY,
					partialX);

//			targetBuf = malloc(width * height);
//					memcpy(targetBuf, scrambledPNG, width * height);
		}

		//}

	} else {
		targetBuf = malloc(width * height);
		memcpy(targetBuf, scrambledPNG, width * height);
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

	if (controller->display_scrambling) {
		stAreaImgInfo.usWidth = controller->xres;
		stAreaImgInfo.usHeight = controller->yres;
	} else {
		stAreaImgInfo.usWidth = width;
		stAreaImgInfo.usHeight = height;
	}

	gettimeofday(&tStop, NULL);
	tTotal = (float) (tStop.tv_sec - tStart.tv_sec)
			+ ((float) (tStop.tv_usec - tStart.tv_usec) / 1000000);
	if (!controller->animationMode)
		printf("Time Load and Scramble: %f\n", tTotal);

	//multiple Buffer Calculation
//	For each address the formula is Image buffer address + number * panel height* panel width.
//	ex:
//	(#0)Image buffer address = 0x10000
//	#1  = 0x10000 + panel height* panel width
//	#2  = 0x10000 + 2*(panel height* panel width)

	//TDWord buf1 = 0x4A3E38;
	//TDWord buf2 = 0x377E30;
	//TDWord buf3 = 0x5CFE40;

	//int test = 0;
	//TWord tempBuf[1280 * 960];
	//while(test < (1280*960)){

	//printf("Image Buffer Address = %X\r\n", buf2);

	//stLdImgInfo.ulImgBufBaseAddr = buf2;

	//memset(tempBuf, 0x00, 1280 * 960);
	//stLdImgInfo.ulStartFBAddr = tempBuf;

	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo);

	//IT8951WaitForReady(bus, type);

//	//TestBufferUpdate
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_DPY_AREA_BUFFER);
//	IT8951WriteData(bus, type, (TWord) 0);     				// Display X
//	IT8951WriteData(bus, type, (TWord) 0); 					// Display Y
//	IT8951WriteData(bus, type, (TWord) controller->imageWidth); // Display W devInfo.usPanelW 1200
//	IT8951WriteData(bus, type, (TWord) controller->imageHeight); // Display H devInfo.usPanelH 960
//	IT8951WriteData(bus, type, (TWord) wfidToUse); 				// Display Mode
//	IT8951WriteData(bus, type, (TWord) gulImgBufAddr & 0xFFFF);
//	IT8951WriteData(bus, type, (TWord) (gulImgBufAddr >> 16) & 0xFFFF);

//	stLdImgInfo.ulImgBufBaseAddr = buf1;
//
//	memset(tempBuf, 0xFF, 1280 * 960);
//	stLdImgInfo.ulStartFBAddr = tempBuf;
//
//	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo);
//
//	IT8951WaitForReady(bus, type);
//
//	//TestBufferUpdate
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_DPY_AREA_BUFFER);
//	IT8951WriteData(bus, type, (TWord) 0);     				// Display X
//	IT8951WriteData(bus, type, (TWord) 0); 					// Display Y
//	IT8951WriteData(bus, type, (TWord) controller->imageWidth); // Display W devInfo.usPanelW 1200
//	IT8951WriteData(bus, type, (TWord) controller->imageHeight); // Display H devInfo.usPanelH 960
//	IT8951WriteData(bus, type, (TWord) 4); 				// Display Mode
//	IT8951WriteData(bus, type, (TWord) buf1 & 0xFFFF);
//	IT8951WriteData(bus, type, (TWord) (buf1 >> 16) & 0xFFFF);
//
//	stLdImgInfo.ulImgBufBaseAddr = buf3;
//	stLdImgInfo.ulStartFBAddr = targetBuf;
//
//	IT8951HostAreaPackedPixelWrite(bus, type, &stLdImgInfo, &stAreaImgInfo);
//
//	IT8951WaitForReady(bus, type);
//
//	//TestBufferUpdate
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_DPY_AREA_BUFFER);
//	IT8951WriteData(bus, type, (TWord) 0);     				// Display X
//	IT8951WriteData(bus, type, (TWord) 0); 					// Display Y
//	IT8951WriteData(bus, type, (TWord) controller->imageWidth); // Display W devInfo.usPanelW 1200
//	IT8951WriteData(bus, type, (TWord) controller->imageHeight); // Display H devInfo.usPanelH 960
//	IT8951WriteData(bus, type, (TWord) 4); 				// Display Mode
//	IT8951WriteData(bus, type, (TWord) buf3 & 0xFFFF);
//	IT8951WriteData(bus, type, (TWord) (buf3 >> 16) & 0xFFFF);

	if (scrambledPNG)
		free(scrambledPNG);

	if (controller->cfa_overlay.r_position == -1) {
		if (gpFrameBuf)
			free(gpFrameBuf);

		if (targetBuf)
			free(targetBuf);

//		if (tempBuf)
//			free(tempBuf);

	}

	return 0;
}

static int wait_update_end(struct pl_generic_controller *controller) {
//	//initialize communication structure
	it8951_t *it8951 = controller->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;
//
//	//Poll the TCON Register, to know when the update has finished
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

	if (*type == I80) {
		TWord *buf = malloc(sizeof(TWord) * setting.valCount + sizeof(TWord));
		buf[0] = setting.addr;
		for (i = 1; i <= setting.valCount; i++) {
			buf[i] = setting.val[i - 1];
		}
		IT8951WriteDataBurst(interface, type, buf, (setting.valCount + 1) * 2);
		free(buf);
	} else {
		IT8951WriteCmdCode(interface, type, setting.addr);

		for (i = 0; i < setting.valCount; i++)
			IT8951WriteData(interface, type, setting.val[i]);
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
		p->temp_mode = PL_EPDC_TEMP_MANUAL;

		if (*type == I80) {
			TWord buf[3];
			buf[0] = USDEF_I80_CMD_FORCE_SET_TEMP;
			buf[1] = 0x0001;
			buf[2] = p->manual_temp;
			IT8951WriteDataBurst(interface, type, buf, 6);

		} else {
			IT8951WriteCmdCode(interface, type, USDEF_I80_CMD_FORCE_SET_TEMP);
			TWord dataTemp[2];
			dataTemp[0] = 0x0001;
			dataTemp[1] = p->manual_temp;
			IT8951WaitForReady(interface, type);
			IT8951WriteData(interface, type, dataTemp[0]);
			IT8951WriteData(interface, type, dataTemp[1]);
			IT8951WaitForReady(interface, type);
		}

		stat = 0;
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		p->temp_mode = PL_EPDC_TEMP_EXTERNAL;
		LOG(
				"Selected External Temp Mode, if no external Sensor read is available internal Temp will be used.");
		break;
	case PL_EPDC_TEMP_INTERNAL:
		p->temp_mode = PL_EPDC_TEMP_INTERNAL;
		LOG("Selected Internal Temp Mode");
		stat = 0;
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
	int shouldUpdate = 0;

	if (controller->temp_mode == PL_EPDC_TEMP_MANUAL) {
		shouldUpdate = 1;
		newTemp = controller->manual_temp;
		printf("Manual set Temperature to %i \n", newTemp);
	} else if (controller->temp_mode == PL_EPDC_TEMP_EXTERNAL) {

//		IT8951WaitForReady(interface, type);
//
//		IT8951WriteCmdCode(interface, type, IT8951_TCON_BYPASS_I2C);
//		IT8951WriteData(interface, type, 0x01); // I2C write command
//		IT8951WriteData(interface, type, 0x68); // TPS65815 Chip Address0
//		IT8951WriteData(interface, type, 0x0D); // Power Up Sequence Register
//		IT8951WriteData(interface, type, 0x01); // Write Size
//		IT8951WriteData(interface, type, 0x80);
//
//		IT8951WaitForReady(interface, type);
//
//		TWord pmicTemp;
//		IT8951WriteCmdCode(interface, type, IT8951_TCON_BYPASS_I2C);
//		IT8951WriteData(interface, type, 0x00); // I2C write command
//		IT8951WriteData(interface, type, 0x68); // TPS65815 Chip Address0
//		IT8951WriteData(interface, type, 0x00); // Power Up Sequence Register
//		IT8951WriteData(interface, type, 0x01); // Write Size
//		pmicTemp = (int) IT8951ReadData(interface, type, 1);  //read data
//		newTemp = pmicTemp >> 8;
//		printf("PMIC Temp is %x \n", pmicTemp >> 8);
		printf("Not yet implemented, using internal mode ! \n");
	} else if (controller->temp_mode == PL_EPDC_TEMP_INTERNAL) {
		printf("Using internal measured Temp ! \n");
		shouldUpdate = 0;
		stat = 0;
	}

	if (shouldUpdate == 1) {

		if (*type == I80) {
			TWord buf[3];
			buf[0] = USDEF_I80_CMD_FORCE_SET_TEMP;
			buf[1] = 0x01;
			buf[2] = newTemp;
			IT8951WaitForReady(interface, type);
			IT8951WriteDataBurst(interface, type, buf, 6);
		} else {
			IT8951WaitForReady(interface, type);
			// Force Set of Temperature to 37 Degree Celcius, cause acutal Waveform in the Firmware only supports 37 Degree
			IT8951WriteCmdCode(interface, type,
			USDEF_I80_CMD_FORCE_SET_TEMP);

			IT8951WaitForReady(interface, type);

			TWord dataTemp[2];
			dataTemp[0] = 0x01;
			dataTemp[1] = newTemp;  //37   //controller->manual_temp;

			//IT8951WriteDataBurst(interface, type, dataTemp, 2);
			IT8951WriteData(interface, type, dataTemp[0]);
			IT8951WriteData(interface, type, dataTemp[1]);
			IT8951WaitForReady(interface, type);
		}

		stat = 0;
	}

	return stat;
}

static int get_resolution(pl_generic_controller_t *p, int* xres, int* yres) {
	it8951_t *it8951 = p->hw_ref;
	I80IT8951DevInfo* pstDevInfo;
	assert(it8951 != NULL);

	if (*it8951->sInterfaceType == I80) {
		IT8951WriteCmdCode(it8951->interface, it8951->sInterfaceType,
		USDEF_I80_CMD_GET_DEV_INFO);
		if (xres && yres) {
			// TODO: Check if scrambled!!!
			pstDevInfo = (I80IT8951DevInfo*) IT8951ReadData(it8951->interface,
					it8951->sInterfaceType, sizeof(I80IT8951DevInfo));

			*xres = pstDevInfo->usPanelW;
			*yres = pstDevInfo->usPanelH;
			return 0;
		}
	} else {
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
	}

//	IT8951WriteCmdCode(it8951->interface, it8951->sInterfaceType,
//	USDEF_I80_CMD_GET_DEV_INFO);
//	if (xres && yres) {
//		// TODO: Check if scrambled!!!
//		pstDevInfo = (I80IT8951DevInfo*) IT8951ReadData(it8951->interface,
//				it8951->sInterfaceType, sizeof(I80IT8951DevInfo) / 2);
//
//		*xres = pstDevInfo->usPanelW;
//		*yres = pstDevInfo->usPanelH;
//		return 0;

	return -EINVAL;
}

static int get_temperature(pl_generic_controller_t *p, int* temperature) {
	it8951_t *it8951 = p->hw_ref;
	pl_generic_interface_t *bus = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;
	TWord *dataTemp = malloc(sizeof(TWord) * 2);

	IT8951WaitForReady(bus, type);

	if (*type == I80) {
		TWord buf[2];
		buf[0] = USDEF_I80_CMD_FORCE_SET_TEMP;
		buf[1] = 0x00;
		IT8951WriteDataBurst(bus, type, buf, 4);
		dataTemp = IT8951ReadData(bus, type, 4);
	} else {
		IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_FORCE_SET_TEMP);
		IT8951WaitForReady(bus, type);

		IT8951WriteData(bus, type, 0x00);
		IT8951WaitForReady(bus, type);

		dataTemp = IT8951ReadData(bus, type, 2);
	}

	IT8951WaitForReady(bus, type);

	temperature[0] = dataTemp[0];
	temperature[1] = dataTemp[1];

	if (dataTemp)
		free(dataTemp);

	return 0;
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

	//devInfo.usPanelW = 2048;

	fillBuffer = malloc(devInfo.usPanelW * devInfo.usPanelH);
	//Write pixel 0xF0(White) to Frame Buffer
	memset(fillBuffer, grey, devInfo.usPanelW * devInfo.usPanelH);

	return 0;
}

