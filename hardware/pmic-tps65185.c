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
 * pmic-tps65185.c -- Driver for TI TPS65185 PMIC
 *
 */

#include <pl/i2c.h>
#include <unistd.h>
#include <stdlib.h>
#include "pl/assert.h"
#include "pl/vcom.h"
#include "pmic-tps65185.h"

#define LOG_TAG "tps65185"
#include "pl/utils.h"

/* Set to 1 to enable verbose log messages */
#define VERBOSE 0
#define EPMIC 199
/* Set to 1 to dump registers */
#define DO_REG_DUMP 0

#define HVPMIC_DAC_MAX          ((1 << 9)-1)
#define HVPMIC_DAC_MIN          0
#define HVPMIC_TEMP_DEFAULT     20
#define HVPMIC_VERSION          0x65

#if 0
#define MV_DIV	33		// Each DAC step is 33mV
#endif

enum tps65185_register {
	HVPMIC_REG_TMST_VALUE = 0x00,
	HVPMIC_REG_ENABLE     = 0x01,
	HVPMIC_REG_VADJ       = 0x02,
	HVPMIC_REG_VCOM1      = 0x03,
	HVPMIC_REG_VCOM2      = 0x04,
	HVPMIC_REG_INT_EN1    = 0x05,
	HVPMIC_REG_INT_EN2    = 0x06,
	HVPMIC_REG_INT1       = 0x07,
	HVPMIC_REG_INT2       = 0x08,
	HVPMIC_REG_UPSEQ0     = 0x09,
	HVPMIC_REG_UPSEQ1     = 0x0A,
	HVPMIC_REG_DWNSEQ0    = 0x0B,
	HVPMIC_REG_DWNSEQ1    = 0x0C,
	HVPMIC_REG_TMST1      = 0x0D,
	HVPMIC_REG_TMST2      = 0x0E,
	HVPMIC_REG_PG_STAT    = 0x0F,
	HVPMIC_REG_REV_ID     = 0x10,
	HVPMIC_REG_MAX
};

union tps65185_version {
	struct {
		char version:4;
		char minor:2;
		char major:2;
	} v;
	uint8_t byte;
};

struct pmic_data {
	uint8_t reg;
	uint8_t data;
};

static const struct pmic_data init_data[] = {
	{ HVPMIC_REG_ENABLE,     0x00 },
	{ HVPMIC_REG_VADJ,       0x03 },
	{ HVPMIC_REG_VCOM1,      0x00 },
	{ HVPMIC_REG_VCOM2,      0x00 },
	{ HVPMIC_REG_INT_EN1,    0x00 },
	{ HVPMIC_REG_INT_EN2,    0x00 },
	{ HVPMIC_REG_UPSEQ0,     0x78 },
	{ HVPMIC_REG_UPSEQ1,     0x00 },
	{ HVPMIC_REG_DWNSEQ0,    0x00 },
	{ HVPMIC_REG_DWNSEQ1,    0x00 },
	{ HVPMIC_REG_TMST1,      0x00 },
	{ HVPMIC_REG_TMST2,      0x78 }
};

static int tps65185_init(pl_pmic_t *p);
static void tps65185_delete(pl_pmic_t *p);
static int tps65185_check_revision(pl_pmic_t *p);
static int tps65185_configure(pl_pmic_t *pmic, struct vcom_cal *cal);
static int tps65185_wait_pok(pl_pmic_t *p);
static int tps65185_hv_enable(pl_pmic_t *p);
static int tps65185_hv_disable(pl_pmic_t *p);
static int tps65185_set_vcom_voltage(pl_pmic_t *p, int mv);
static int tps65185_get_vcom_voltage(pl_pmic_t *p);
static int tps65185_set_vcom_register(pl_pmic_t *p, int value);
static int tps65185_temperature_measure(pl_pmic_t *p, int16_t *measured);
static int tps65185_vcom_enable(pl_pmic_t *p);
static int tps65185_vcom_disable(pl_pmic_t *p);
static int tps65185_apply_timings(pl_pmic_t *p);
static int tps65185_temp_disable(pl_pmic_t *p);
static int tps65185_temp_enable(pl_pmic_t *p);

#if DO_REG_DUMP
/* Note: reading some registers will modify the status of the device */
static void reg_dump(struct tps65185_info *p)
{
	uint8_t data;
	uint8_t reg;

	for (reg = HVPMIC_REG_TMST_VALUE; reg < HVPMIC_REG_MAX; reg++) {
		if (!pl_i2c_reg_read_8(p->i2c, p->i2c_addr, reg, &data))
			LOG("reg[0x%02X] = 0x%02X", reg, data);
	}
}
#endif

pl_pmic_t *tps65185_new(struct pl_i2c *i2c, uint8_t i2c_addr){
	pl_pmic_t *p = (pl_pmic_t *)malloc(sizeof(pl_pmic_t));

	assert(i2c);
	assert(p);

	p->i2c_addr = i2c_addr;
	p->i2c = i2c;
	p->cal = NULL;

	p->delete = tps65185_delete;
	p->init = tps65185_init;

	p->check_revision_code = tps65185_check_revision;
	p->configure = tps65185_configure;
	p->hv_disable = tps65185_hv_disable;
	p->hv_enable = tps65185_hv_enable;
	p->vcom_disable = tps65185_vcom_disable;
	p->vcom_enable = tps65185_vcom_enable;
	p->apply_timings = tps65185_apply_timings;
	p->set_vcom_register = tps65185_set_vcom_register;
	p->set_vcom_voltage = tps65185_set_vcom_voltage;
	p->get_vcom_voltage = tps65185_get_vcom_voltage;
	p->temp_disable = tps65185_temp_disable;
	p->temp_enable = tps65185_temp_enable;
	p->temperature_measure = tps65185_temperature_measure;
	p->wait_pok = tps65185_wait_pok;

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
static void tps65185_delete(pl_pmic_t *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

static int tps65185_init(pl_pmic_t *p)
{
	int i;
	int stat = 0;

	if (p->is_initialized == false){
		stat = p->check_revision_code(p);
		p->is_initialized = true;


		// write initial data
		for (i = 0; i < ARRAY_SIZE(init_data); i++) {
			if (pl_i2c_reg_write_8(p->i2c, p->i2c_addr, init_data[i].reg,
					init_data[i].data))
				return -EINVAL;
		}
	}

	// enable 3V3 Output
	pl_i2c_reg_write_8(p->i2c, p->i2c_addr,HVPMIC_REG_ENABLE, 0x20);

	return stat;
}

static int tps65185_check_revision(pl_pmic_t *p)
{
	union tps65185_version ver;
	assert(p);

	if (pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_REV_ID, &ver.byte))
		return -EPMIC;

	LOG("Version: %d.%d.%d", ver.v.major, ver.v.minor, ver.v.version);

	if (ver.byte == 0x00 || ver.byte == 0xff) {
		LOG("Wrong version: 0x%02X",
		    ver.byte, HVPMIC_VERSION);
		return -EPMIC;
	}

	return 0;
}

static int tps65185_configure(pl_pmic_t *pmic, struct vcom_cal *cal)
{
	assert(pmic);

	/* cal may be null if not being used */
	pmic->cal = cal;

	return 0;
}

static int tps65185_wait_pok(pl_pmic_t *p)
{
	uint8_t pgstat;
	uint8_t int1, int2;

	assert(p != NULL);

	if (pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_PG_STAT,
			   &pgstat))
		return -EPMIC;

	if (pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_INT1, &int1) ||
	    pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_INT2, &int2))
		return -EPMIC;

#if VERVBOSE
	if (int1 || int2)
		LOG("PGSTAT: 0x%02X, INT1: 0x%02X, INT2: 0x%02X",
		       pgstat, int1, int2);
#endif

#if DO_REG_DUMP
	reg_dump(pmic);
#endif

	//abort_msg("TPS65185 POK feature not tested", ABORT_UNDEFINED);

	return 0;
}

/* use the i2c interface to power up the PMIC */
static int tps65185_hv_enable(pl_pmic_t *p)
{
	//LOG("tps65185_hv_enable - not yet implemented!");
	pl_i2c_reg_write_8(p->i2c, p->i2c_addr,HVPMIC_REG_ENABLE, 0xbf);
	usleep(10000);
	return 0;
}

/* use the i2c interface to power down the PMIC */
static int tps65185_hv_disable(pl_pmic_t *p)
{
	//LOG("tps65185_hv_disable - not yet implemented!");
	usleep(100000);
	pl_i2c_reg_write_8(p->i2c, p->i2c_addr,HVPMIC_REG_ENABLE, 0x60);

	return 0;
}

/* program the internal VCOM Dac to give us the required voltage */
static int tps65185_set_vcom_register(pl_pmic_t *p, int value)
{
	const uint8_t v1 = value;
	const uint8_t v2 = (value >> 8) & 0x01;

	assert(p != NULL);

	if (pl_i2c_reg_write_8(p->i2c, p->i2c_addr, HVPMIC_REG_VCOM1, v1))
		return -EPMIC;

	if (pl_i2c_reg_write_8(p->i2c, p->i2c_addr, HVPMIC_REG_VCOM2, v2))
		return -EPMIC;

	return 0;
}

/* program the internal VCOM Dac to give us the required voltage */
static int tps65185_set_vcom_voltage(pl_pmic_t *p, int mv)
{
	int dac_value;

	uint8_t v1;
	uint8_t v2;

	assert(p != NULL);

	dac_value = vcom_calculate(p->cal, mv);
	LOG("calculate: %i, %i", mv, dac_value);

	if (dac_value < HVPMIC_DAC_MIN)
		dac_value = HVPMIC_DAC_MIN;
	else if (dac_value > HVPMIC_DAC_MAX)
		dac_value = HVPMIC_DAC_MAX;

	v1 = dac_value & 0x00FF;
	v2 = ((dac_value >> 8) & 0x0001);

	if (pl_i2c_reg_write_8(p->i2c, p->i2c_addr, HVPMIC_REG_VCOM1, v1))
	    return -EPMIC;

	return pl_i2c_reg_write_8(p->i2c, p->i2c_addr, HVPMIC_REG_VCOM2, v2);
}

/* program the internal VCOM Dac to give us the required voltage */
static int tps65185_get_vcom_voltage(pl_pmic_t *pmic)
{

	uint8_t v1 = 0;
	uint8_t v2 = 0;
	int mv;
	uint16_t dac_value;
	assert(pmic != NULL);

	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_VCOM1, &v1);
	pl_i2c_reg_read_8(pmic->i2c, pmic->i2c_addr, HVPMIC_REG_VCOM2, &v2);

	dac_value = v1 + ((uint16_t) v2 << 8);

	mv = vcom_calculate_dac(pmic->cal, dac_value);
#if VERBOSE
	LOG("calculate: %i, %i", mv, dac_value);
#endif
	return mv;
}


static int tps65185_temperature_measure(pl_pmic_t *p, int16_t *measured)
{
	int8_t temp;
	uint8_t progress;

	/* Trigger conversion */
	if (pl_i2c_reg_write_8(p->i2c, p->i2c_addr,HVPMIC_REG_TMST1, 0x80))
		return -EPMIC;

	/* wait for it to complete */
	do {
		if (pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_TMST1,
				      &progress))
			return -EPMIC;
	} while ((progress & 0x20));

	/* read the temperature */
	if (pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_TMST_VALUE,
			      (uint8_t *)&temp)) {
		temp = HVPMIC_TEMP_DEFAULT;
		LOG("Warning: using default temperature %d", temp);
	}

	*measured = temp;

#if VERBOSE
	LOG("Temperature: %d", *measured);
#endif

	return 0;
}

static int tps65185_vcom_enable(pl_pmic_t *p){

	//LOG("tps65185_vcom_enable - not yet implemented");

	uint8_t data;
	pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_ENABLE, &data);

	// enable vcom
	data |= 0x10;

	pl_i2c_reg_write_8(p->i2c, p->i2c_addr, HVPMIC_REG_ENABLE, data);
	usleep(10000);

	return 0;
}
static int tps65185_vcom_disable(pl_pmic_t *p){

	//LOG("tps65185_vcom_disable - not yet implemented");

	uint8_t data;
	pl_i2c_reg_read_8(p->i2c, p->i2c_addr, HVPMIC_REG_ENABLE, &data);

	// disable vcom
	data &= 0xef;

	pl_i2c_reg_write_8(p->i2c, p->i2c_addr, HVPMIC_REG_ENABLE, data);

	return 0;
}

static int tps65185_apply_timings(pl_pmic_t *p){
	LOG("tps65185_apply_timings - not yet implemented");
	return 0;
}
static int tps65185_temp_disable(pl_pmic_t *p){
	LOG("tps65185_temp_disable - not yet implemented");
	return 0;
}
static int tps65185_temp_enable(pl_pmic_t *p){
	LOG("tps65185_temp_enable - not yet implemented");
	return 0;
}

