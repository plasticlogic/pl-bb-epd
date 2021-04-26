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
 * epdc-app.c
 *
 *  Created on: 10.09.2014
 *      Author: sebastian.friebe
 *
 *  Main file for the project.
 *  Handling command line calls and controls the different EPDC operations.
 *
 *  Following operations are available via command line parameters:
 *  (most of them will require additional parameters to work)
 *
 * 		-start_epdc		: initializes the EPDC
 * 		-stop_epdc		: de-initializes the EPDC
 * 		-set_waveform   : send the waveform file to the EPDC
 * 		-set_vcom       : sets the target vcom
 * 		-update_image   : execute an image update
 *
 *
 */
#include <errno.h>
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <pl/generic_epdc.h>
#include <pl/generic_controller.h>
#include <pl/vcom.h>
#include <pl/i2c.h>
#include <pl/gpio.h>
#include <pl/hwinfo.h>
#include <pl/assert.h>
#include <pl/parser.h>
#include "pindef.h"
#include "hw_setup.h"
#define LOG_TAG "main"
#include <pl/utils.h>
#include "text.h"
#include <libgd/src/gd.h>
#include <libgd/src/gdfontl.h>

#define INTERNAL_USAGE
#define IC2_INTERFACE
#define VERSION_INFO		"v1.2*"

// ----------------------------------------------------------------------
// -  global variables
// ----------------------------------------------------------------------
static pl_generic_epdc_t *epdc;	//!< pointer to a generic globally used EPDC object
static hw_setup_t *hardware;

struct CmdLineOptions {
	char *operation;
	char *short_description;
	int (*execute_operation)(int argc, char **argv);
	void (*print_help_message)();

};

#define ERROR_ARGUMENT_COUNT_MISMATCH 	125

// ----------------------------------------------------------------------
// function declarations
// ----------------------------------------------------------------------
int initialize_environment();
int release_environment();
int readBinaryFile(const char *binaryPath, uint8_t **buffer);

int start_epdc(int load_nvm_content, int execute_clear, int gpio_only);
int stop_epdc();
int set_vcom(int vcom);
int set_waveform(char *waveform, float *temperature);
int set_temperature(float temperature);
int get_vcom(void);
int get_waveform(void);
int get_temperature(void);
int get_resolution(void);
int update_image(char *path, const char* wfID, enum pl_update_mode mode,
		int vcom_switch, int updateCount, int waitTime);
int update_image_regional(char *path, const char* wfID,
		enum pl_update_mode mode, int vcom_switch, int updateCount,
		int waitTime, struct pl_area* area, int top, int left);
int read_register(regSetting_t regSetting);
int write_register(regSetting_t regSetting, const uint32_t bitmask);
int send_cmd(regSetting_t regSetting);
//remove int pgm_nvm(const char *waveform);
int switch_hv(int state);
int switch_com(int state);
void debug_print_parameters(int argc, char **argv);
void print_application_help(int print_all);
int info();
int show_image(const char *dir, const char *file, int wfid);
int counter(const char* wf);
int fill(uint8_t gl, uint8_t wfid, int update_mode);
//remove int interface_data(	char* interface,int number_of_values,char values);
int slideshow(const char *path, const char* wf, int count, int anim);

int execute_help(int argc, char **argv);
int execute_start_epdc(int argc, char **argv);
int execute_stop_epdc(int argc, char **argv);
int execute_set_vcom(int argc, char **argv);
int execute_set_waveform(int argc, char **argv);
int execute_set_temperature(int argc, char **argv);
int execute_get_vcom(int argc, char **argv);
int execute_get_waveform(int argc, char **argv);
int execute_get_temperature(int argc, char **argv);
int execute_get_resolution(int argc, char **argv);
int execute_update_image(int argc, char **argv);
int execute_update_image_regional(int argc, char **argv);
int execute_update_gfx(int argc, char **argv);
int execute_slideshow(int argc, char **argv);
int execute_counter(int argc, char **argv);
int execute_write_reg(int argc, char **argv);
int execute_read_reg(int argc, char **argv);
int execute_pgm_epdc(int argc, char **argv);
int execute_info(int argc, char **argv);
int execute_switch_hv(int argc, char **argv);
int execute_switch_com(int argc, char **argv);
int execute_send_cmd(int argc, char **argv);
int execute_fill(int argc, char **argv);
int execute_detect_i2c(int argc, char **argv);
int execute_write_i2c(int argc, char **argv);
int execute_read_i2c(int argc, char **argv);
int print_versionInfo(int argc, char **argv);

void printHelp_start_epdc(int identLevel);
void printHelp_stop_epdc(int identLevel);
void printHelp_set_vcom(int identLevel);
void printHelp_set_waveform(int identLevel);
void printHelp_set_temperature(int identLevel);
void printHelp_get_vcom(int identLevel);
void printHelp_get_resolution(int identLevel);
void printHelp_get_waveform(int identLevel);
void printHelp_get_temperature(int identLevel);
void printHelp_update_image(int identLevel);
void printHelp_update_image_regional(int identLevel);
void printHelp_update_gfx(int identLevel);
void printHelp_write_reg(int identLevel);
void printHelp_read_reg(int identLevel);
void printHelp_pgm_epdc(int identLevel);
void printHelp_info(int identLevel);
void printHelp_slideshow(int identLevel);
void printHelp_counter(int identLevel);
void printHelp_switch_hv(int identLevel);
void printHelp_switch_com(int identLevel);
void printHelp_send_cmd(int identLevel);
void printHelp_fill(int identLevel);
void printHelp_detect_i2c(int identLevel);
void printHelp_write_i2c(int identLevel);
void printHelp_read_i2c(int identLevel);

struct CmdLineOptions supportedOperations[] = {
		{ "-start_epdc", "initializes the EPD controller", execute_start_epdc, printHelp_start_epdc },
		{ "-stop_epdc", "de-initializes the EPD controller", execute_stop_epdc, printHelp_stop_epdc },
		{ "-set_vcom", "sets com voltage", execute_set_vcom, printHelp_set_vcom },
		{ "-set_waveform", "sets the waveform", execute_set_waveform, printHelp_set_waveform },
		{ "-set_temperature", "sets the temperature", execute_set_temperature, printHelp_set_temperature },
		{ "-get_vcom", "gets com voltage", execute_get_vcom, printHelp_get_vcom },
		{ "-get_resolution", "gets display resolution", execute_get_resolution, printHelp_get_resolution },
		{ "-get_waveform", "gets the waveform", execute_get_waveform, printHelp_get_waveform },
		{ "-get_temperature", "gets the temperature", execute_get_temperature, printHelp_get_temperature },
		{ "-update_image", "updates the display", execute_update_image, printHelp_update_image },
		{ "-update_image_regional", "updates the display on certain area", execute_update_image_regional, printHelp_update_image_regional },
		{ "-update_gfx", "updates the display with auto gen. gfx image", execute_update_gfx, printHelp_update_gfx },
		{ "-slideshow", "shows a slidshow of .png images", execute_slideshow, printHelp_slideshow },
		{ "-fill", "fill the screen with a defined greylevel", execute_fill, printHelp_fill },
		{ "-count", "shows a counting number", execute_counter, printHelp_counter },
#ifdef IC2_INTERFACE
		{ "-detect_i2c", "searches for all devices connected to ic2", execute_detect_i2c, printHelp_detect_i2c },
		{ "-write_i2c", "send data over i2c", execute_write_i2c, printHelp_write_i2c },
		{ "-read_i2c", "receives data over i2c", execute_read_i2c, printHelp_read_i2c },
#endif
#ifdef INTERNAL_USAGE
		{ "-send_cmd", "sends a command of EPD controller", execute_send_cmd, printHelp_send_cmd },
		{ "-write_reg", "writes to a register of EPD controller", execute_write_reg, printHelp_write_reg },
		{ "-read_reg", "reads from a register of EPD controller", execute_read_reg, printHelp_read_reg },
		{ "-pgm_epdc", "programs firmware to the EPD controller", execute_pgm_epdc, printHelp_pgm_epdc },
		{ "-info", "displays general display informations", execute_info, printHelp_info },
#endif
		{ "-switch_hv", "switches hv on/off based on parameter", execute_switch_hv, printHelp_switch_hv },
		{ "-switch_com", "switches com on/off based on parameter", execute_switch_com, printHelp_switch_com },
		{ "--version", "displays version info", print_versionInfo, NULL },
		{ "--help", "prints this help message", execute_help, NULL }, };

/**
 * Main sequence
 * Checks the command line parameter and executes selected operation.
 */
int main(int argc, char* argv[]) {
	// debug features
	int stat = 0;
	char message[200];

	if (initialize_environment()) {
		abort_msg("Initialization failed", ABORT_APPLICATION);
		return -1;
	}

	epdc->controller->animationMode = 0;

	// parse input parameter
	if (argc > 1) {
		int operationIdx = 0;
		struct CmdLineOptions *matchedOperation = NULL;
		for (operationIdx = 0;
				operationIdx
						< (sizeof(supportedOperations)
								/ sizeof(struct CmdLineOptions));
				operationIdx++) {
			if (!strcmp(argv[1], supportedOperations[operationIdx].operation)) {
				matchedOperation = &supportedOperations[operationIdx];
				break;
			}
		}

		// found operation, so execute it
		if (matchedOperation != NULL) {

			// want to print a help message for this operation?
			if ((argc > 2) && !strcmp(argv[2], "--help")) {
				if (matchedOperation->print_help_message != NULL) {
					matchedOperation->print_help_message(0);
				}
			}

			// just execute the operation
			else {
				stat = matchedOperation->execute_operation(argc, argv);
				if (stat < 0) {

					if ((stat == ERROR_ARGUMENT_COUNT_MISMATCH)
							&& (matchedOperation->print_help_message != NULL)) {
						sprintf(message, "wrong number of arguments...");
						abort_msg(message, ABORT_APPLICATION);

						printf("Please check the help message below:\n\n");
						matchedOperation->print_help_message(0);
					} else {
						sprintf(message, "operation '%s' failed: Error: %i",
								matchedOperation->operation, stat);
						abort_msg(message, ABORT_APPLICATION);
					}
				}
			}
		}

		else {
			abort_msg("Specified operation not supported\n", ABORT_APPLICATION);
			print_application_help(false);
			return 1;
		}
	} else {
		abort_msg("Wrong number of parameter. Try: --help\n",
				ABORT_APPLICATION);
		print_application_help(false);
		return 1;
	}
	release_environment();

	return stat;
}

int initialize_environment() {
	int stat = 0;

	hardware = hw_setup_new();

#ifdef INTERNAL_USAGE
	stat |= hardware->init_from_configfile(hardware, NULL);	///"/boot/uboot/epdc/S115_T1.1/epdc.config");
#else
			stat |= hardware->init_from_configfile(hardware, "epdc-app.config");
#endif

	epdc = generic_epdc_new();
	epdc->hv = hv_new();
	epdc->hv->hvConfig = hardware->hvConfig;
	epdc->hv->hvDriver = hardware->hvDriver;
	epdc->hv->hvTiming = hardware->hvTiming;
	epdc->hv->vcomConfig = hardware->vcomConfig;
	epdc->hv->vcomDriver = hardware->vcomDriver;
	epdc->hv->vcomSwitch = hardware->vcomSwitch;
	epdc->nvm = hardware->nvm;
	epdc->controller = hardware->controller;
	epdc->default_vcom = hardware->default_vcom;

	return stat;
}

/**
 * Releases all allocated ressources as SPI and controller as well as hv hardware parts.
 * @return always 0;
 */
int release_environment() {
	// release spi device
	hardware->sInterface->close(hardware->sInterface);
	hardware->sInterface->delete(hardware->sInterface);

	epdc->delete(epdc);
	return 0;
}

// ----------------------------------------------------------------------
// operation functions
// ----------------------------------------------------------------------
int execute_help(int argc, char **argv) {
	int print_all = false;

	if (argc >= 3) {
		if (strcmp(argv[2], "-all") == 0) {
			print_all = true;
		}
	}

	print_application_help(print_all);
	return 0;
}

int execute_start_epdc(int argc, char **argv) {
	int stat = 0;
	int executeClear = false;
	int initFromEEprom = false;
	int gpio_only = false;

	if (argc >= 5) {
		gpio_only = atoi(argv[4]);
	}

	if (argc >= 4) {
		executeClear = atoi(argv[3]);
	}

	if (argc >= 3) {
		initFromEEprom = atoi(argv[2]);
	}

	stat = start_epdc(initFromEEprom, executeClear, gpio_only);

	return stat;
}

int execute_stop_epdc(int argc, char **argv) {
	int stat = 0;

	stat = stop_epdc();

	return stat;
}

int execute_set_vcom(int argc, char **argv) {
	int stat;

	if (argc == 3) {
		stat = set_vcom(atoi(argv[2]));
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_set_waveform(int argc, char **argv) {
	int stat;
	float temperature;

	if (argc == 3) {
		stat = set_waveform(argv[2], NULL);
	} else if (argc == 4) {
		temperature = atof(argv[3]);
		stat = set_waveform(argv[2], &temperature);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_set_temperature(int argc, char **argv) {
	int stat;
	float temperature;

	if (argc == 3) {
		temperature = atof(argv[2]);
		stat = set_temperature(temperature);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_get_vcom(int argc, char **argv) {
	int stat;
	stat = get_vcom();
	return stat;
}

int execute_get_waveform(int argc, char **argv) {
	int stat;
	stat = get_waveform();

	return stat;
}

int execute_get_temperature(int argc, char **argv) {
	int stat;
	stat = get_temperature();

	return stat;
}

int execute_get_resolution(int argc, char **argv) {
	int stat;
	stat = get_resolution();

	return stat;
}

int execute_update_image(int argc, char **argv) {
	int stat;

	char* wfID = "default";
	int mode = 0;
	int vcom_switch_enable = 1;
	int updateCount = 1;
	int waitTime = 0;

	if (argc >= 8)
		vcom_switch_enable = atol(argv[7]);

	if (argc >= 7)
		waitTime = atoi(argv[6]);

	if (argc >= 6)
		updateCount = atoi(argv[5]);

	if (argc >= 5)
		mode = atoi(argv[4]);

	if (argc >= 4)
		wfID = argv[3];
	if (argc >= 3) {
		stat = update_image(argv[2], wfID, (enum pl_update_mode) mode,
				vcom_switch_enable, updateCount, waitTime);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_update_image_regional(int argc, char **argv) {
	int stat;

	char* wfID = "default";
	int mode = 0;
	int vcom_switch_enable = 1;
	int updateCount = 1;
	int waitTime = 0;

	int top = 0;
	int left = 0;
	struct pl_area* area = malloc(sizeof(struct pl_area));

	if (argc >= 10)
		vcom_switch_enable = atol(argv[9]);

	if (argc >= 9)
		waitTime = atoi(argv[8]);

	if (argc >= 8)
		updateCount = atoi(argv[7]);

	if (argc >= 7)
		mode = atoi(argv[6]);

	if (argc >= 6)
		wfID = argv[5];

	if (argc >= 5) {
		parser_read_int(argv[4] + parser_read_int(argv[4], ",", &left), ",",
				&top);
	}

	if (argc >= 4) {
		parser_read_area(argv[3], ",", area);
		stat = update_image_regional(argv[2], wfID, (enum pl_update_mode) mode,
				vcom_switch_enable, updateCount, waitTime, area, top, left);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}
	return stat;
}

int execute_update_gfx(int argc, char **argv) {

	int x, y;

	int stat = epdc->controller->get_resolution(epdc->controller, &x, &y);
	if (stat < 0)
		return stat;

	char* wfID = "default";
	char* path = "/tmp/gfx.png";
	char *s = "Hello this Image Indicates your working Display ;)";

	FILE * f = fopen(path, "w");

	gdImagePtr gfx = gdImageCreateTrueColor(x, y);
	//gdImageFilledRectangle(gfx, 0, 0, 99, 99, 0xFFFFFF);

	gdFontPtr fontptr = gdFontGetLarge();
	gdImageString(gfx, fontptr, gfx->sx / 2 - (strlen(s) * fontptr->w / 2),
			gfx->sy / 2 - fontptr->h / 2, (unsigned char*) s, 0xFFFFFF);

	gdImagePng(gfx, f);

	fclose(f);

	update_image(path, wfID, 0, 0, 1, 0);

	return 0;
}

int execute_counter(int argc, char**argv) {
	int stat;
	printf("%s\n", __func__);
	stat = counter(NULL);

	return stat;
}

int execute_slideshow(int argc, char**argv) {
	int stat;
	//slideshow path wfid waittime
	int waitTime = 0;
	int anim = 0;
	char* wfID = NULL;
	if (argc >= 6)
		anim = atoi(argv[5]);
	if (argc >= 5)
		waitTime = atoi(argv[4]);
	if (argc >= 4)
		wfID = argv[3];

	if (argc >= 3) {
		stat = slideshow(argv[2], wfID, waitTime, anim);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}
	return stat;
}

int execute_send_cmd(int argc, char **argv) {
	int stat;
	regSetting_t regData;
	regData.valCount = 1;
	uint16_t *data;
	static const char sep[] = ",";
	unsigned int val, len;

	if (argc >= 5) {
		regData.addr = (int) strtol(argv[2], NULL, 0);
		regData.valCount = (int) strtol(argv[3], NULL, 0);

		data = malloc(regData.valCount * sizeof(uint16_t));

		printf("value string: %s\n", argv[4]);
		int currentPosition = 0;
		int idx = 0;

		for (idx = 0; idx < regData.valCount; idx++) {
			len = parser_read_word(argv[4] + currentPosition, sep, &val);
			currentPosition += len;
			data[idx] = val;
			if (len <= 0)
				break;
		}

		regData.val = data;

		printf("found data values: ");

		for (idx = 0; idx < regData.valCount; idx++) {
			printf("0x%x,", ((uint16_t *) regData.val)[idx]);
		}
		printf("\n");
	} else if (argc >= 3) {
		regData.addr = (int) strtol(argv[2], NULL, 0);
		regData.valCount = 0;
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	stat = send_cmd(regData);
	if (regData.val)
		free(regData.val);
	return stat;
}

int execute_fill(int argc, char **argv) {
	uint8_t gl = 0xFF;
	int update_mode = PL_FULL_UPDATE;
	int wfid = 2;
	if (argc > 2) {
		if (!strncmp(argv[2], "GL", 2)) {
			int _gl;
			sscanf(argv[2], "GL%i", &_gl);
			gl = (uint8_t) (_gl * 16);

		} else {
			gl = atoi(argv[2]);
		}

		if (argc > 3) {
			wfid = atoi(argv[3]);

			if (argc > 4) {
				if (!strcmp(argv[4], "partial")) {
					update_mode = PL_PART_UPDATE;
				} else if (!strcmp(argv[4], "full")) {
					update_mode = PL_FULL_UPDATE;
				} else {
					return -22;
				}
			}
		}
	}
	return fill(gl, wfid, update_mode);
}

int execute_detect_i2c(int argc, char **argv) {

	int stat = 0;

	if (argc == 2)
	{
		struct pl_i2c* i2c = &(hardware->host_i2c);

		stat = i2c->detect(i2c);
	}
	else
	{
		LOG("Wrong number of parameter.");
		return -1;
	}

	return stat;
}

int execute_write_i2c(int argc, char **argv) {

	uint8_t addr;
	uint8_t reg;
	uint8_t data;

	int stat = 0;

	if (argc == 5)
	{
		addr = (uint8_t) strtol(argv[2], NULL, 0);
		reg = (uint8_t) strtol(argv[3], NULL, 0);
		data = (uint8_t) strtol(argv[4], NULL, 0);

		struct pl_i2c* i2c = &(hardware->host_i2c);

		stat = pl_i2c_reg_write_8(i2c, addr, reg, data);
	}
	else
	{
		LOG("Wrong number of parameter.");
		return -1;
	}

	return stat;
}

int execute_read_i2c(int argc, char **argv) {

	uint8_t addr;
	uint8_t reg;
	uint8_t *data;
	int stat = 0;

	if (argc == 4)
	{
		addr = (int) strtol(argv[2], NULL, 0);
		reg = (int) strtol(argv[3], NULL, 0);

		struct pl_i2c* i2c = &(hardware->host_i2c);

		stat = pl_i2c_reg_read_8(i2c, addr, reg, data);
	}
	else
	{
		LOG("Wrong number of parameter.");
		return -1;
	}

	return stat;
}

int execute_write_reg(int argc, char **argv) {
	int stat;
	regSetting_t regData;
	regData.valCount = 1;
	uint16_t *data;
	static const char sep[] = ",";
	unsigned int val, len;
	uint32_t bitmask = 0xFFFFFFFF;

	if (argc >= 6) {
		bitmask = (int) strtol(argv[5], NULL, 0);
	}

	if (argc >= 5) {
		regData.addr = (int) strtol(argv[2], NULL, 0);
		regData.valCount = (int) strtol(argv[3], NULL, 0);

		data = malloc(regData.valCount * sizeof(uint16_t));

		printf("value string: %s\n", argv[4]);
		int currentPosition = 0;
		int idx = 0;

		for (idx = 0; idx < regData.valCount; idx++) {
			len = parser_read_word(argv[4] + currentPosition, sep, &val);
			currentPosition += len;
			data[idx] = val;
			if (len <= 0)
				break;
		}

		regData.val = data;

		printf("found data values: ");

		for (idx = 0; idx < regData.valCount; idx++) {
			printf("0x%x,", ((uint16_t *) regData.val)[idx]);
		}
		printf("\n");
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	stat = write_register(regData, bitmask);
	free(data);

	return stat;
}

int execute_read_reg(int argc, char **argv) {
	int stat;
	regSetting_t regData;
	regData.valCount = 1;

	if (argc >= 4)
		regData.valCount = (int) strtol(argv[3], NULL, 0);

	if (argc >= 3) {
		regData.addr = (int) strtol(argv[2], NULL, 0);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	stat = read_register(regData);
	return stat;
}

int execute_pgm_epdc(int argc, char **argv) {
	int stat = 0;

	uint8_t* data;

	pl_nvm_t* nvm = epdc->nvm;

	unsigned int addr = 0x00;

	int isFirmware = atoi(argv[3]);

	int len = readBinaryFile(argv[2], &data);

	struct pl_spi *spi = (struct pl_spi *) nvm->hw_ref;

	if (isFirmware == 1) {
		spi->cs_gpio = FALCON_FIRMWARE_NVM_CS;
		printf("Using Firmware CS: %i \n", spi->cs_gpio);
	} else {
		spi->cs_gpio = FALCON_DISPLAY_NVM_CS;
		printf("Using Display CS: %i \n", spi->cs_gpio);
		stat = switch_hv(1);
	}

	nvm->hw_ref = spi;

	stat = nvm->pgm(nvm, addr, data, len);

	spi->cs_gpio = FALCON_DISPLAY_NVM_CS;

	nvm->hw_ref = spi;

	stat = switch_hv(0);

	return stat;
}

int execute_info(int argc, char **argv) {
	int stat = 0;

	stat = info();

	return stat;
}

int execute_switch_hv(int argc, char **argv) {
	int stat;
	int state;

	if (argc >= 3) {
		state = (int) strtol(argv[2], NULL, 0);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	if (state < 0 || state > 1) {
		LOG("Given HV state (%d) not supported.", state);
		return -EINVAL;
	}
	stat = switch_hv(state);
	return stat;
}

int execute_switch_com(int argc, char **argv) {
	int stat;
	int state;

	if (argc >= 3) {
		state = (int) strtol(argv[2], NULL, 0);
	} else {
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	if (state < -1 || state > 1) {
		LOG("Given COM state (%d) not supported.", state);
		return -EINVAL;
	}
	stat = switch_hv(state);
	return stat;
}

int print_versionInfo(int argc, char **argv) {
	printf("epdc-app version = %s\n", VERSION_INFO);

	return 0;
}
// ----------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------

/**
 * Initializes the EPDC.
 *
 * @return status
 */
int start_epdc(int load_nvm_content, int execute_clear, int gpio_only) {
	int stat = 0;
	LOG("load_nvm_content?: %d", load_nvm_content);

	//initialize GPIOs
	stat = pl_gpio_config_list(&(hardware->gpios), hardware->board_gpios,
			hardware->gpio_count);
	if (stat < 0) {
		LOG("GPIO init failed");
		return stat;
	}

	//enable VDD
	hardware->gpios.set(hardware->vddGPIO, 1);

	sleep(2);

	if (gpio_only == 0) {
		stat = epdc->init(epdc, load_nvm_content);
		if (stat < 0) {
			LOG("EPDC-Init failed: %i\n", stat);
			return stat;
		}
		if (execute_clear) {
			stat = epdc->clear_init(epdc);
		}

		if (execute_clear != 1)
			stat = switch_hv(0);
	} else {
		printf("GPIO only set ! \n");
	}

	return stat;
}
;

/**
 * De-Initializes the EPDC
 *
 * @return status
 */
int stop_epdc() {
	int stat = 0;
	stat = switch_hv(0);
	hardware->gpios.set(hardware->vddGPIO, 0);

	// de-configure Epson GPIOs
	stat = pl_gpio_deconfigure_list(&(hardware->gpios), hardware->board_gpios,
			hardware->gpio_count);
	if (stat < 0) {
		LOG("GPIO deconfigure failed");
		return stat;
	}

	return 0;
}

/**
 * Sets target vcom
 * @param vcom the target vcom voltage
 * @return status
 */
int set_vcom(int vcom) {

	printf("vcom %d\n", vcom);
	return epdc->set_vcom(epdc, vcom);
}

/**
 * Sends given waveform data to EPDC.
 * @param waveform path to a waveform file.
 * @param manual set temperature.
 * @return status
 */
int set_waveform(char *waveform, float *temperature) {
	int do_update = 0;
	int stat = 0;

	if (temperature != NULL
			&& (epdc->controller->temp_mode == PL_EPDC_TEMP_MANUAL)) {
		printf("waveform %s, temperature %f\n", waveform, *temperature);
		epdc->controller->manual_temp = (int) *temperature;
		do_update = 1;
	}

	if (do_update || (epdc->controller->temp_mode != PL_EPDC_TEMP_MANUAL))
		epdc->controller->update_temp(epdc->controller);

	stat = epdc->controller->load_wflib(epdc->controller, waveform);
	if (stat < 0)
		return stat;

	return 0;
}

/**
 * Sends given temperature to EPDC if manual temperature mode is active.
 * @param temperature.
 * @return status
 */
int set_temperature(float temperature) {

	printf("temperature %f\n", temperature);
	epdc->controller->set_temp_mode(epdc->controller, PL_EPDC_TEMP_MANUAL);
	epdc->controller->manual_temp = (int) temperature;
	epdc->controller->update_temp(epdc->controller);
	printf(
			"Temperature Mode will be set automatically to Manual, till reset of the ITE \n");
	return 0;
}

int get_vcom(void) {
	printf("VCOM: %i\n", epdc->get_vcom(epdc));
	return 0;
}

/**
 * Sends given waveform data to EPDC.
 * @param waveform path to a waveform file.
 * @param manual set temperature.
 * @return status
 */
int get_waveform(void) {

	int isPgm = 0;
	int stat = epdc->nvm->read_header(epdc->nvm, &isPgm);

	if (stat < 0)
		return stat;

	printf("Waveform Version: %s\n", epdc->nvm->wfVers);

	return 0;
}

/**
 * Sends given temperature to EPDC if manual temperature mode is active.
 * @return status
 */
int get_temperature(void) {
//	if (epdc->controller->temp_mode == PL_EPDC_TEMP_MANUAL) {
//		printf("temperature %i\n", epdc->controller->manual_temp);
//	} else {
//		printf(
//				"Manual get temperature not possible. Temperature mode is not set to \"MANUAL\". \n\r");
//		return -EINVAL;
//	}
	int stat = 0;
	int temperature[2];
	stat = epdc->controller->get_temp(epdc->controller, temperature);
	printf("Real Temp: %i , Set Temp: %i \n", temperature[0], temperature[1]);
	return 0;
}

int get_resolution(void) {
	int x, y;
	int stat = epdc->controller->get_resolution(epdc->controller, &x, &y);
	if (stat < 0)
		return stat;
	LOG("Physical Resolution: %ix%i", x, y);
	return 0;
}

/**
 * Updates image.
 * @param path image path.
 * @param wfID refers to the waveform id.
 * @param mode refers to the update mode, i.e. 0=full update, 1=partial update.
 * @param vcomSwitchEnable enables vcom switch control via epdc, 1=enable, 0=bypass.
 * @param updateCount refers to the count of image updates to execute
 * @param waitTime [ms] refers to the time to wait after one image update
 * @return status
 */
int update_image(char *path, const char* wfID, enum pl_update_mode mode,
		int vcomSwitchEnable, int updateCount, int waitTime) {
	int cnt = 0;
	int stat;

	struct timeval tStop, tStart; // time variables
	float tTotal;

	LOG("path: %s", path);

	int wfId = pl_generic_controller_get_wfid(epdc->controller, wfID);
	LOG("wfID: %d", wfId);
	LOG("updateMode: %d", mode);
	LOG("updateCount: %d", updateCount);
	LOG("waitTime: %d", waitTime);
	LOG("vcomSwitch: %d", vcomSwitchEnable);

	if (wfId < 0)
		return -EINVAL;

	gettimeofday(&tStart, NULL);
	stat = epdc->controller->load_image(epdc->controller, path, NULL, 0, 0);

	if (stat < 0)
		return stat;

	for (cnt = 0; cnt < updateCount; cnt++) {
		stat = epdc->update(epdc, wfId, mode, NULL);
		if (stat < 0)
			return stat;

		usleep(waitTime * 1000);
	}

	gettimeofday(&tStop, NULL);
	tTotal = (float) (tStop.tv_sec - tStart.tv_sec)
			+ ((float) (tStop.tv_usec - tStart.tv_usec) / 1000000);
	printf("Time Complete: %f\n", tTotal);

	return 0;
}
/**
 * Updates image.
 * @param path image path.
 * @param wfID refers to the waveform id.
 * @param mode refers to the update mode, i.e. 0=full update, 1=partial update.
 * @param vcomSwitchEnable enables vcom switch control via epdc, 1=enable, 0=bypass.
 * @param updateCount refers to the count of image updates to execute
 * @param waitTime [ms] refers to the time to wait after one image update
 * @return status
 */
int update_image_regional(char *path, const char* wfID,
		enum pl_update_mode mode, int vcomSwitchEnable, int updateCount,
		int waitTime, struct pl_area *area, int top, int left) {
	int cnt = 0;
	int stat;
	LOG("path: %s", path);

	int wfId = pl_generic_controller_get_wfid(epdc->controller, wfID);
	LOG("wfID: %d", wfId);
	LOG("updateMode: %d", mode);
	LOG("updateCount: %d", updateCount);
	LOG("waitTime: %d", waitTime);
	LOG("vcomSwitch: %d", vcomSwitchEnable);
	LOG("top: %d", top);
	LOG("left: %d", left);
	LOG("area: l: %i, t: %i, h: %i, w: %i", area->left, area->top, area->height,
			area->width);

	if (wfId < 0)
		return -EINVAL;

	stat = epdc->controller->load_image(epdc->controller, path, area, left,
			top);
	if (stat < 0)
		return stat;

	if (epdc->hv->vcomSwitch != NULL) {
		if (vcomSwitchEnable == 0) {
			epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 1);
		} else {
			epdc->hv->vcomSwitch->disable_bypass_mode(epdc->hv->vcomSwitch);
		}
	}

	for (cnt = 0; cnt < updateCount; cnt++) {
		stat = epdc->update(epdc, wfId, mode, area);
		if (stat < 0)
			return stat;

		usleep(waitTime * 1000);
	}

	if (epdc->hv->vcomSwitch != NULL) {
		if (vcomSwitchEnable == 0) {
			epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 0);
		}
	}

	return 0;
}

/**
 * print epdc register values to the command line
 * @param regSetting regSettings structure
 * @return status
 */
int read_register(regSetting_t regSetting) {
	uint16_t *data = malloc(sizeof(uint16_t) * regSetting.valCount);
	regSetting.val = data;
	if (regSetting.val == NULL)
		return -EINVAL;

	int stat = epdc->read_register(epdc, &regSetting);
	if (stat < 0)
		return stat;

	printf("register addr    = 0x%04X:\n", regSetting.addr);
	printf("register data    = \n");
	dump_hex16(regSetting.val, regSetting.valCount);

	free(regSetting.val);
	return 0;
}

int fill(uint8_t gl, uint8_t wfid, int update_mode) {

	int x, y, stat;

	stat = epdc->controller->get_resolution(epdc->controller, &x, &y);

	struct pl_area a = { 0, 0, x, y };

	if (stat < 0)
		return stat;
	LOG("FILL: %i, area: %i, %i, %i, %i", gl, a.width, a.height, a.top, a.left);

	stat = epdc->controller->fill(epdc->controller, &a, gl);
	if (stat < 0)
		return stat;
	stat = epdc->controller->load_image(epdc->controller, NULL, NULL, 0, 0);
	stat = epdc->update(epdc, wfid, update_mode, &a);
	return stat;
}

/**
 * writes epdc register values
 * @param regSetting regSettings structure
 * @return status
 */
int write_register(regSetting_t regSetting, const uint32_t bitmask) {
	if (regSetting.val == NULL)
		return -EINVAL;

	int stat = epdc->write_register(epdc, regSetting, bitmask);
	if (stat < 0)
		return stat;

	return 0;
}

/**
 * executes epdc command
 * @param regSetting regSettings structure
 * @return status
 */
int send_cmd(regSetting_t regSetting) {

	if (regSetting.val == NULL)
		return -EINVAL;

	int stat = epdc->send_cmd(epdc, regSetting);
	if (stat < 0)
		return stat;

	return 0;
}

/**
 * displays the general display informations.
 */
int info() {

	int isPgm = 0;
	int stat = epdc->nvm->read_header(epdc->nvm, &isPgm);
	if (stat < 0)
		return stat;

	printf("NVM is programmed: %d\n", isPgm);
	printf("Display ID: %s\n", epdc->nvm->dispId);
	printf("Vcom: %d\n", epdc->nvm->vcom);
	printf("Product ID: %s\n", epdc->nvm->prodId);
	printf("Waveform Version: %s\n", epdc->nvm->wfVers);
	printf("FPL Version: %s\n", epdc->nvm->fplVers);
	printf("NVM Version: %s\n", epdc->nvm->nvmVers);
	printf("Feature 1: %s\n", epdc->nvm->feature1);
	printf("Feature 2: %s\n", epdc->nvm->feature2);
	printf("Feature 3: %s\n", epdc->nvm->feature3);
	printf("Feature 4: %s\n", epdc->nvm->feature4);

	return 0;
}

/**
 * switches the high voltages on/off.
 * @param state 0=off, 1=on.
 * @return status
 */
int switch_hv(int state) {
	int stat;

	if (state == 1) {
		if ((epdc->hv->hvDriver != NULL)
				&& (epdc->hv->hvDriver->switch_on != NULL))
			stat = epdc->hv->hvDriver->switch_on(epdc->hv->hvDriver);
		if ((epdc->hv->vcomDriver != NULL)
				&& (epdc->hv->vcomDriver->switch_on != NULL))
			stat = epdc->hv->vcomDriver->switch_on(epdc->hv->vcomDriver);
	} else {
		if ((epdc->hv->vcomDriver != NULL)
				&& (epdc->hv->vcomDriver->switch_off != NULL))
			stat = epdc->hv->vcomDriver->switch_off(epdc->hv->vcomDriver);
		if ((epdc->hv->hvDriver != NULL)
				&& (epdc->hv->hvDriver->switch_off != NULL))
			stat = epdc->hv->hvDriver->switch_off(epdc->hv->hvDriver);
	}

	return stat;
}

/**
 * switches the com voltage on/off.
 * @param state 0=off, 1=on.
 * @return status
 */
int switch_com(int state) {
	int stat = 0;

	if (state == 1) {
		if (epdc->hv->vcomSwitch != NULL) {
			epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 1);
		}
	} else if (state == 0) {
		if (epdc->hv->vcomSwitch != NULL) {
			epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 0);
		}
	} else {
		if (epdc->hv->vcomSwitch != NULL) {
			epdc->hv->vcomSwitch->disable_bypass_mode(epdc->hv->vcomSwitch);
		}
	}

	return stat;
}

/**
 * opens and reads existing binary file into memory and returns the data pointer
 * @param binaryPath path to the binary file.
 * @param blob memory pointer to the data.
 * @return status
 */
int readBinaryFile(const char *binaryPath, uint8_t **blob) {
// read binary blob

	FILE *fs = fopen(binaryPath, "rb");
	int len = 0;

	if (fs == NULL) {
		LOG("Can't open specified binary file.");
		return -ENOENT;
	}

	fseek(fs, 0, SEEK_END);
	len = ftell(fs);
	fseek(fs, 0, SEEK_SET);

// read file content to blob
	*blob = (uint8_t *) malloc(sizeof(uint8_t) * len);
	int bytecount = fread(*blob, sizeof(uint8_t), len, fs);
	fclose(fs);

	if (bytecount != len) {
		LOG("Error during binary file reading.");
		return -EIO;
	}

	return bytecount;
}

// ----------------------------------------------------------------------
// Counter
// ----------------------------------------------------------------------
int counter(const char* wf) {
	return -ENOSYS;
	printf("%s\n", __func__);
	int wfid = 4;
	unsigned char count = 0;
	char counter[10] = { 0, };

	if (wf != NULL) {
		sscanf(wf, "%i", &wfid);
		LOG("Using Waveform %i", wfid);
	}
	struct pl_area area;
	area.height = 192;
	area.width = 1024;
	area.top = epdc->controller->yoffset;
	area.left = epdc->controller->xoffset;
//#if VERBOSE
	LOG("Running counter");
//#endif

	epdc->controller->fill(epdc->controller, &area, 0xFF);
	epdc->clear_init(epdc);

	if (epdc->hv->vcomSwitch != NULL) {
		epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 1);
	}
//struct timespec t;

//*
	while (1) {
		//start_stopwatch(&t);
		sprintf(counter, "%u", count++);

		//read_stopwatch(&t, "start loop", 1);
		if (show_text(epdc->controller, &area, counter, FONT0, 270, 50, 5, 1,
				1))
			return -1;
		//read_stopwatch(&t, "show text", 1);
		if (epdc->update(epdc, wfid, PL_FULL_UPDATE_NOWAIT, NULL))
			return -1;
		//read_stopwatch(&t, "update", 1);
	}
//*/
	return 0;
}

// ----------------------------------------------------------------------
// Slideshow
// ----------------------------------------------------------------------
int slideshow(const char *path, const char* wf, int waittime, int anim) {
	DIR *dir;
	struct dirent *d;
	int wfid = -1;
	if (wf != NULL) {
		sscanf(wf, "%i", &wfid);
		LOG("Using Waveform %i", wfid);
	}
	int stat;
	assert(path != NULL);
	int count = 10;
//#if VERBOSE
	LOG("Running slideshow");
//#endif
//*
	epdc->controller->animationMode = anim;

	if (anim == 1) {

		struct dirent **namelist;
		int n;
		int i = 0;

		n = scandir(path, &namelist, NULL, alphasort);

		if (n < 0)
			perror("scandir");
		else {
			while (count--) {
				while (i < n) {

					if (namelist[i]->d_name[0] == '.') {
						i++;
						continue;
					}

					printf("%s\n", namelist[i]->d_name);

					stat = show_image(path, namelist[i]->d_name, wfid);
//				free(namelist[i]);
					++i;
					if (stat < 0) {
						LOG("Failed to show image");
						return stat;
					}

				}
				i = 0;
			}
			free(namelist);
			switch_hv(0);
		}
	} else {
		while (count--) {
			if ((dir = opendir(path)) == NULL) {
				LOG("Failed to open directory [%s]", path);
				return -ENOENT;
			}
			while ((d = readdir(dir)) != NULL) {
				LOG("%s", d->d_name);
				if (d->d_name[0] == '.')
					continue;

				stat = show_image(path, d->d_name, wfid);

				if (stat < 0) {
					LOG("Failed to show image");
					return stat;

				}
				usleep(waittime);
			}
		}
	}
	closedir(dir);

	return 0;
}

int show_image(const char *dir, const char *file, int wfid) {
	char path[256];
	int stat = 0;

	struct timeval tStop, tStart; // time variables
	float tTotal;
	gettimeofday(&tStart, NULL);

	LOG("Image: %s %s", dir, file);
	if (wfid <= -1 || wfid > 14) {
		wfid = 2; // = pl_generic_controller_get_wfid(epdc->controller, wfID);
	}
	if (wfid < 0)
		return -EINVAL;
	if (dir != NULL) {
		LOG("Dir is not NULL: %s", dir);
		stat = join_path(path, sizeof(path), dir, file);
		if (stat < 0)
			return stat;
		LOG("Show: %s", path);
		stat = epdc->controller->load_image(epdc->controller, path, NULL, 0, 0);
		if (stat < 0)
			return stat;
	} else {
		LOG("Dir is NULL: %s", file);
		stat = epdc->controller->load_image(epdc->controller, file, NULL, 0, 0);
		if (stat < 0)
			return stat;
	}
	stat = epdc->update(epdc, wfid, 0, NULL);

	gettimeofday(&tStop, NULL);
	tTotal = (float) (tStop.tv_sec - tStart.tv_sec)
			+ ((float) (tStop.tv_usec - tStart.tv_usec) / 1000000);
	printf("Time: %f\n", tTotal);
	return stat;
}

// ----------------------------------------------------------------------
// help messages
// ----------------------------------------------------------------------
void print_application_help(int print_all) {
	printf("\n");
	printf(
			"Usage:  epdc-app [operation] [parameter]   --> executes an operation \n");
	printf(
			"        epdc-app [operation] --help        --> prints detailed help  \n");
	printf(
			"        epdc-app [operation] --help -all   --> prints complete help at once \n");
	printf("\n");
	printf("Available Operations:\n");
	int operationIdx = 0;
	for (operationIdx = 0;
			operationIdx
					< (sizeof(supportedOperations)
							/ sizeof(struct CmdLineOptions)); operationIdx++) {
		printf("\t%20s: \t%s\n", supportedOperations[operationIdx].operation,
				supportedOperations[operationIdx].short_description);
	}

	if (print_all == true) {
		printf("\n");
		printf("Detailed Operation Description:\n");

		for (operationIdx = 0;
				operationIdx
						< (sizeof(supportedOperations)
								/ sizeof(struct CmdLineOptions));
				operationIdx++) {
			if (supportedOperations[operationIdx].print_help_message != NULL) {
				printf("\n");
				printf("%20s: %*s %s\n",
						supportedOperations[operationIdx].operation, 3, " ",
						supportedOperations[operationIdx].short_description);
				supportedOperations[operationIdx].print_help_message(25);
			}
		}
	}
	printf("\n");
}

void printHelp_start_epdc(int identLevel) {
	printf("%*s Activates and initializes the EPD controller.\n", identLevel,
			" ");
	printf("\n");
	printf("%*s Usage: epdc-app -start_epdc [<nvm_flag> [ <clear_flag> ]]]\n",
			identLevel, " ");
	printf("\n");
	printf(
			"%*s \t<nvm_flag>    	   : \tif 0 = override of waveform and vcom enabled (default).\n",
			identLevel, " ");
	printf(
			"%*s \t                       \tif 1 = override disabled, use settings from NV memory.\n",
			identLevel, " ");
	printf(
			"%*s \t<clear_flag>         : \tif 0 = display will not be cleared. (default)\n",
			identLevel, " ");
	printf(
			"%*s \t                       \tif 1 = display will be cleared with default clear operation.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_stop_epdc(int identLevel) {
	printf("%*s De-initializes the EPD controller.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -stop_epdc\n", identLevel, " ");
	printf("\n");
}

void printHelp_set_vcom(int identLevel) {
	printf("%*s Sets the Vcom voltage.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -set_vcom <voltage>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<voltage>  : \tcom voltage in millivolts.\n", identLevel,
			" ");
	printf("\n");
}

void printHelp_set_waveform(int identLevel) {
	printf("%*s Sets the waveform used for later update operations.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -set_waveform <waveform> <temp>\n", identLevel,
			" ");
	printf("\n");
	printf("%*s \t<waveform> : \tpath to the waveform file.\n", identLevel,
			" ");
	printf("%*s \t<temp>     : \tTemperature in degree celsius.\n", identLevel,
			" ");
	printf("\n");
}

void printHelp_set_temperature(int identLevel) {
	printf("%*s Sets the temperature.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -set_temperature <temp>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<temp>     : \tTemperature in degree celsius.\n", identLevel,
			" ");
	printf("\n");
}

void printHelp_get_resolution(int identLevel) {
	printf("%*s Sets the Vcom voltage.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -get_resolution <voltage>\n", identLevel, " ");
	printf("\n");
}

void printHelp_get_vcom(int identLevel) {
	printf("%*s Gets the Vcom voltage.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -get_vcom\n", identLevel, " ");
	printf("\n");
}

void printHelp_get_waveform(int identLevel) {
	printf("%*s Gets the waveform used for later update operations.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -get_waveform\n", identLevel, " ");
	printf("\n");
}

void printHelp_get_temperature(int identLevel) {
	printf("%*s Gets the temperature.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -get_temperature \n", identLevel, " ");
	printf("\n");
}

void printHelp_update_image(int identLevel) {
	printf("%*s Updates the display with a given image.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -update_image_regional <image>\n", identLevel,
			" ");
	printf("\n");
	printf("%*s \t<image>               : \tpath to the image file.\n",
			identLevel, " ");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n",
			identLevel, " ");
	printf("%*s \t<updateMode>          : \tid of the used update mode.\n",
			identLevel, " ");
	printf(
			"%*s \t<updateCount>         : \tcount of image updates to execute.\n",
			identLevel, " ");
	printf(
			"%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n",
			identLevel, " ");
	printf(
			"%*s \t<vcomSwitchEnable>    : \tautomatic vcom switch enable: 0=disable/1=enable.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_update_image_regional(int identLevel) {
	printf("%*s Updates the display with a given image.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -update_image <image> <area> <position>\n",
			identLevel, " ");
	printf("\n");
	printf("%*s \t<image>               : \tpath to the image file.\n",
			identLevel, " ");
	printf(
			"%*s \t<area>                : \tarea to be used (top,left,height,width).\n",
			identLevel, " ");
	printf(
			"%*s \t<position>            : \tposition, where the area is printed to (top,left).\n",
			identLevel, " ");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n",
			identLevel, " ");
	printf("%*s \t<updateMode>          : \tid of the used update mode.\n",
			identLevel, " ");
	printf(
			"%*s \t<updateCount>         : \tcount of image updates to execute.\n",
			identLevel, " ");
	printf(
			"%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n",
			identLevel, " ");
	printf(
			"%*s \t<vcomSwitchEnable>    : \tautomatic vcom switch enable: 0=disable/1=enable.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_update_gfx(int identLevel) {
	printf("%*s Updates the display with a given image.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -update_image <image> <area> <position>\n",
			identLevel, " ");
	printf("\n");
	printf("%*s \t<image>               : \tpath to the image file.\n",
			identLevel, " ");
	printf(
			"%*s \t<area>                : \tarea to be used (top,left,height,width).\n",
			identLevel, " ");
	printf(
			"%*s \t<position>            : \tposition, where the area is printed to (top,left).\n",
			identLevel, " ");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n",
			identLevel, " ");
	printf("%*s \t<updateMode>          : \tid of the used update mode.\n",
			identLevel, " ");
	printf(
			"%*s \t<updateCount>         : \tcount of image updates to execute.\n",
			identLevel, " ");
	printf(
			"%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n",
			identLevel, " ");
	printf(
			"%*s \t<vcomSwitchEnable>    : \tautomatic vcom switch enable: 0=disable/1=enable.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_write_reg(int identLevel) {
	printf("%*s Writes to a specified register of the EPD controller.\n",
			identLevel, " ");
	printf("\n");
	printf(
			"%*s Usage: epdc-app -write_reg <reg_addr> <datacount> <data> [bitmask]\n",
			identLevel, " ");
	printf("\n");
	printf(
			"%*s \t<reg_addr> : \tspecifies the address of the register where the data is written.\n",
			identLevel, " ");
	printf(
			"%*s \t<datacount>: \tspecifies the amount of data portions to be written.\n",
			identLevel, " ");
	printf(
			"%*s \t<data>     : \tThe data to be written. Data portions must be separated by commata (',').\n",
			identLevel, " ");
	printf(
			"%*s \t             \tData should be given as one string without any spaces.\n",
			identLevel, " ");
	printf(
			"%*s \t<bitmask>  : \toptional parameter, which can mask out bits from write operation.\n",
			identLevel, " ");
	printf("\n");

}

void printHelp_fill(int identLevel) {
// TODO...
	printf("%*s Fill the screen with a Greylevel.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -fill <GLx | y>\n", identLevel, " ");
	printf("\n");
	printf(
			"%*s \t<GLy | y> : \toptional paraetmer, specifies the Greylevel to be used; GL0 to GL15 or integer from 0 to 255.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_send_cmd(int identLevel) {
	printf(
			"%*s Sends a command with specified arguments to the EPD controller.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -send_cmd <cmd> <datacount> <data> [bitmask]\n",
			identLevel, " ");
	printf("\n");
	printf("%*s \t<cmd> : \tspecifies the cmd to be executed.\n", identLevel,
			" ");
	printf(
			"%*s \t<datacount>: \toptional paraetmer, specifies the amount of data portions to be written.\n",
			identLevel, " ");
	printf(
			"%*s \t<data>     : \toptional parameter, arguments to be send. Data portions must be separated by commata (',').\n",
			identLevel, " ");
	printf("\n");

}

void printHelp_read_reg(int identLevel) {

	printf(
			"%*s Reads from a specified register of the EPD controller and prints the values to the shell.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -read_reg <reg_addr> <datacount>\n", identLevel,
			" ");
	printf("\n");
	printf(
			"%*s \t<reg_addr> : \tspecifies the address of the register to be read.\n",
			identLevel, " ");
	printf(
			"%*s \t<datacount>: \tspecifies the amount of data portions to be read.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_detect_i2c(int identLevel) {

	printf(
			"%*s Detects all devices on the epdc I2C bus.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -detect_i2c\n", identLevel,
			" ");
	printf("\n");
}

void printHelp_write_i2c(int identLevel) {

	printf(
			"%*s Writes data to the epdc I2C bus.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -write_i2c\n", identLevel,
			" ");
	printf("\n");
}

void printHelp_read_i2c(int identLevel) {

	printf(
			"%*s Reads data frome the epdc I2C bus.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -read_i2c\n", identLevel,
			" ");
	printf("\n");
}

void printHelp_pgm_epdc(int identLevel) {
	printf(
			"%*s Programs the specified binary to the boot nvm or directly to the epdc.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -pgm_epdc <binary_path>\n", identLevel, " ");
	printf("\n");
}
/*//remove
 void printHelp_pgm_nvm_binary(int identLevel){
 printf("%*s Programs the specified binary to the specific NVM.\n", identLevel, " ");
 printf("\n");
 printf("%*s Usage: epdc-app -pgm_nvm <displayId> <Vcom> <binary_path>\n", identLevel, " ");
 printf("\n");
 printf("%*s \t<display_id>  : specifies display id to be programmed into the nvm.\n", identLevel, " ");
 printf("%*s \t<Vcom>        : specifies the Vcom to be programmed into the nvm.\n", identLevel, " ");
 printf("%*s \t<binary_path> : specifies the path to the binary file to be programmed into the nvm.\n", identLevel, " ");
 printf("%*s \t<product_id>  : optional.\n", identLevel, " ");
 printf("%*s \t<waveform_vers>  : optional.\n", identLevel, " ");
 printf("%*s \t<fpl_vers>    : optional.\n", identLevel, " ");
 printf("%*s \t<nvm_vers>    : optional.\n", identLevel, " ");
 printf("%*s \t<feature1>    : optional.\n", identLevel, " ");
 printf("%*s \t<feature2>    : optional.\n", identLevel, " ");
 printf("%*s \t<feature3>    : optional.\n", identLevel, " ");
 printf("%*s \t<feature4>    : optional.\n", identLevel, " ");
 printf("\n");
 }
 */
void printHelp_counter(int identLevel) {
	printf("%*s Shows a counting number.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -count\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n",
			identLevel, " ");
	printf(
			"%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_slideshow(int identLevel) {
	printf("%*s Updates the display with a slideshow of a given path.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -slideshow <image path>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<image path>          : \tpath to the image files.\n",
			identLevel, " ");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n",
			identLevel, " ");
	printf(
			"%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_interface_data(int identLevel) {
	printf("%*s Updates the display with a data of a given interface.\n",
			identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -interface <dev path>\n", identLevel, " ");
	printf("\n");
	printf(
			"%*s \t<dev path>            : \tpath to the interface. Default is /dev/can1.\n",
			identLevel, " ");
	printf(
			"%*s \t<number of values>    : \tvalues count to be displayed at a time.\n",
			identLevel, " ");
	printf(
			"%*s \t<values>              : \tflags which values should be displayed.\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_info(int identLevel) {
	printf("%*s Displays general display informations.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -info\n", identLevel, " ");
	printf("\n");
}

void printHelp_switch_com(int identLevel) {
	printf("%*s Switches COMswitch on/off based on parameter.\n", identLevel,
			" ");
	printf("\n");
	printf("%*s Usage: epdc-app -switch_com <state>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<state>: 0 = off; 1= on; -1= disable manual settings\n",
			identLevel, " ");
	printf("\n");
}

void printHelp_switch_hv(int identLevel) {
	printf("%*s Switches HV on/off based on parameter.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: epdc-app -switch_hv <state>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<state>: 0 = off; 1= on.\n", identLevel, " ");
	printf("\n");
}

void debug_print_parameters(int argc, char **argv) {
	int paramIdx = 0;
	for (paramIdx = 0; paramIdx < argc; paramIdx++) {
		printf("param %d: '%s'\n", paramIdx, argv[paramIdx]);
	}
}
