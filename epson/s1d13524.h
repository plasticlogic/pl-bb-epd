/*
 * s1d13524.h
 *
 *  Created on: 30.01.2015
 *      Author: Bastus
 */

#ifndef S1D13524_H_
#define S1D13524_H_

#include "epson-s1d135xx.h"

#define S1D13524_PROD_CODE              0x004F
#define S1D13524_STATUS_HRDY            (1 << 5)
#define S1D13524_PLLCFG0                0x340F
#define S1D13524_PLLCFG1                0x0300
#define S1D13524_PLLCFG2                0x1680
#define S1D13524_PLLCFG3                0x1880
#define S1D13524_I2C_CLOCK_DIV          7 /* 400 kHz */
#define S1D13524_I2C_DELAY              3
#define S1D13524_AUTO_RETRIEVE_ON       0x0000
#define S1D13524_AUTO_RETRIEVE_OFF      0x0001
#define S1D13524_LD_IMG_4BPP            (0 << 4)
#define S1D13524_LD_IMG_8BPP            (1 << 4)
#define S1D13524_LD_IMG_16BPP           (2 << 4)
#define S1D13541_WF_CHECKSUM_ERROR      0x1F00
#define S1D13524_CTLR_AUTO_WFID         0x0200
#define S1D13524_CTLR_NEW_AREA_PRIORITY 0x4000
#define S1D13524_CTLR_PROCESSED_SINGLE  0x0000
#define S1D13524_CTLR_PROCESSED_DOUBLE  0x0001
#define S1D13524_CTLR_PROCESSED_TRIPLE  0x0002

enum s1d13524_reg {
	S1D13524_REG_POWER_SAVE_MODE    = 0x0006,
	S1D13524_REG_FRAME_DATA_LENGTH  = 0x0300,
	S1D13524_REG_LINE_DATA_LENGTH   = 0x0306,
	S1D13524_REG_TEMP_AUTO_RETRIEVE = 0x0320,
	S1D13524_REG_TEMP               = 0x0322,
	S1D13541_REG_WF_ADDR_0          = 0x0390,
	S1D13541_REG_WF_ADDR_1          = 0x0392,
};

enum s1d13524_cmd {
	S1D13524_CMD_INIT_PLL           = 0x01,
	S1D13524_CMD_INIT_CTLR_MODE     = 0x0E,
	S1D13524_CMD_RD_WF_INFO         = 0x30,
};

#endif /* S1D13524_H_ */
