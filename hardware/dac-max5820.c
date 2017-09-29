/*
 * dac-max5820.c
 *
 *  Created on: 25.07.2014
 *      Author: sebastian.friebe
 */

#include <pl/i2c.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "pl/vcom.h"
#include <hardware/dac-max5820.h>
#define LOG_TAG "max5820"
#include "pl/utils.h"

#define MAX5820_DAC_MIN 0x00
#define MAX5820_DAC_MAX 0xFF

#define MAX5820_LOAD_A_OUTPUT_BOTH 			0x00
#define MAX5820_LOAD_B_OUTPUT_BOTH 			0x10
#define MAX5820_LOAD_A_OUTPUT_NONE 			0x40
#define MAX5820_LOAD_B_OUTPUT_NONE 			0x50
#define MAX5820_UPDATE_BOTH_FIRST_LOAD_A 	0x80
#define MAX5820_UPDATE_BOTH_FIRST_LOAD_B 	0x90
#define MAX5820_LOAD_AND_UPDATE_BOTH 		0xC0
#define MAX5820_LOAD_BOTH_UPDATE_NONE 		0xD0
#define MAX5820_READ_A				 		0xF1
#define MAX5820_READ_B				 		0xF2
#define MAX5820_EXTENDED_CMD				0xF0

static void dac_max5820_delete(dac_max5820_t *p);
static int dac_max5820_configure(dac_max5820_t* p, struct vcom_cal *cal);
static int dac_max5820_set_output_mode(dac_max5820_t *p, int channel_mask, int output_mode);
static int dac_max5820_set_vcom_voltage(dac_max5820_t *p, int mv);
static int dac_max5820_set_vcom_register(dac_max5820_t *p, int dac_value);

static int dac_max5820_get_vcom_register(dac_max5820_t *p, int *dac_value);


// -----------------------------------------------------------------------------
// private interface functions
// ------------------------------
/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 * @todo delete all internal stuff
 */
static void dac_max5820_delete(dac_max5820_t *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

dac_max5820_t *dac_max5820_new(struct pl_i2c *i2c, uint8_t i2c_addr)
{
	dac_max5820_t *p = (dac_max5820_t *)malloc(sizeof(dac_max5820_t));

	assert(i2c);
	assert(i2c_addr);

	p->i2c_addr = i2c_addr;
	p->i2c = i2c;

	p->configure = dac_max5820_configure;
	p->set_output_mode = dac_max5820_set_output_mode;
	p->set_vcom_voltage = dac_max5820_set_vcom_voltage;
	p->set_vcom_register = dac_max5820_set_vcom_register;
	p->get_vcom_register = dac_max5820_get_vcom_register;
	p->delete = dac_max5820_delete;

	return p;
}

static int dac_max5820_configure(dac_max5820_t *p, struct vcom_cal *cal)
{
	assert(p);
	assert(cal);

	p->cal = cal;

	return 0;
}

static int dac_max5820_set_vcom_voltage(dac_max5820_t *p, int mv){
	uint8_t dac_value;

	assert(p);

	dac_value = vcom_calculate(p->cal, mv);

	if (dac_value < MAX5820_DAC_MIN)
		dac_value = MAX5820_DAC_MIN;
	else if (dac_value > MAX5820_DAC_MAX)
		dac_value = MAX5820_DAC_MAX;

	uint8_t xferBytes[2] = {0};
	xferBytes[0] = MAX5820_LOAD_AND_UPDATE_BOTH | (dac_value >> 4);
	xferBytes[1] = (dac_value << 4);

	if (pl_i2c_reg_write_8(p->i2c, p->i2c_addr, xferBytes[0], xferBytes[1]))
		return -1;

	return 0;
}

static int dac_max5820_set_vcom_register(dac_max5820_t *p, int dac_value)
{
	assert(p);

	uint8_t xferBytes[2] = {0};
	xferBytes[0] = MAX5820_LOAD_AND_UPDATE_BOTH | ((uint8_t)dac_value >> 4);
	xferBytes[1] = ((uint8_t)dac_value << 4);

	return pl_i2c_reg_write_8(p->i2c, p->i2c_addr, xferBytes[0], xferBytes[1]);
}


static int dac_max5820_get_vcom_register(dac_max5820_t *p, int *dac_value)
{
	assert(p);

	uint8_t data[2];
	if (pl_i2c_reg_read_16be(p->i2c, p->i2c_addr, MAX5820_READ_A, (uint16_t*)data))
		return -1;

	uint8_t val = (data[1] << 4) | (data[0] >> 4);
	*dac_value = val;

	return 0;
}

static int dac_max5820_set_output_mode(dac_max5820_t *p, int channel_mask, int output_mode){

	channel_mask = channel_mask & 0x03;
	output_mode = output_mode & 0x03;
	
	return pl_i2c_reg_write_8(p->i2c, p->i2c_addr, MAX5820_EXTENDED_CMD, (uint8_t)((channel_mask << 2) | output_mode));
}
