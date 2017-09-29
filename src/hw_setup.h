/*
 * hw_setup.h
 *
 *  Created on: 27.04.2015
 *      Author: sebastian.friebe
 */

#ifndef HW_SETUP_H_
#define HW_SETUP_H_

#include "config_defs.h"
#include "pl/gpio.h"
#include "pl/i2c.h"
#include "pl/spi.h"
#include "pl/parallel.h"
#include "epson/epson-s1d135xx.h"
#include "pl/pmic.h"
#include "pl/vcom.h"
#include "pl/nvm.h"
#include "pl/hwinfo.h"
#include "pl/hv.h"
#include "pl/generic_controller.h"
#include "pl/generic_interface.h"

typedef struct hw_setup {
	int default_vcom;
	int i2c_port;									//!< the I2C port used
	unsigned vddGPIO;
	struct pl_i2c host_i2c;							// host (beaglebone) i2c interface
	struct pl_gpio gpios;							// list of all gpios
	const struct s1d135xx_pins *epson_pins;
	const struct s1d135xx_pins *s1d13524_pins;		// pointer to selected pins for epson controller
	const struct s1d135xx_pins *s1d13541_pins;		// pointer to selected pins for epson controller
	const struct pl_gpio_config *board_gpios;		//!< pointer to list of used gpios (based on driver board)
	int gpio_count;									//!< number of gpios stored in board_gpios list
	struct vcom_cal g_vcom_cal;						// vcom structure
	struct pl_hw_vcom_info g_vcom_info;   			// Vcom + PMIC settings
	enum nvm_format_version nvm_format;

	int (*initialize_control_system)(struct hw_setup *p, const char *selection);
	int (*initialize_driver_board)(struct hw_setup *p, const char *selection);
	int (*initialize_nvm)(struct hw_setup *p, const char *selection, const char *format);
	int (*initialize_hv_config)(struct hw_setup *p, const char *selection);
	int (*initialize_vcom_driver)(struct hw_setup *p, const char *selection);
	int (*initialize_vcom_config)(struct hw_setup *p, const char *selection);
	int (*initialize_vcom_switch)(struct hw_setup *p, const char *selection);
	int (*initialize_hv_timing)(struct hw_setup *p, const char *selection);
	int (*initialize_hv_driver)(struct hw_setup *p, const char *selection);
	int (*initialize_controller)(struct hw_setup *p, const char *selection);
	int (*init_from_configfile)(struct hw_setup *p, const char *filename);

	//pl_parallel_t *sParallel;
	//pl_spi_t* sSPI;
	enum interfaceType sInterfaceType;
	pl_generic_interface_t* sInterface;
	pl_spi_t *nvmSPI;						//!< nvm spi port
	pl_nvm_t *nvm;							//!< display nvm
	pl_generic_controller_t *controller;	//!< epd controller
	pl_hv_config_t *hvConfig;
	pl_hv_driver_t *hvDriver;
	pl_hv_timing_t *hvTiming;
	pl_vcom_config_t *vcomConfig;
	pl_vcom_driver_t *vcomDriver;
	pl_vcom_switch_t *vcomSwitch;

} hw_setup_t;

#define I2C_PMIC_ADDR_MAX17135 0x48
#define I2C_PMIC_ADDR_TPS65185 0x68

hw_setup_t *hw_setup_new();

#endif /* HW_SETUP_H_ */
