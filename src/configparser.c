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
 * configparser.c
 *
 *  Created on: 27.04.2015
 *      Author: sebastian.friebe
 */



#include <pl/parser.h>
#include <pl/assert.h>
#include "iniparser.h"
#include <src/pindef.h>

#include "beaglebone/beaglebone-gpio.h"
#include "beaglebone/beaglebone-i2c.h"
#include "ite/it8951_i2c.h"
//#include "beaglebone/beaglebone-spi.h"
#include <pl/generic_interface.h>
#include "epson/epson-i2c.h"

#include "configparser.h"
#include "hw_setup.h"

#define LOG_TAG "configparser"
#include "pl/utils.h"



static int getRegValCount(char *str);
static int getRegVal(char *str, int count, uint16_t *out);
static int loadRegisterSettings(hw_setup_t *setup, dictionary *dictConfig);
static int setTemperatureMode(pl_generic_controller_t *controller, dictionary *dictConfig);

int parse_config(hw_setup_t *setup, const char *filename){
	char *str;
	int i;
	dictionary *dictConfig;

	if(!filename){

		FILE * config = NULL ;
		char line[1025] ;
		char filename_str[64];
		char displaytype[64];
		//int len = 0;

		if ((config=fopen("/boot/uboot/config.txt", "r"))==NULL) {
			fprintf(stderr, "parser: cannot open /boot/uboot/config.txt\n");
			return -ENOENT;
		}
		memset(line,    0, 1024);

		while(parser_read_file_line(config, line, 1024)){
			//LOG("Line: %s" , line);
			if(strncmp(line, "display_type", 12)==0){
				strcpy(displaytype, &line[13]);
				// evaluate string
				LOG("DISPLAYTYPE: %s", displaytype);
				break;
			}
		}
		strcpy(filename_str, "/boot/uboot/");
		strcat(filename_str, displaytype);
		strcat(filename_str, "/epdc.config");
		dictConfig = iniparser_load(filename_str);
	}else{
		dictConfig = iniparser_load(filename);
	}

	LOG("version - %s\n", iniparser_getstring(dictConfig, "version:name", ""));

	if (dictConfig == NULL)
		return -EINVAL;
	int stat = 0;

	// initialize gpio strcuture
	beaglebone_gpio_init(&(setup->gpios));
	// ------------------------------------
	// initialize board related settings
	// ----------------------
	str = iniparser_getstring(dictConfig, "general:control_system", NULL);
	if (str == NULL) LOG("missing control system setting...");
	stat = setup->initialize_control_system(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "general:driver_board", NULL);
	if (str == NULL) LOG("missing driver board setting...");
	stat = setup->initialize_driver_board(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "general:nvm_spi_port", NULL);
	if (str == NULL) LOG("missing general:nvm_spi_port setting...");
	int nvm_spi_channel = atoi(str);

	str = iniparser_getstring(dictConfig, "general:epdc_spi_port", NULL);
	if (str == NULL) LOG("missing general:epdc_spi_port setting...");
	int epdc_spi_channel = atoi(str);

	// ------------------------------------
	// initialize spi devices
	// ----------------------

	setup->sInterface = interface_new(epdc_spi_channel, &(setup->gpios), setup->sInterfaceType);
	if (setup->sInterface == NULL){
		LOG("EPD Interface init has failed");
		return -ENODEV;
	}

 	if (setup->sInterface->open(setup->sInterface) != 1)
		return -EBUSY;

	// nvm spi device
// 	if (setup->sInterfaceType == I80 || setup->sInterfaceType == SPI_HRDY){
// 		//setup->nvmSPI = setup->sInterface;
//		setup->nvmSPI = beaglebone_spi_hrdy_new((uint8_t) nvm_spi_channel,
//				&(setup->gpios));
//		setup->nvmSPI->cs_gpio = FALCON_SPI_CS_ITE;
//		setup->nvmSPI->hrdy_gpio = FALCON_I80_HRDY;
//
// 	}else {
 		setup->nvmSPI = beaglebone_spi_new((uint8_t) nvm_spi_channel, &(setup->gpios));
	//}


	if (setup->nvmSPI == NULL){
		LOG("nvmSPI init has failed");
		return -ENODEV;
	}

	if (setup->nvmSPI->open(setup->nvmSPI) != 1)
		return -EBUSY;

	// ------------------------------------
	// initialize i2c
	// ----------------------
	str = iniparser_getstring(dictConfig, "general:i2c_master", NULL);
	if (str == NULL)
	{
		LOG("missing general:i2c_master setting... set default: i2c_master=BEAGLEBONE");
		str = "BEAGLEBONE";
	}

	if(!strcmp(str, "EPDC"))
	{
		stat = it8951_i2c_init(setup->controller, &(setup->host_i2c));
		if(stat < 0) return -ENODEV;
	}
	else
	{
		// str == BEAGLEBONE and others
		stat = beaglebone_i2c_init(setup->i2c_port, &(setup->host_i2c));
		if(stat < 0) return -ENODEV;
	}



	switch(setup->sInterfaceType){
	case SPI: 			LOG("Interface: SPI"); break;
	case SPI_HRDY: 		LOG("Interface: SPI with HRDY Pin"); break;
	case I80: 			LOG("Interface: I80"); break;
	case PARALLEL: 		LOG("Interface: Parallel");; break;
	}

	// ------------------------------------
	// initialize controller
	// ----------------------
	str = iniparser_getstring(dictConfig, "display:controller", NULL);
	if (str == NULL) LOG("missing controller setting...");
	stat = setup->initialize_controller(setup, str);
	if(stat < 0) return stat;

	setup->controller->regDefaults = NULL;
	setup->controller->regDefaultsCount = 0;
	stat = loadRegisterSettings(setup, dictConfig);
	if(stat < 0)
		return stat;

	stat = setTemperatureMode(setup->controller, dictConfig);
	if(stat < 0)
		return stat;

	str = iniparser_getstring(dictConfig, "display:default_waveform", NULL);
	if (str == NULL) LOG("missing display:default_waveform setting...");
	setup->controller->waveform_file_path = str;

	str = iniparser_getstring(dictConfig, "display:default_temp", NULL);
	if (str == NULL) LOG("missing display:default_temp setting... using default 23degC");
	setup->controller->manual_temp = (str == NULL) ? 23 : atoi(str);

	str = iniparser_getstring(dictConfig, "display:instruction_code_file", NULL);
	if (str == NULL) LOG("missing display:instruction_code_file setting...");
	setup->controller->instruction_code_path = str;

	str = iniparser_getstring(dictConfig, "display:default_vcom", NULL);
	if (str == NULL) LOG("missing display:default_vcom setting... using default 0V");
	setup->default_vcom = (str == NULL)  ? 0 : atoi(str);

	str = iniparser_getstring(dictConfig, "general:DISPLAY_SCRAMBLE_CONFIG", NULL);
	if (str == NULL) LOG("missing general:DISPLAY_SCRAMBLE_CONFIG setting... using default=0");
	setup->controller->display_scrambling = (str == NULL) ? 0 : atoi(str);

	str = iniparser_getstring(dictConfig, "general:DISPLAY_SCRAMBLE_XOFFSET", NULL);
	setup->controller->xoffset = (str == NULL) ? 0 : atoi(str);

	str = iniparser_getstring(dictConfig, "general:DISPLAY_SCRAMBLE_YOFFSET", NULL);
	setup->controller->yoffset = (str == NULL) ? 0 : atoi(str);

	str = iniparser_getstring(dictConfig, "general:CFA", NULL);
	if(str==NULL){
		setup->controller->cfa_overlay.r_position = -1;
		setup->controller->cfa_overlay.b_position = -1;
		setup->controller->cfa_overlay.g_position = -1;
		setup->controller->cfa_overlay.w_position = -1;
	}else{
		for(i=0; i<4; i++){
			switch(str[i]){
				case 'R':{
					setup->controller->cfa_overlay.r_position = i;
					break;
				}
				case 'G':{
					setup->controller->cfa_overlay.g_position = i;
					break;
				}
				case 'B':{
					setup->controller->cfa_overlay.b_position = i;
					break;
				}
				case 'W':{
					setup->controller->cfa_overlay.w_position = i;
					break;
				}
				default:{
					LOG("Invalid CFA overlay (Use upper case letters only)");
					setup->controller->cfa_overlay.r_position = -1;
					setup->controller->cfa_overlay.b_position = -1;
					setup->controller->cfa_overlay.g_position = -1;
					setup->controller->cfa_overlay.w_position = -1;
					i=4;
				}
			}
		}
		rgbw_pixel_t rgbw_pixel = {'R','G','B','W'};

		LOG("CFA overlay: %c%c%c%c",
				get_rgbw_pixel_value(0, setup->controller->cfa_overlay, rgbw_pixel),
				get_rgbw_pixel_value(1, setup->controller->cfa_overlay, rgbw_pixel),
				get_rgbw_pixel_value(2, setup->controller->cfa_overlay, rgbw_pixel),
				get_rgbw_pixel_value(3, setup->controller->cfa_overlay, rgbw_pixel));
	}

	// ------------------------------------
	// initialize HV parts
	// ----------------------
	str = iniparser_getstring(dictConfig, "hv_hardware:hv_config", NULL);
	if (str == NULL) LOG("missing hv_config setting...");
	stat = setup->initialize_hv_config(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "hv_hardware:hv_driver", NULL);
	if (str == NULL) LOG("missing hv_driver setting...");
	stat = setup->initialize_hv_driver(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "hv_hardware:vcom_switch", NULL);
	if (str == NULL) LOG("missing vcom_switch setting...");
	stat = setup->initialize_vcom_switch(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "hv_hardware:vcom_config", NULL);
	if (str == NULL) LOG("missing vcom_config setting...");
	stat = setup->initialize_vcom_config(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "hv_hardware:vcom_driver", NULL);
	if (str == NULL) LOG("missing vcom_driver setting...");
	stat = setup->initialize_vcom_driver(setup, str);
	if(stat < 0) return stat;

	str = iniparser_getstring(dictConfig, "hv_hardware:hv_timing", NULL);
	if (str == NULL) LOG("missing hv_timing setting...");
	stat = setup->initialize_hv_timing(setup, str);
	if(stat < 0) return stat;

	if (setup->hvTiming != NULL){
		setup->hvTiming->toffset_vgl_on  = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VGL_ON" , "0"));
		setup->hvTiming->toffset_vgl_off = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VGL_OFF", "0"));
		setup->hvTiming->toffset_vgh_on  = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VGH_ON" , "0"));
		setup->hvTiming->toffset_vgh_off = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VGH_OFF", "0"));
		setup->hvTiming->toffset_vsl_on  = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VSL_ON" , "0"));
		setup->hvTiming->toffset_vsl_off = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VSL_OFF", "0"));
		setup->hvTiming->toffset_vsh_on  = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VSH_ON" , "0"));
		setup->hvTiming->toffset_vsh_off = atoi(iniparser_getstring(dictConfig, "hv_hardware:TOFFSET_VSH_OFF", "0"));
	}

	// ------------------------------------
	// initialize nvm
	// ----------------------
	char *format_str = iniparser_getstring(dictConfig, "display:nvm_format", NULL);
	if (format_str == NULL) {
		LOG("missing display:nvm_format setting..., using default (PLAIN)");
		format_str =  "PLAIN";
	}

	str = iniparser_getstring(dictConfig, "display:nvm", NULL);
	if (str == NULL) LOG("missing display:nvm setting...");
	stat |= setup->initialize_nvm(setup, str, format_str);
	if(stat < 0) return stat;

	// ------------------------------------
	// initialize vcom
	// ----------------------
	str = iniparser_getstring(dictConfig, "vcom:dac_x1", NULL);
	if (str == NULL) LOG("missing vcom:dac_x1 setting...");
	int vcom_dac_x1 = atoi(str);

	str = iniparser_getstring(dictConfig, "vcom:dac_x2", NULL);
	if (str == NULL) LOG("missing vcom:dac_x2 setting...");
	int vcom_dac_x2 = atoi(str);

	str = iniparser_getstring(dictConfig, "vcom:dac_y1", NULL);
	if (str == NULL) LOG("missing vcom:dac_y1 setting...");
	int vcom_dac_y1 = atoi(str);

	str = iniparser_getstring(dictConfig, "vcom:dac_y2", NULL);
	if (str == NULL) LOG("missing vcom:dac_y2 setting...");
	int vcom_dac_y2 = atoi(str);

	str = iniparser_getstring(dictConfig, "vcom:vgpos_mv", NULL);
	if (str == NULL) LOG("missing vcom:vgpos_mv setting...");
	int vcom_vgpos_mv = atoi(str);

	str = iniparser_getstring(dictConfig, "vcom:vgneg_mv", NULL);
	if (str == NULL) LOG("missing vcom:vgneg_mv setting...");
	int vcom_vgneg_mv = atoi(str);

	str = iniparser_getstring(dictConfig, "vcom:swing_ideal", NULL);
	if (str == NULL) LOG("missing vcom:swing_ideal setting...");
	int vcom_swing_ideal = atoi(str);

	setup->g_vcom_info.dac_x1 = vcom_dac_x1;
	setup->g_vcom_info.dac_y1 = vcom_dac_y1;
	setup->g_vcom_info.dac_x2 = vcom_dac_x2;
	setup->g_vcom_info.dac_y2 = vcom_dac_y2;
	setup->g_vcom_info.swing_ideal = vcom_swing_ideal;
	setup->g_vcom_info.vgneg_mv = vcom_vgneg_mv;
	setup->g_vcom_info.vgpos_mv = vcom_vgpos_mv;

	vcom_init(&(setup->g_vcom_cal), &(setup->g_vcom_info));

	return stat;
}

// ----------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------
static int loadRegisterSettings(hw_setup_t *setup, dictionary *dictConfig){
	int i,j;
	int nsec = iniparser_getnsec(dictConfig);
	int posRegSettings = -1;
	int nRegSettings = 0;

	// Get number of register settings
	for(i=0; i<nsec; i++){
		if(!strcmp(iniparser_getsecname(dictConfig,i), "register_settings")){
			posRegSettings = i;
			nRegSettings = iniparser_getnkeys(dictConfig,i+1);
			break;
		}
	}

	// Set register settings
	regSetting_t* register_settings = NULL;
	char* strValues;
	char* strAddr;
	uint16_t* intValues;
	int iAddr, valCount;
	if(nRegSettings == 0){
		// just for information ...
		LOG("No register settings specified in the configuration file.");
	}
	else{
		register_settings = malloc(nRegSettings * sizeof(regSetting_t));
		for(i=0; i<nRegSettings; i++){
			// get address
			strAddr = iniparser_getkeyname(dictConfig, posRegSettings, i);
			sscanf(strAddr, "%x", &iAddr);
			// get values
			strValues = iniparser_getkeyvalue(dictConfig, posRegSettings, i);
			valCount = getRegValCount(strValues);
			intValues = malloc(sizeof(uint16_t)*valCount);
			getRegVal(strValues, valCount, intValues );
			// set address and values
			register_settings[i].addr = iAddr;
			register_settings[i].valCount = valCount;
			register_settings[i].val = malloc(sizeof(int)*valCount);
			for(j=0; j<register_settings[i].valCount; j++){
				register_settings[i].val[j] = intValues[j];
			}
		}
	}

	setup->controller->regDefaults = register_settings;
	setup->controller->regDefaultsCount = nRegSettings;

	return 0;
}

static int setTemperatureMode(pl_generic_controller_t *controller, dictionary *dictConfig){
	assert(controller != NULL);
	assert(dictConfig != NULL);

	char *str = iniparser_getstring(dictConfig, "display:temp_mode", NULL);
	if (str == NULL) {
		LOG("missing display:temp_mode setting...");
		return -EINVAL;
	}


	int selected_temp_mode = PL_EPDC_TEMP_MANUAL;
	if (!strcmp(str, "EXTERNAL"))
		selected_temp_mode = PL_EPDC_TEMP_EXTERNAL;
	else if (!strcmp(str, "MANUAL"))
		selected_temp_mode = PL_EPDC_TEMP_MANUAL;
	else if (!strcmp(str, "INTERNAL"))
		selected_temp_mode = PL_EPDC_TEMP_INTERNAL;
	else {
		LOG("setting in display:temp_mode not supported...");
		return -EINVAL;
	}

	controller->temp_mode  = selected_temp_mode;
	return 0;
}


static int getRegValCount(char *str){

	char* sep = ",";
	int len = 1;
	int currentPosition = 0, count = 0;
	unsigned int val;

	while(len > 0){
		len = parser_read_word(str + currentPosition, sep, &val);
		currentPosition+=len;
		if (len <= 0)
			break;
		count++;
	}

	return count;
}

static int getRegVal(char *str, int count, uint16_t *out){

	char* sep = ",";

	int len, currentPosition = 0;
	int i;
	unsigned int val;

	for(i=0; i<=count; i++){
		len = parser_read_word(str + currentPosition, sep, &val);
		if (len <= 0){
			return len;
		}
		currentPosition+=len;
		out[i] = val;
	}

	return 0;
}


