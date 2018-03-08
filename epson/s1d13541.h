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
 * s1d13541.h
 *
 *  Created on: 30.01.2015
 *      Author: Bastus
 */

#include "epson-s1d135xx.h"
#include <errno.h>
#ifndef S1D13541_H_
#define S1D13541_H_

#define S1D13541_PROD_CODE              0x0053
#define S1D13541_STATUS_HRDY            (1 << 13)
#define S1D13541_INTERNAL_CLOCK_ENABLE  (1 << 7)
#define S1D13541_I2C_CLOCK_DIV          7 /* 100 kHz */
#define S1D13541_PROT_KEY_1             0x5678 /* ToDo: add to s1d135xx_data */
#define S1D13541_PROT_KEY_2             0x1234
#define S1D13541_TEMP_SENSOR_CONTROL    (1 << 14)
#define S1D13541_TEMP_SENSOR_EXTERNAL   (1 << 6)
#define S1D13541_AUTO_TEMP_JUDGE_EN     (1 << 2)
#define S1D13541_GENERIC_TEMP_EN        (1 << 15)
#define S1D13541_GENERIC_TEMP_JUDGE_EN  (1 << 14)
#define S1D13541_GENERIC_TEMP_MASK      0x01FF
#define S1D13541_INT_RAW_WF_UPDATE      (1 << 14)
#define S1D13541_INT_RAW_OUT_OF_RANGE   (1 << 10)
#define S1D13541_LD_IMG_1BPP            (0 << 4)
#define S1D13541_LD_IMG_2BPP            (1 << 4)
#define S1D13541_LD_IMG_4BPP            (2 << 4)
#define S1D13541_LD_IMG_8BPP            (3 << 4)
#define S1D13541_WF_ADDR                0x00080000L

enum s1d13541_reg {
	S1D13541_REG_CLOCK_CONFIG          = 0x0010,
	S1D13541_REG_PROT_KEY_1            = 0x042C,
	S1D13541_REG_PROT_KEY_2            = 0x042E,
	S1D13541_REG_FRAME_DATA_LENGTH     = 0x0400,
	S1D13541_REG_LINE_DATA_LENGTH      = 0x0406,
	S1D13541_REG_WF_DECODER_BYPASS     = 0x0420,
	S1D13541_REG_TEMP_SENSOR_VALUE     = 0x0576,
	S1D13541_REG_GENERIC_TEMP_CONFIG   = 0x057E,
};

enum s1d13541_cmd {
	S1D13541_CMD_RD_TEMP               = 0x12,
};


#endif /* S1D13541_H_ */
