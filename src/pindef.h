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
 * pindef.h
 *
 *  Created on: 27.04.2015
 *      Author: sebastian.friebe
 */

#ifndef PINDEF_H_
#define PINDEF_H_

#include "pl/gpio.h"
#include "beaglebone/beaglebone-gpio.h"
#include <errno.h>

#define VCOM_EN         BEAGLEBONE_GPIO(0,14) //P9-26 // VCOM switch enable
#define PMIC_EN           BEAGLEBONE_GPIO(1,14) //P8-16 // HV-PMIC enable
#define PMIC_POK          BEAGLEBONE_GPIO(0,15) //P9-24 // HV-PMIC power OK
#define PMIC_FLT          BEAGLEBONE_GPIO(0,27) //P8-17 // HV-PMIC fault condition

#define RUDDOCK_HIRQ   		BEAGLEBONE_GPIO(3,21)  //P9-25
#define RUDDOCK_HDC    		BEAGLEBONE_GPIO(1,17)  //P9-23
#define RUDDOCK_RESET  		BEAGLEBONE_GPIO(0,26)  //P8-14
#define RUDDOCK_CS   		BEAGLEBONE_GPIO(0,5)   //P9-17
#define RUDDOCK_HRDY      	BEAGLEBONE_GPIO(3,19)  //P9-27
#define RUDDOCK_5V_EN		BEAGLEBONE_GPIO(1,15)  //P8-15
#define RUDDOCK_RESERVE_2	BEAGLEBONE_GPIO(1,13)  //P8-11

#define RUDDOCK_HDB0		BEAGLEBONE_GPIO(2,6)   //P8-45
#define RUDDOCK_HDB1		BEAGLEBONE_GPIO(2,7)   //P8-46
#define RUDDOCK_HDB2		BEAGLEBONE_GPIO(2,8)   //P8-43
#define RUDDOCK_HDB3		BEAGLEBONE_GPIO(2,9)   //P8-44
#define RUDDOCK_HDB4		BEAGLEBONE_GPIO(2,10)  //P8-41
#define RUDDOCK_HDB5		BEAGLEBONE_GPIO(2,11)  //P8-42
#define RUDDOCK_HDB6		BEAGLEBONE_GPIO(2,12)  //P8-39
#define RUDDOCK_HDB7		BEAGLEBONE_GPIO(2,13)  //P8-40
#define RUDDOCK_HDB8		BEAGLEBONE_GPIO(2,14)  //P8-37
#define RUDDOCK_HDB9		BEAGLEBONE_GPIO(2,15)  //P8-38
#define RUDDOCK_HDB10		BEAGLEBONE_GPIO(2,16)  //P8-36
#define RUDDOCK_HDB11		BEAGLEBONE_GPIO(2,17)  //P8-34
#define RUDDOCK_HDB12		BEAGLEBONE_GPIO(0,8)   //P8-35
#define RUDDOCK_HDB13		BEAGLEBONE_GPIO(0,9)   //P8-33
#define RUDDOCK_HDB14		BEAGLEBONE_GPIO(0,10)  //P8-31
#define RUDDOCK_HDB15		BEAGLEBONE_GPIO(0,11)  //P8-32


#define CHIFFCHAFF_32MHZ_EN BEAGLEBONE_GPIO(0,22)  //P8-19
#define CHIFFCHAFF_CS		BEAGLEBONE_GPIO(1,16)  //P9-15
#define CHIFFCHAFF_NVM_CS	BEAGLEBONE_GPIO(0,31)  //P9-13

#define HBZ1_3_HIRQ 		BEAGLEBONE_GPIO(0,20)  //P9-41
#define HBZ1_3_PMIC_EN 		BEAGLEBONE_GPIO(3,21)  //P9-25
#define HBZ1_3_3V3_EN		BEAGLEBONE_GPIO(1,28)  //P9-12

#define FALCON_NVM_CS		BEAGLEBONE_GPIO(3,17)	//P9-28


static const struct pl_gpio_config g_chiffchaff_gpios[] = {
	{CHIFFCHAFF_32MHZ_EN, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ VCOM_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_POK,   PL_GPIO_INPUT                   },
	{ PMIC_FLT,   PL_GPIO_INPUT                   },
	{ RUDDOCK_5V_EN,  		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HRDY,    		PL_GPIO_INPUT  | PL_GPIO_INIT_H },  // needed for ChiffChaff
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESET,   		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ CHIFFCHAFF_CS,      	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESERVE_2,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ CHIFFCHAFF_NVM_CS,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};

static const struct pl_gpio_config g_ruddock_gpios[] = {
	{ VCOM_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_POK,   PL_GPIO_INPUT                   },
	{ PMIC_FLT,   PL_GPIO_INPUT                   },
	{ RUDDOCK_5V_EN,  		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HRDY,    		PL_GPIO_INPUT  | PL_GPIO_INIT_H },  // needed for Ruddock
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESET,   		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESERVE_2,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};

static const struct pl_gpio_config g_falcon_gpios[] = {
	{ VCOM_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_POK,   PL_GPIO_INPUT                   },
	{ PMIC_FLT,   PL_GPIO_INPUT                   },
	{ RUDDOCK_5V_EN,  		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HRDY,    		PL_GPIO_INPUT  | PL_GPIO_INIT_H },  // needed for Ruddock
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESET,   		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ FALCON_NVM_CS,      	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESERVE_2,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};


static const  struct pl_gpio_config g_ruddock_parallel_gpios[] = {
	{ VCOM_EN,    			PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_EN,    			PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_POK,   			PL_GPIO_INPUT                   },
	{ PMIC_FLT,   			PL_GPIO_INPUT                   },
	{ RUDDOCK_5V_EN,  		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HRDY,    		PL_GPIO_INPUT  | PL_GPIO_INIT_H },  // needed for Ruddock parallel
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESET,   		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESERVE_2,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_HDB0,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB1,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB2,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB3,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB4,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB5,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB6,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB7,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB8,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB9,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB10,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB11,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB12,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB13,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB14,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB15,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
};

static const  struct pl_gpio_config g_falcon_parallel_gpios[] = {
	{ VCOM_EN,    			PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_EN,    			PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_POK,   			PL_GPIO_INPUT                   },
	{ PMIC_FLT,   			PL_GPIO_INPUT                   },
	{ RUDDOCK_5V_EN,  		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HRDY,    		PL_GPIO_INPUT  | PL_GPIO_INIT_H },  // needed for Ruddock parallel
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESET,   		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ FALCON_NVM_CS,      	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESERVE_2,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_HDB0,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB1,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB2,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB3,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB4,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB5,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB6,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB7,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB8,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB9,			PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB10,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB11,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB12,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB13,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB14,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HDB15,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
};

static const struct pl_gpio_config g_HBZ1_3_gpios[] = {
	{ HBZ1_3_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ VCOM_EN,  			PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ HBZ1_3_PMIC_EN,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_POK,   			PL_GPIO_INPUT                   },
	{ HBZ1_3_3V3_EN,		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
};

static const struct pl_gpio_config g_hbz6_3_gpios[] = {
	{ VCOM_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ PMIC_POK,   PL_GPIO_INPUT                   },
	{ PMIC_FLT,   PL_GPIO_INPUT                   },
	{ RUDDOCK_5V_EN,  		PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ RUDDOCK_HIRQ,    		PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ RUDDOCK_HRDY,    		PL_GPIO_INPUT  | PL_GPIO_INIT_H },  // needed for Ruddock
	{ RUDDOCK_HDC,     		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESET,   		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_CS,      		PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ RUDDOCK_RESERVE_2,   	PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};
#endif /* PINDEF_H_ */
