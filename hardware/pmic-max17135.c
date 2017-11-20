/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

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
 * max17135-pmic.c -- Driver for the MAXIM MAX17135 HVPMIC device.
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <pl/i2c.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "pl/vcom.h"
#include "pmic-max17135.h"

#define LOG_TAG "max17135"
#include "pl/utils.h"

/* problems with temperature sensor return magic temperature value */
#define	HVPMIC_TEMP_INVALID	0x7FC0
#define	HVPMIC_TEMP_DEFAULT	20

#define	HVPMIC_DAC_MAX		((1 << 8)-1)
#define	HVPMIC_DAC_MIN		0

#define	HVPMIC_TEMP_BUSY	(1 << 0)
#define	HVPMIC_TEMP_OPEN	(1 << 1)
#define	HVPMIC_TEMP_SHORT	(1 << 2)

#define MAX17135_PROD_ID 0x4D

#if 0
#define MV_DIV	41			// Each DAC step is 41mV ((620/150)*10)
#define MV_OFFSET	2066	// DAC value of 0 => 2066mV
#endif

enum max17135_register {
	HVPMIC_REG_EXT_TEMP   = 0x00,
	HVPMIC_REG_CONF       = 0x01,
	HVPMIC_REG_INT_TEMP   = 0x04,
	HVPMIC_REG_TEMP_STAT  = 0x05,
	HVPMIC_REG_PROD_REV   = 0x06,
	HVPMIC_REG_PROD_ID    = 0x07,
	HVPMIC_REG_DVR        = 0x08,
	HVPMIC_REG_ENABLE     = 0x09,
	HVPMIC_REG_FAULT      = 0x0A,
	HVPMIC_REG_PROG       = 0x0C,
	HVPMIC_REG_TIMING_1   = 0x10,
	HVPMIC_REG_TIMING_2   = 0x11,
	HVPMIC_REG_TIMING_3   = 0x12,
	HVPMIC_REG_TIMING_4   = 0x13,
	HVPMIC_REG_TIMING_5   = 0x14,
	HVPMIC_REG_TIMING_6   = 0x15,
	HVPMIC_REG_TIMING_7   = 0x16,
	HVPMIC_REG_TIMING_8   = 0x17
};

static void max17135_delete(pl_pmic_t *p);
static int max17135_init(pl_pmic_t *p);
static int max17135_check_revision(pl_pmic_t *pmic);
static int max17135_load_timings(pl_pmic_t *pmic);
static int max17135_configure(pl_pmic_t *pmic, struct vcom_cal *cal);
static int max17135_set_vcom_register(pl_pmic_t *pmic, int dac_value);
static int max17135_set_vcom_voltage(pl_pmic_t *pmic, int mv);
static int max17135_get_vcom_voltage(pl_pmic_t *pmic);
static int max17135_wait_pok(pl_pmic_t *pmic);
static int max17135_hv_enable(pl_pmic_t *pmic);
static int max17135_hv_disable(pl_pmic_t *pmic);
static int max17135_vcom_enable(pl_pmic_t *pmic);
static int max17135_vcom_disable(pl_pmic_t *pmic);
static int max17135_temp_enable(pl_pmic_t *pmic);
static int max17135_temp_disable(pl_pmic_t *pmic);
static int max17135_temperature_measure(pl_pmic_t *pmic, int16_t *measured);

// -----------------------------------------------------------------------------
// constructor and setup function
// ------------------------------

/**
 * Constructor for a new MAX17135 object.
 * Initializes member functions as well as the hardware access information, as I2C object and I2C address.
 *
 * @param i2c pointer to an i2c interface implementation
 * @param i2c_addr i2c address used for the device
 * @return pointer to memory of a newly created max17135 object
 *
 * @todo add calibration for HV voltages
 */
pl_pmic_t *max17135_new(struct pl_i2c *i2c, uint8_t i2c_addr){
	pl_pmic_t *p = (pl_pmic_t *)malloc(sizeof(pl_pmic_t));

	assert(i2c);
	assert(p);

	p->i2c_addr = i2c_addr;
	p->i2c = i2c;
	p->cal = NULL;

	p->delete = max17135_delete;
	p->init = max17135_init;

	p->check_revision_code = max17135_check_revision;
	p->configure = max17135_configure;
	p->hv_disable = max17135_hv_disable;
	p->hv_enable = max17135_hv_enable;
	p->vcom_disable = max17135_vcom_disable;
	p->vcom_enable = max17135_vcom_enable;
	p->apply_timings = max17135_load_timings;
	p->set_vcom_register = max17135_set_vcom_register;
	p->set_vcom_voltage = max17135_set_vcom_voltage;
	p->get_vcom_voltage = max17135_get_vcom_voltage;
	p->temp_disable = max17135_temp_disable;
	p->temp_enable = max17135_temp_enable;
	p->temperature_measure = max17135_temperature_measure;
	p->wait_pok = max17135_wait_pok;

	p->is_initialized = false;
	return p;
}

// -----------------------------------------------------------------------------
// private interface functions
// ------------------------------
/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 * @todo delete all internal stuff
 */
static void max17135_delete(pl_pmic_t *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * Initializes PMIC.
 * Connection is verified by checking the revision code of the device,
 * if this function runs the first time.
 *
 * @param p pointer to the hv structure
 * @see pl_hv
 * @return success flag: 0 if passed, <> 0 otherwise
 * @todo init all internal stuff if required
 */
static int max17135_init(pl_pmic_t *p){
	assert(p != NULL);
	int stat = 0;

	if (p->is_initialized == false){

		stat = p->check_revision_code(p);

		p->is_initialized = true;
	}
	return stat;
}

/**
 * Applies internally stored timing values to the real hardware.
 *
 * @param pmic pointer to a pmic object
 * @return success of operation: 0 if passed; -2 if PMIC is not initialized; any other value if error occurred
 */
static int max17135_load_timings(pl_pmic_t *pmic)
{
	assert(pmic);
	if (pmic->is_initialized == false){
		return -2;
	}

	int stat = 0;

	// set power-on time offsets
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_1, pmic->vgl_on_offset_time);
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_2, pmic->vsl_on_offset_time);
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_3, pmic->vsh_on_offset_time);
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_4, pmic->vgh_on_offset_time);
	LOG("timings  on: vgl=%d, vsl=%d, vsh=%d, vgh=%d", pmic->vgl_on_offset_time, pmic->vsl_on_offset_time, pmic->vsh_on_offset_time, pmic->vgh_on_offset_time);

	// set_power-off time offsets
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_5, pmic->vgh_on_offset_time);
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_6, pmic->vsh_on_offset_time);
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_7, pmic->vsl_on_offset_time);
	stat |= pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TIMING_8, pmic->vgl_on_offset_time);

	LOG("timings off: vgl=%d, vsl=%d, vsh=%d, vgh=%d",pmic->vgl_off_offset_time, pmic->vsl_off_offset_time, pmic->vsh_off_offset_time, pmic->vgh_off_offset_time);

	return stat;
}

static int max17135_check_revision(pl_pmic_t *pmic)
{
	assert(pmic);
	uint8_t prod_rev;
	uint8_t prod_id;

	if (pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_PROD_REV, &prod_rev))
		return -1;

	if (pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_PROD_ID, &prod_id))
		return -1;

	LOG("rev 0x%02X, id 0x%02X", prod_rev, prod_id);

	if (prod_id != MAX17135_PROD_ID) {
		LOG("Invalid product ID");
		return -1;
	}

	return 0;
}

static int max17135_configure(pl_pmic_t *pmic, struct vcom_cal *cal)
{
	assert(pmic);

	/* cal may be null if not being used */
	pmic->cal = cal;

	return 0;
}

static int max17135_set_vcom_register(pl_pmic_t *pmic, int dac_value)
{
	assert(pmic);

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_DVR,
				  (uint8_t)dac_value);
}

static int max17135_set_vcom_voltage(pl_pmic_t *pmic, int mv)
{
	int dac_value;

	assert(pmic);

	dac_value = vcom_calculate(pmic->cal, mv);
	LOG("calculate: %i, %i", mv, dac_value);
	if (dac_value < HVPMIC_DAC_MIN)
		dac_value = HVPMIC_DAC_MIN;
	else if (dac_value > HVPMIC_DAC_MAX)
		dac_value = HVPMIC_DAC_MAX;

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_DVR,
				  (uint8_t)dac_value);
}

static int max17135_get_vcom_voltage(pl_pmic_t *pmic)
{
	int mv;
	uint8_t dac_value;
	assert(pmic);
	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_DVR, &dac_value);
	mv = vcom_calculate_dac(pmic->cal, dac_value);
#if VERBOSE
	LOG("calculate: %i, %i", mv, dac_value);
#endif
	return mv;
}

static int max17135_wait_pok(pl_pmic_t *pmic)
{
	static const unsigned POLL_DELAY_MS = 5;
	unsigned timeout = 100;
	int pok = 0;

	assert(pmic);

	while (!pok) {
		union max17135_fault fault;

		usleep(POLL_DELAY_MS*1000);

		if (pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr,
				      HVPMIC_REG_FAULT, &fault.byte)) {
			LOG("Failed to read HVPMIC POK");
			return -1;
		}

		pok = fault.pok;

		if (timeout > POLL_DELAY_MS) {
			timeout -= POLL_DELAY_MS;
		} else {
			timeout = 0;

			if (!pok) {
				LOG("POK timeout");
				return -1;
			}
		}
	}

	return 0;
}

/* use the i2c interface to power up the PMIC */
static int max17135_hv_enable(pl_pmic_t *pmic)
{
	uint dac_value = 0x01;
	uint8_t mask = 0x01;
	assert(pmic);

	uint8_t current_value;
	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE, &current_value);
	uint8_t regValue = (dac_value & mask) | (current_value & ~mask);

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE,  regValue);
}

/* use the i2c interface to power down the PMIC */
static int max17135_hv_disable(pl_pmic_t *pmic)
{
	uint dac_value = 0x00;
	uint8_t mask = 0x01;
	assert(pmic);

	uint8_t current_value;
	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE, &current_value);
	uint8_t regValue = (dac_value & mask) | (current_value & ~mask);

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE,  regValue);
}

static int max17135_vcom_enable(pl_pmic_t *pmic)
{
	uint dac_value = 0x02;
	uint8_t mask = 0x02;
	assert(pmic);

	uint8_t current_value;
	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE, &current_value);
	uint8_t regValue = (dac_value & mask) | (current_value & ~mask);

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE,  regValue);
}

/* use the i2c interface to power down the PMIC */
static int max17135_vcom_disable(pl_pmic_t *pmic)
{
	uint dac_value = 0x00;
	uint8_t mask = 0x02;
	assert(pmic);

	uint8_t current_value;
	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE, &current_value);
	uint8_t regValue = (dac_value & mask) | (current_value & ~mask);

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_ENABLE,  regValue);
}

/* enable temperature sensing */
static int max17135_temp_enable(pl_pmic_t *pmic)
{
	union max17135_temp_config config;

	config.byte = 0;
	config.shutdown = 0;

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_CONF,
				  config.byte);
}

/* disable temperature sensing */
static int max17135_temp_disable(pl_pmic_t *pmic)
{
	union max17135_temp_config config;

	config.byte = 0;
	config.shutdown = 1;

	return pl_i2c_reg_write_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_CONF,
				  config.byte);
}

/* read the temperature from the PMIC */
static int max17135_temperature_measure(pl_pmic_t *pmic, int16_t *measured)
{
	union max17135_temp_status status;
	union max17135_temp_value temp;
	int stat;

	if (pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_TEMP_STAT,
			      &status.byte))
		goto error;

	if (pl_i2c_reg_read_16be(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_EXT_TEMP,
				 &temp.word))
		goto error;

	if (status.byte & (HVPMIC_TEMP_OPEN | HVPMIC_TEMP_SHORT)) {
		LOG("Temperature sensor error: 0x%02x", status.byte);
	}

	if (temp.word == HVPMIC_TEMP_INVALID) {
		*measured = HVPMIC_TEMP_DEFAULT;
		stat = -1;
	} else {
		*measured = (temp.measured >> 1);
		stat = 0;
	}

	LOG("Temperature: %d", *measured);

error:
	return stat;
}
