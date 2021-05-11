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
 * s1d13541_epdc.c
 *
 *  Created on: 30.01.2015
 *      Author: Bastus
 */

#include <epson/s1d135xx_controller.h>
#include <epson/s1d13541.h>
#define LOG_TAG "s1d13541_controller"
#include <pl/utils.h>
#include <pl/assert.h>
#include <pl/gpio.h>
#include <src/pindef.h>

#define VERBOSE 0

static const struct pl_wfid wf_table[] = {
	{ "default",	   2 },
	{ "0",             0 },
	{ "1",             1 },
	{ "2",             2 },
	{ "3",             3 },
	{ "4",             4 },
	{ "5",             5 },
	{ "6",             6 },
	{ "7",             7 },
	{ "8",             8 },
	{ "9",             9 },
	{ "10",           10 },
	{ "11",           11 },
	{ "12",           12 },
	{ "13",           13 },
	{ "14",           14 },
	{ "15",           15 },
	{ NULL,           -1 }
};

const char *standard_waveform_filename = "/tmp/S1D13541_current_waveform.bin";
const char *standard_acvcom_filename = "/tmp/BBACVCom.config";

static int configure_update(pl_generic_controller_t *p, int wfid, enum pl_update_mode mode, const struct pl_area *area);
static int trigger_update(pl_generic_controller_t *p);
static int clear_update(pl_generic_controller_t *p);
static int wait_update_end(pl_generic_controller_t *p);
static int read_register(pl_generic_controller_t *p, const regSetting_t* setting);
static int write_register(pl_generic_controller_t *p, const regSetting_t setting, const uint32_t bitmask);
static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting);

static int load_wflib(pl_generic_controller_t *p, const char *filename);
static int set_temp_mode(pl_generic_controller_t *p, enum pl_epdc_temp_mode mode);
static int s1d13541_update_temp(pl_generic_controller_t *p);
static int fill(pl_generic_controller_t *p, const struct pl_area *area, uint8_t grey);

static int load_png_image(pl_generic_controller_t *p, const char *path,  const struct pl_area *area, int top, int left);
static int init_controller(pl_generic_controller_t *p, int use_wf_from_nvm);
static int s1d13541_update_temp(pl_generic_controller_t *p);

static void update_temp(struct s1d135xx *p, uint16_t reg);
static int update_temp_manual(struct s1d135xx *p, int manual_temp);
static int update_temp_auto(struct s1d135xx *p, uint16_t temp_reg);

static int set_power(pl_generic_controller_t *p, enum pl_epdc_power_state state);
static int get_resolution(pl_generic_controller_t *p, int* xres, int* yres);

// -----------------------------------------------------------------------------
// initialization
// ------------------------------
int s1d13541_controller_setup(pl_generic_controller_t *p, struct s1d135xx *s1d135xx){

	assert(p != NULL);
	assert(s1d135xx != NULL);

	if (s1d135xx->pins->hrdy != PL_GPIO_NONE)
		LOG("Using HRDY GPIO");

	if (s1d135xx->pins->hdc != PL_GPIO_NONE)
		LOG("Using HDC GPIO");


	p->configure_update = configure_update;
	p->trigger_update = trigger_update;
	p->clear_update = clear_update;
	p->read_register = read_register;
	p->write_register = write_register;
	p->send_cmd = send_cmd;

	p->wait_update_end = wait_update_end;
	p->load_wflib = load_wflib;
	p->fill = fill;
	p->load_image = load_png_image;
	p->set_power_state = set_power;
	p->set_temp_mode = set_temp_mode;
	p->update_temp = s1d13541_update_temp;

	p->init = init_controller;
	p->wf_table = wf_table;
	p->hw_ref = s1d135xx;
	p->get_resolution = get_resolution;

	s1d135xx->flags.needs_update = 0;
	s1d135xx->hrdy_mask = S1D13541_STATUS_HRDY;
	s1d135xx->hrdy_result = S1D13541_STATUS_HRDY;
	s1d135xx->measured_temp = -127;
	p->manual_temp = 23;

	return 0;

}


// -----------------------------------------------------------------------------
// private controller interface functions
// ------------------------------
static int get_resolution(pl_generic_controller_t *p, int* xres, int* yres){
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);
	if(xres && yres){
		// TODO: Check if scrambled!!!
		int x,y;
		x = s1d135xx->read_reg(s1d135xx, S1D13541_REG_LINE_DATA_LENGTH);
		y = s1d135xx->read_reg(s1d135xx, S1D13541_REG_FRAME_DATA_LENGTH);
		*xres = x;
		*yres = y;
		return 0;
	}
	return -EINVAL;
}

static int configure_update(pl_generic_controller_t *p, int wfid, enum pl_update_mode mode, const struct pl_area *area)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->configure_update(s1d135xx, wfid, mode, area);
}

static int trigger_update(pl_generic_controller_t *p)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->execute_update(s1d135xx);
}

static int clear_update(pl_generic_controller_t *p){
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat;
	assert(s1d135xx != NULL);

	stat = p->configure_update(p, 0, PL_FULL_UPDATE, NULL);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	stat = p->trigger_update(p);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	return 0;
}


static int wait_update_end(pl_generic_controller_t *p)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	if(s1d135xx->wait_update_end(s1d135xx))
		if(s1d135xx->wait_update_end(s1d135xx))
			if(s1d135xx->wait_update_end(s1d135xx))
				return s1d135xx->wait_update_end(s1d135xx);

	return 0;
}

static int read_register(pl_generic_controller_t *p, const regSetting_t* setting)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	*(setting->val) = s1d135xx->read_reg(s1d135xx, setting->addr);

	return 0;
}

static int write_register(pl_generic_controller_t *p, const regSetting_t setting, const uint32_t bitmask)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	int stat = -EINVAL;

	s1d135xx->update_reg(s1d135xx, setting.addr, *(setting.val), bitmask);
	if ((*(setting.val) & bitmask) == (s1d135xx->read_reg(s1d135xx, setting.addr) & bitmask)) {
		stat = 0;	// success
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	}

	return stat;
}

static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	uint16_t cmd = setting.addr & 0xFF;
	uint16_t *val = setting.val;
	int valCount = setting.valCount;

	s1d135xx->send_cmd_with_params(s1d135xx, cmd, val, valCount);

	return 0;
}

/* -- pl_epdc interface -- */

/**
 * Loads the given waveform library to the S1D13541 controller
 *
 * Opens the given waveform binary file and sends it's content to the EPSON controller.
 * In case this succeeds, it will update a fixed symlink location (standard_waveform_filename)
 * with the filename given to the function.
 * Updates to internally stored waveform file path (waveform_file_path)
 *
 * @param p pointer to a controller object
 * @param filename the path to the waveform file
 * @return success indication; PASS == 0, FAIL != 0
 *
 * @see standard_waveform_filename
 * @see waveform_file_path
 */
static int load_wflib(pl_generic_controller_t *p, const char *filename)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat = 0;
	assert(s1d135xx != NULL);

	stat = s1d135xx->load_wflib(s1d135xx, filename, S1D13541_WF_ADDR);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	p->waveform_file_path = (char*) filename;

	char absolute_filename[300];
	realpath(filename, absolute_filename);

	// if file is not the standard symlink,
	// then update the standard symlink with this file
	if (strcmp(absolute_filename, standard_waveform_filename) != 0){
		char command[1000];
		sprintf(command, "ln -f -s %s %s", absolute_filename, standard_waveform_filename);
		if (system(command)){
			LOG("failed to set symlink for given waveform file...");
			return -ENOENT;
		}
	}

	char * pos = strrchr(absolute_filename, '.');
	*pos = '\0';
	strcat(absolute_filename, "_BBACVCom.config");

	// if acvcom file is not the standard symlink,
	// then update the standard symlink with this file
	if (strcmp(absolute_filename, standard_acvcom_filename) != 0){
		char command[1000];
		sprintf(command, "ln -f -s %s %s", absolute_filename, standard_acvcom_filename);
		if (system(command)){
			LOG("failed to set symlink for given acvcom file...");
			return -ENOENT;
		}
	}

	return 0;
}

static int set_temp_mode(pl_generic_controller_t *p, enum pl_epdc_temp_mode mode)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);
	uint16_t reg;

	if (mode == p->temp_mode)
		return 0;

	reg = s1d135xx->read_reg(s1d135xx, S1D135XX_REG_PERIPH_CONFIG);
	// ToDo: when do we set this bit back?
	reg &= S1D13541_TEMP_SENSOR_CONTROL;

	switch (mode) {
	case PL_EPDC_TEMP_MANUAL:
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		reg &= ~S1D13541_TEMP_SENSOR_EXTERNAL;
		break;
	case PL_EPDC_TEMP_INTERNAL:
		reg |= S1D13541_TEMP_SENSOR_EXTERNAL;
		break;
	default:
		assert_fail("Invalid temperature mode");
	}

	s1d135xx->write_reg(s1d135xx, S1D135XX_REG_PERIPH_CONFIG, reg);

	// Configure the controller to automatically update the waveform table
	// after each temperature measurement.
	reg = s1d135xx->read_reg(s1d135xx, S1D13541_REG_WF_DECODER_BYPASS);
	reg |= S1D13541_AUTO_TEMP_JUDGE_EN;
	s1d135xx->write_reg(s1d135xx, reg, S1D13541_REG_WF_DECODER_BYPASS);

	p->temp_mode = mode;

	return 0;
}

static int s1d13541_update_temp(pl_generic_controller_t *p)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);
	int stat;

	switch (p->temp_mode) {
	case PL_EPDC_TEMP_MANUAL:
		stat = update_temp_manual(s1d135xx, p->manual_temp);
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		stat = update_temp_auto(s1d135xx, S1D135XX_REG_I2C_TEMP_SENSOR_VALUE);
		break;
	case PL_EPDC_TEMP_INTERNAL:
		stat = update_temp_auto(s1d135xx, S1D13541_REG_TEMP_SENSOR_VALUE);
		break;
	}

	if(stat < 0)
		return stat;

	if (s1d135xx->flags.needs_update) {
#if VERBOSE_TEMPERATURE
		LOG("Updating waveform table");
#endif
		stat = p->load_wflib(p, standard_waveform_filename);
		if(stat < 0)
			return stat;
	}

	return 0;
}

static int fill(pl_generic_controller_t *p, const struct pl_area *area, uint8_t grey)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->fill(s1d135xx, S1D13541_LD_IMG_4BPP, 4, area, grey);
}

static int load_png_image(pl_generic_controller_t *p, const char *path,  const struct pl_area *area, int top, int left)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);
	s1d135xx->cfa_overlay = p->cfa_overlay;
	s1d135xx->mediaType = p->mediaType;
	s1d135xx->display_scrambling = p->display_scrambling;
	s1d135xx->xoffset = p->xoffset;
	s1d135xx->yoffset = p->yoffset;
	s1d135xx->xres = s1d135xx->read_reg(s1d135xx, S1D13541_REG_LINE_DATA_LENGTH);
	s1d135xx->yres = s1d135xx->read_reg(s1d135xx, S1D13541_REG_FRAME_DATA_LENGTH);

	return s1d135xx->load_png_image(s1d135xx, path, S1D13541_LD_IMG_4BPP, 4, (struct pl_area *) area, top, left);
}
/* -- initialisation -- */

static int init_controller(pl_generic_controller_t *p, int use_wf_from_nvm){
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat = 0;
	assert(s1d135xx != NULL);

	// enable external 32MHz clock for s1d13524
	//pl_gpio_set(s1d135xx->gpio, CHIFFCHAFF_32MHZ_EN, 1);

	stat = s1d135xx->init(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	stat =s1d135xx->load_init_code(s1d135xx, p->instruction_code_path);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0){
		LOG("Failed to load init code");

			return stat;
	}

	s1d135xx->set_registers(s1d135xx, p->regDefaults, p->regDefaultsCount);

	s1d135xx->write_reg(s1d135xx, S1D13541_REG_PROT_KEY_1, S1D13541_PROT_KEY_1);
	s1d135xx->write_reg(s1d135xx, S1D13541_REG_PROT_KEY_2, S1D13541_PROT_KEY_2);

	stat = s1d135xx->wait_for_idle(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	stat = set_power(p, PL_EPDC_RUN);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	stat = s1d135xx->init_gate_drv(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	stat = s1d135xx->wait_dspe_trig(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	p->xres = s1d135xx->read_reg(s1d135xx, S1D13541_REG_LINE_DATA_LENGTH);
	p->yres = s1d135xx->read_reg(s1d135xx, S1D13541_REG_FRAME_DATA_LENGTH);
	s1d135xx->xres = p->xres;
	s1d135xx->yres = p->yres;

	stat = p->set_temp_mode(p, p->temp_mode);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	LOG("Ready %dx%d", p->xres, p->yres);

	return 0;
}

static int set_power(pl_generic_controller_t *p, enum pl_epdc_power_state state)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat = 0;
	assert(s1d135xx != NULL);

	stat = s1d135xx->set_power_state(s1d135xx, state);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	p->power_state = state;

	return 0;
}

// -----------------------------------------------------------------------------
// private functions
// ------------------------------
static void update_temp(struct s1d135xx *p, uint16_t reg)
{
	uint16_t regval;

	regval = p->read_reg(p, S1D135XX_REG_INT_RAW_STAT);
	p->flags.needs_update = (regval & S1D13541_INT_RAW_WF_UPDATE) ? 1 : 0;
	p->write_reg(p, S1D135XX_REG_INT_RAW_STAT,
			   (S1D13541_INT_RAW_WF_UPDATE |
			    S1D13541_INT_RAW_OUT_OF_RANGE));
	regval = p->read_reg(p, reg) & S1D135XX_TEMP_MASK;

#if VERBOSE_TEMPERATURE
	if (regval != p->measured_temp)
		LOG("Temperature: %d", regval);
#endif

	p->measured_temp = regval;
}
#if 0
static void get_temp(struct s1d135xx *p, int* temperature)
{

	uint16_t regval;

	regval = p->read_reg(p, S1D135XX_REG_INT_RAW_STAT);
	p->flags.needs_update = (regval & S1D13541_INT_RAW_WF_UPDATE) ? 1 : 0;
	p->write_reg(p, S1D135XX_REG_INT_RAW_STAT,
			   (S1D13541_INT_RAW_WF_UPDATE |
			    S1D13541_INT_RAW_OUT_OF_RANGE));
	regval = p->read_reg(p, reg) & S1D135XX_TEMP_MASK;

#if VERBOSE_TEMPERATURE
	if (regval != p->measured_temp)
		LOG("Temperature: %d", regval);
#endif

	p->measured_temp = regval;

}
#endif

static int update_temp_manual(struct s1d135xx *p, int manual_temp)
{
	uint16_t regval;

	regval = (S1D13541_GENERIC_TEMP_EN |
		  S1D13541_GENERIC_TEMP_JUDGE_EN |
		  (manual_temp & S1D13541_GENERIC_TEMP_MASK));
	p->write_reg(p, S1D13541_REG_GENERIC_TEMP_CONFIG, regval);

	if (p->wait_for_idle(p))
		return -ETIME;

	update_temp(p, S1D13541_REG_GENERIC_TEMP_CONFIG);

	return 0;
}

static int update_temp_auto(struct s1d135xx *p, uint16_t temp_reg)
{
	int stat =p->set_power_state(p, PL_EPDC_STANDBY);
	if(stat < 0)
		return stat;

	p->send_cmd_with_params(p, S1D13541_CMD_RD_TEMP, NULL, 0);

	if (p->wait_for_idle(p))
		return -ETIME;

	stat = p->set_power_state(p, PL_EPDC_RUN);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if(stat < 0)
		return stat;

	update_temp(p, temp_reg);

	return 0;
}

