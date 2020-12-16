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
 * hw_setup.c
 *
 *  Created on: 27.04.2015
 *      Author: sebastian.friebe
 */

#include "stdlib.h"
#include "pl/assert.h"
#include "pindef.h"
#include "hw_setup.h"
#include "epson/s1d135xx_controller.h"
#include "epson/epson-s1d135xx.h"
#include "epson/s1d135xx_hv.h"
#include "epson/s1d13541-nvm.h"
#include "ite/it8951_controller.h"
#include "hardware/pmic-max17135.h"
#include "hardware/pmic-tps65185.h"
#include <hardware/nvm-i2c-24LC014H.h>
#include <hardware/nvm-i2c-24AA256.h>
#include <hardware/nvm-spi-MX25U4033E.h>
#include <beaglebone/beaglebone-hv.h>
#include <hardware/pmic-max17135-hv.h>
#include <hardware/pmic-tps65185-hv.h>
#include <hardware/dac-max5820-hv.h>
#include <beaglebone/beaglebone-spi.h>
#include "configparser.h"

#define LOG_TAG "hw-setup"
#include "pl/utils.h"


static pl_pmic_t *get_max17135_instance(hw_setup_t *p);
static pl_pmic_t *get_tps65185_instance(hw_setup_t *p);
static s1d135xx_t *get_s1d13524_instance(hw_setup_t *p);
static s1d135xx_t *get_s1d13541_instance(hw_setup_t *p);
static it8951_t *get_it8951_instance(hw_setup_t *p);
static pl_pmic_t *get_it8951_pmic_instance(hw_setup_t *p);

static int initialize_control_system(hw_setup_t *p, const char *selection);
static int initialize_driver_board(hw_setup_t *p, const char *selection);
static int initialize_nvm(hw_setup_t *p, const char *selection, const char *format_str);
static int initialize_hv_config(hw_setup_t *p, const char *selection);
static int initialize_vcom_driver(hw_setup_t *p, const char *selection);
static int initialize_vcom_config(hw_setup_t *p, const char *selection);
static int initialize_vcom_switch(hw_setup_t *p, const char *selection);
static int initialize_hv_timing(hw_setup_t *p, const char *selection);
static int initialize_hv_driver(hw_setup_t *p, const char *selection);
static int initialize_controller(hw_setup_t *p, const char *selection);

static it8951_t * it8951;						//!< pointer to an ITE IT8951 implementation
static pl_pmic_t * it8951_pmic;					//!< pointer to an ITE IT8951 implementation
static s1d135xx_t *s1d13541;					//!< pointer to an Epson S1D13541 implementation
static s1d135xx_t *s1d13524;					//!< pointer to an Epson S1D13524 implementation
static pl_pmic_t *max17135;						//!< pointer to MAX17135 PMIC object
static pl_pmic_t *tps65185;						//!< pointer to TPS65185 PMIC object


hw_setup_t *hw_setup_new(){
	hw_setup_t *p = (hw_setup_t *)malloc(sizeof(hw_setup_t));

	//p->sSPI = NULL;
	//p->sParallel = NULL;
	p->sInterface = NULL;
	p->nvmSPI = NULL;
	p->initialize_control_system = initialize_control_system;
	p->initialize_driver_board = initialize_driver_board;
	p->initialize_nvm = initialize_nvm;
	p->initialize_hv_config = initialize_hv_config;
	p->initialize_vcom_driver = initialize_vcom_driver;
	p->initialize_vcom_config = initialize_vcom_config;
	p->initialize_vcom_switch = initialize_vcom_switch;
	p->initialize_hv_timing = initialize_hv_timing;
	p->initialize_hv_driver = initialize_hv_driver;
	p->initialize_controller = initialize_controller;

	p->init_from_configfile = parse_config;
	return p;
}

const struct s1d135xx_pins g_s1d135xx_pins = {
	RUDDOCK_RESET,
	RUDDOCK_CS,
	RUDDOCK_HIRQ,
	PL_GPIO_NONE,
	RUDDOCK_HDC,
	PL_GPIO_NONE, //EPSON_CLK_EN,
	RUDDOCK_5V_EN,
};

const struct s1d135xx_pins g_s1d13524_falcon_p_pins = {
	RUDDOCK_RESET,
	RUDDOCK_CS,
	RUDDOCK_HIRQ,
	PL_GPIO_NONE,
	RUDDOCK_HDC,
	PL_GPIO_NONE, //EPSON_CLK_EN,
	RUDDOCK_5V_EN,
};

const struct s1d135xx_pins g_s1d13524_chiffchaff_pins = {
	RUDDOCK_RESET,
	CHIFFCHAFF_CS,
	RUDDOCK_HIRQ,
	PL_GPIO_NONE, //EPSON_HRDY,
	RUDDOCK_HDC,
	PL_GPIO_NONE, //EPSON_CLK_EN,
	RUDDOCK_5V_EN,
};

const struct s1d135xx_pins g_HBZ1_3_pins = {
	PL_GPIO_NONE,	// Reset signal is generated by HBz1.3
	RUDDOCK_CS,
	HBZ1_3_HIRQ,
	PL_GPIO_NONE, //EPSON_HRDY,
	RUDDOCK_HDC,
	PL_GPIO_NONE, //EPSON_CLK_EN,
	PL_GPIO_NONE,
};

const struct s1d135xx_pins g_HBZ6_3_pins = {
	RUDDOCK_RESET,
	RUDDOCK_CS,
	RUDDOCK_HIRQ,
	PL_GPIO_NONE, //EPSON_HRDY,
	RUDDOCK_HDC,
	PL_GPIO_NONE, //EPSON_CLK_EN,
	RUDDOCK_RESERVE_2,
};

const struct it8951_pins g_it8951_falcon_pins = {
	FALCON_I80_RESET_N,
	FALCON_I80_HRDY,
	FALCON_I80_HCS_N,
	FALCON_I80_HDC,
	FALCON_I80_HWE_N,
	FALCON_I80_HRD_N,
	FALCON_SPI_CS_ITE,  //for SPI communication
	// more for the parallel bus???
};

/**
 * Initializes control system dependent settings (e.g. I2C port) based on the selected system.
 * @param selection  defines which type of control system is used
 * @return allways pass (0)
 * @see control_system
 */
static int initialize_control_system(hw_setup_t *p,const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	if (!strcmp(selection, "BEAGLEBONE_WHITE" ))
		p->i2c_port = 3;
	else if (!strcmp(selection, "BEAGLEBONE_BLACK" ))
		// beaglebone black (using i2c-2 (hw descripion), but mapped to /dev/i2c-1)
		p->i2c_port = 1;
	else {
		LOG("Given control system type %s not supported.", selection);
		return -EINVAL;
	}

	return 0;
}


/**
 * Initializes driver board dependent settings (e.g. gpio pin list) based on the selected board.
 * @param selection  defines which type of driver board is used
 * @return allways pass (0)
 * @see driving_board
 */
static int initialize_driver_board(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in");
		return -EINVAL;
	}
	p->boardname = (char*) selection;
	p->sInterfaceType = SPI;
	if (!strcmp(selection, "CHIFFCHAFF" )){
		p->s1d13524_pins = &g_s1d13524_chiffchaff_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_chiffchaff_gpios;
		p->gpio_count = ARRAY_SIZE(g_chiffchaff_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
	}
	else if (!strcmp(selection, "RUDDOCK" ) || !strcmp(selection, "FALCON_SERIAL" )){
		p->s1d13524_pins = &g_s1d135xx_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_ruddock_gpios;
		p->gpio_count = ARRAY_SIZE(g_ruddock_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
	}
	else if (!strcmp(selection, "RUDDOCK_PARALLEL" )){
		p->s1d13524_pins = &g_s1d13524_falcon_p_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_ruddock_parallel_gpios;
		p->gpio_count = ARRAY_SIZE(g_ruddock_parallel_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
		p->sInterfaceType = PARALLEL;
	}
	else if (!strcmp(selection, "FALCON_PARALLEL" )){
		p->s1d13524_pins = &g_s1d13524_falcon_p_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_falcon_parallel_gpios;
		p->gpio_count = ARRAY_SIZE(g_falcon_parallel_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
		p->sInterfaceType = PARALLEL;
	}
	else if (!strcmp(selection, "FALCON_I80" )){
		p->it8951_pins = &g_it8951_falcon_pins;
		p->s1d13524_pins = &g_s1d13524_falcon_p_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_falcon_i80_gpios;
		p->gpio_count = ARRAY_SIZE(g_falcon_i80_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
		p->sInterfaceType = I80;
	}
	else if (!strcmp(selection, "FALCON_SPI" )){
		p->it8951_pins = &g_it8951_falcon_pins;
		p->s1d13524_pins = &g_s1d13524_falcon_p_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_falcon_spi_gpios;
		p->gpio_count = ARRAY_SIZE(g_falcon_spi_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
		p->sInterfaceType = SPI_HRDY;
	}
	else if (!strcmp(selection, "FALCON" )){
		p->s1d13524_pins = &g_s1d135xx_pins;
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_falcon_gpios;
		p->gpio_count = ARRAY_SIZE(g_falcon_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
	}
	else if (!strcmp(selection, "HBZ1_3" )){
		p->s1d13541_pins = &g_HBZ1_3_pins;
		p->board_gpios = g_HBZ1_3_gpios;
		p->gpio_count = ARRAY_SIZE(g_HBZ1_3_gpios);
		p->vddGPIO = HBZ1_3_3V3_EN;
	}
	else if (!strcmp(selection, "HBZ6_3" )){
		p->s1d13541_pins = &g_s1d135xx_pins;
		p->board_gpios = g_hbz6_3_gpios;
		p->gpio_count = ARRAY_SIZE(g_ruddock_gpios);
		p->vddGPIO = RUDDOCK_5V_EN;
		p->vddGPIO = RUDDOCK_5V_EN;
	}
	else {
		LOG("Given board type %s not supported.", selection);
		return -EINVAL;
	}

	return 0;
}


/**
 * Implements singleton pattern for MAXIM Max17135 PMIC variable.
 * @return instance of MAX17135 pmic
 * @see pl_pmic_t
 */
static pl_pmic_t *get_max17135_instance(hw_setup_t *p){
	if (max17135 == NULL){
		// get new pointer to max17135 object
		max17135 = max17135_new(&(p->host_i2c), I2C_PMIC_ADDR_MAX17135);

		if (max17135->configure(max17135, &(p->g_vcom_cal))){
			// free up created max17135 object since an error occured
			free(max17135);
			max17135 = NULL;
		}
	}

	return max17135;
}

/**
 * Implements singleton pattern for EPSON S1D13524 variable.
 * @return instance of S1D13524
 * @see s1d135xx_t
 */
static s1d135xx_t *get_s1d13524_instance(hw_setup_t *p){
	if (s1d13524 == NULL){
		s1d13524 = s1d13524_new(&(p->gpios), p->sInterface, &(p->host_i2c), p->s1d13524_pins);
	}

	return s1d13524;
}

/**
 * Implements singleton pattern for EPSON S1D13541 variable.
 * @return instance of S1D13541
 * @see s1d135xx_t
 */
static s1d135xx_t *get_s1d13541_instance(hw_setup_t *p){
	if (s1d13541 == NULL){
		s1d13541 = s1d13541_new(&(p->gpios), (pl_generic_interface_t*) p->sInterface, &(p->host_i2c), p->s1d13541_pins);
	}

	return s1d13541;
}

/**
 * Implements singleton pattern for ITE IT8951 variable.
 * @return instance of IT8951
 * @see it8951_t
 */
static it8951_t *get_it8951_instance(hw_setup_t *p){
	if (it8951 == NULL){
		it8951 = it8951_new(&(p->gpios), (pl_generic_interface_t*) p->sInterface, &(p->sInterfaceType), &(p->host_i2c), p->it8951_pins);
	}

	return it8951;
}

/**
 * Implements singleton pattern for ITE IT8951 variable.
 * @return instance of IT8951
 * @see it8951_t
 */
static pl_pmic_t *get_it8951_pmic_instance(hw_setup_t *p){
	if (it8951_pmic == NULL){
		it8951_pmic = (pl_pmic_t *)malloc(sizeof(pl_pmic_t));
		it8951_pmic->hw_ref = it8951_new(&(p->gpios), (pl_generic_interface_t*) p->sInterface, &(p->sInterfaceType), &(p->host_i2c), p->it8951_pins);
		it8951_pmic->cal = &p->g_vcom_cal;
	}

	return it8951_pmic;
}

/**
 * Implements singleton pattern for Texas Instruments TPS65185 PMIC variable.
 * @return instance of TPS65185 pmic
 * @see pl_pmic_t
 */
static pl_pmic_t *get_tps65185_instance(hw_setup_t *p){
	if (tps65185 == NULL){
		// get new pointer to max17135 object
		tps65185 = tps65185_new(&(p->host_i2c), I2C_PMIC_ADDR_TPS65185);

		if (tps65185->configure(tps65185, &(p->g_vcom_cal))){
			// free up created max17135 object since an error occured
			free(tps65185);
			tps65185 = NULL;
		}
	}

	return tps65185;
}


static int initialize_nvm(hw_setup_t *p, const char *selection, const char *format_str){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	// intialize nvm

	if (!strcmp(selection, "MICROCHIP_24AA256" )){
			p->nvm = pl_nvm_new();
			nvm_24AA256_i2c_init(p->nvm, &(p->host_i2c));
	}
	else if (!strcmp(selection, "MICROCHIP_24LC014H" )){
			p->nvm = pl_nvm_new();
			nvm_24LC014H_i2c_init(p->nvm, &(p->host_i2c));
	}
	else if(!strcmp(selection, "MACRONIX_MX25U4033E")){
		LOG("%s: Boardname: %s", __func__, p->boardname);
		if(!strcmp("FALCON_PARALLEL", p->boardname)){
			p->nvmSPI->cs_gpio = FALCON_DISPLAY_NVM_CS;
		}else if(!strcmp("FALCON_I80", p->boardname)){
			p->nvmSPI->cs_gpio = FALCON_FIRMWARE_NVM_CS;
		}else if(!strcmp("FALCON_SPI", p->boardname)){
			p->nvmSPI->cs_gpio = FALCON_DISPLAY_NVM_CS;
		}else if(!strcmp("CHIFFCHAFF", p->boardname)){
			p->nvmSPI->cs_gpio = CHIFFCHAFF_NVM_CS;
		}
			p->nvm = pl_nvm_new();
			nvm_MX25U4033E_spi_init(p->nvm, p->nvmSPI);
	}
	else if (!strcmp(selection, "S1D13541" )){
			p->nvm = s1d13541_get_nvm(get_s1d13541_instance(p), &(p->host_i2c), p->hvDriver, p->hvTiming);
	}
	else if (!strcmp(selection, "NULL" )){
			p->nvm = NULL;
			LOG("Disable usage of display nvm...");
	}
	else {
		LOG("Disable usage of display nvm...");
	}

	if (p->nvm != NULL){
		// set nvm content format
		if (!strcmp(format_str, "S1D13541")){
			p->nvm->nvm_format = NVM_FORMAT_S1D13541;
		}
		else if (!strcmp(format_str, "PLAIN")){
			p->nvm->nvm_format = NVM_FORMAT_PLAIN;
		}
		else if (!strcmp(format_str, "S040")){
			p->nvm->nvm_format = NVM_FORMAT_S040;
		}
		else if (!strcmp(format_str, "EPSON")){
			p->nvm->nvm_format = NVM_FORMAT_EPSON;
		}
		else {
			LOG("Given nvm format (%s) not supported.", format_str);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * Creates new hv config object based on selected hardware.
 * @param hv reference to a generic hv object, in which the newly created object will be stored
 * @param selection defines which type of hv config will be created
 * @return -1 if selected type is not supported, otherwise 0
 * @see hv_config_choises
 * @see pl_hv_t
 */
static int initialize_hv_config(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	if (!strcmp(selection, "MAX17135" ))
		p->hvConfig = max17135_get_hv_config(get_max17135_instance(p));

	else if (!strcmp(selection, "TPS65185" ))
		p->hvConfig = tps65185_get_hv_config(get_tps65185_instance(p));

	else if (!strcmp(selection, "NULL" ))
		p->hvConfig = NULL;

	else {
		LOG("Given EPD controller type not supported.");
		return -EINVAL;
	}

	return 0;
}

/**
 * Creates new vcom driver object based on selected hardware.
 * @param hv reference to a generic hv object, in which the newly created object will be stored
 * @param selection defines which type of vcom driver will be created
 * @return -1 if selected type is not supported, otherwise 0
 * @see vcom_driver_choises
 * @see pl_hv_t
 */
static int initialize_vcom_driver(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	if (!strcmp(selection, "MAX17135" ))
		p->vcomDriver = max17135_get_vcom_driver(get_max17135_instance(p));

	else if (!strcmp(selection, "MAX8520" )){
		LOG("not yet implemented.");
		return -ENOSYS;
	}

	else if (!strcmp(selection, "TPS65185" ))
		p->vcomDriver = tps65185_get_vcom_driver(get_tps65185_instance(p));

	else if (!strcmp(selection, "S1D13524" ))
		p->vcomDriver = s1d135xx_get_vcom_driver(get_s1d13524_instance(p));

	else if (!strcmp(selection, "S1D13541" ))
		p->vcomDriver = s1d135xx_get_vcom_driver(get_s1d13541_instance(p));

	else if (!strcmp(selection, "NULL" ))
		p->vcomDriver = NULL;

	else {
		LOG("Given EPD controller type not supported.");
		return -EINVAL;
	}

	return 0;
}

/**
 * Creates new vcom config object based on selected hardware.
 * @param hv reference to a generic hv object, in which the newly created object will be stored
 * @param selection defines which type of vcom config will be created
 * @return -1 if selected type is not supported, otherwise 0
 * @see vcom_config_choises
 * @see pl_hv_t
 */
static int initialize_vcom_config(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	if (!strcmp(selection, "MAX17135" ))
		p->vcomConfig = max17135_get_vcom_config(get_max17135_instance(p));

	else if (!strcmp(selection, "MAX8520" )){
		dac_max5820_t *max5820 = dac_max5820_new(&(p->host_i2c), MAX5820_I2C_ADDRESS);
		max5820->configure(max5820, &(p->g_vcom_cal));
		max5820->set_output_mode(max5820, 0x03, 0x00);
		p->vcomConfig = dac_max5820_get_vcom_config(max5820);
	}

	else if (!strcmp(selection, "TPS65185" ))
		p->vcomConfig = tps65185_get_vcom_config(get_tps65185_instance(p));

	else if (!strcmp(selection, "IT8951" ))
			p->vcomConfig = it8951_get_vcom_config(get_it8951_pmic_instance(p));

	else if (!strcmp(selection, "NULL" ))
		p->vcomConfig = NULL;

	else {
		LOG("Given EPD controller type not supported.");
		return -EINVAL;
	}

	return 0;
}
/**
 * Creates new vcom switch object based on selected hardware.
 * @param hv reference to a generic hv object, in which the newly created object will be stored
 * @param selection defines which type of vcom switch will be created
 * @return -1 if selected type is not supported, otherwise 0
 * @see vcom_switch_choises
 * @see pl_hv_t
 */
static int initialize_vcom_switch(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	if (!strcmp(selection, "NULL" ))
		p->vcomSwitch = NULL;

	else if (!strcmp(selection, "GPIO" ))
		p->vcomSwitch = beaglebone_get_vcom_switch(&(p->gpios));

	else if (!strcmp(selection, "TPS65185" ))
		p->vcomSwitch = tps65185_get_vcom_switch(get_tps65185_instance(p));

	else if (!strcmp(selection, "S1D13524" ))
		p->vcomSwitch = s1d135xx_get_vcom_switch(get_s1d13524_instance(p));

	else if (!strcmp(selection, "S1D13541" ))
		p->vcomSwitch = s1d135xx_get_vcom_switch(get_s1d13541_instance(p));

	else {
		LOG("Given EPD controller type not supported.");
		return -EINVAL;
	}

	return 0;
}
/**
 * Creates new hv timing object based on selected hardware.
 * @param hv reference to a generic hv object, in which the newly created object will be stored
 * @param selection defines which type of hv timing will be created
 * @return -1 if selected type is not supported, otherwise 0
 * @see hv_timing_choises
 * @see pl_hv_t
 */
static int initialize_hv_timing(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	int stat = 0;

	if (!strcmp(selection, "MAX17135" ))
		p->hvTiming = max17135_get_hv_timing(get_max17135_instance(p));

	else if (!strcmp(selection, "TPS65185" ))
		p->hvTiming = tps65185_get_hv_timing(get_tps65185_instance(p));

	else if (!strcmp(selection, "NULL" ))
		p->hvTiming = NULL;

	else {
		LOG("Given EPD controller type not supported.");
		return -EINVAL;
	}

	return stat;
}
/**
 * Creates new hv driver object based on selected hardware.
 * @param hv reference to a generic hv object, in which the newly created object will be stored
 * @param selection defines which type of hv driver will be created
 * @return -1 if selected type is not supported, otherwise 0
 * @see hv_driver_choises
 * @see pl_hv_t
 */
static int initialize_hv_driver(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	if (!strcmp(selection, "MAX17135" ))
		p->hvDriver = max17135_get_hv_driver(get_max17135_instance(p));

	else if (!strcmp(selection, "TPS65185" ))
		p->hvDriver = tps65185_get_hv_driver(get_tps65185_instance(p));

	else if (!strcmp(selection, "S1D13524" ))
		p->hvDriver = s1d135xx_get_hv_driver(get_s1d13524_instance(p));

	else if (!strcmp(selection, "S1D13541" ))
		p->hvDriver = s1d135xx_get_hv_driver(get_s1d13541_instance(p));

	else if (!strcmp(selection, "IT8951" ))
		p->hvDriver = it8951_get_hv_driver(get_it8951_instance(p));

	else if (!strcmp(selection, "GPIO" ))
		p->hvDriver = beaglebone_get_hv_driver(&(p->gpios));

	else if (!strcmp(selection, "NULL"))
		p->hvDriver = NULL;

	else {
		LOG("Given EPD controller type not supported.");
		return -EINVAL;
	}

	return 0;
}

/**
 * Creates a new generic controller object based on given controller selection.
 *
 * @param controller reference to a generic controller object, which will be update by newly created object
 * @param selection defines which type of controller will be created
 * @return -1 if selected controller type is not supported, otherwise 0
 * @see epd_controller_type
 * @see pl_generic_controller_t
 */
static int initialize_controller(hw_setup_t *p, const char *selection){
	if (p == NULL || selection ==  NULL){
		LOG("Parameter exception in %s!", __func__);
		return -EINVAL;
	}

	p->controller = generic_controller_new();

	if (!strcmp(selection, "S1D13524" )) {
		s1d13524 = get_s1d13524_instance(p);
		s1d13524_controller_setup(p->controller, s1d13524);
	}
	else if (!strcmp(selection, "S1D13541" )){
		s1d13541 = get_s1d13541_instance(p);
		s1d13541_controller_setup(p->controller, s1d13541);
	}
	else if (!strcmp(selection, "IT8951" )){
		it8951 = get_it8951_instance(p);
		it8951_controller_setup(p->controller, it8951);
	}
	else if (!strcmp(selection, "NONE" )){

	}
	else {
		LOG("Given EPD controller type not supported.");
		p->controller->delete(p->controller);
		return -EINVAL;
	}

	return 0;
}
