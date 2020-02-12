/*
 * beaglebone_i80.h
 *
 *  Created on: 24.01.2020
 *      Author: matti.haugwitz
 */

#ifndef BEAGLEBONE_I80_H_
#define BEAGLEBONE_I80_H_

#include <stddef.h>
#include <stdint.h>
#include <pl/global.h>
#include <pl/i80.h>
#include <pl/parallel.h>
#include <errno.h>

//typedef for variables
typedef unsigned char TByte; //1 byte
typedef unsigned short TWord; //2 bytes
typedef unsigned long TDWord; //4 bytes

typedef struct
{
    TWord usPanelW;
    TWord usPanelH;
    TWord usImgBufAddrL;
    TWord usImgBufAddrH;
    TWord usFWVersion[8]; //16 Bytes String
    TWord usLUTVersion[8]; //16 Bytes String

}I80IT8951DevInfo;

#define WAIT_FOR_READY_TIMEOUT_I80 10000

struct pl_gpio;

pl_parallel_t *beaglebone_i80_new(struct pl_gpio * hw_ref);

#endif /* BEAGLEBONE_I80_H_ */
