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
 * s1d13524_controller.c
 *
 *  Created on: 25.03.2015
 *      Author: sebastian.friebe
 */

#include <epson/s1d135xx_controller.h>
#include <epson/s1d13524.h>
#define LOG_TAG "s1d13524_controller"
#include <pl/utils.h>
#include <pl/assert.h>

#define VERBOSE 1

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

static int configure_update(pl_generic_controller_t *p, int wfid, enum pl_update_mode mode, const struct pl_area *area);
static int trigger_update(pl_generic_controller_t *p);
static int clear_update(pl_generic_controller_t *p);
static int wait_update_end(pl_generic_controller_t *p);
static int read_register(pl_generic_controller_t *p, const regSetting_t* setting);
static int write_register(pl_generic_controller_t *p, const regSetting_t setting, const uint32_t bitmask);
static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting);

static int load_buffer(pl_generic_controller_t *p, const char *buffer, const struct pl_area *area);
static int load_png_image(pl_generic_controller_t *p, const char *path, const struct pl_area *area);
static int init_controller(pl_generic_controller_t *p, int use_wf_from_nvm);
static int fill(pl_generic_controller_t *p, const struct pl_area *area, uint8_t grey);
static int load_wflib(pl_generic_controller_t *p,  const char *filename);

static int update_temp(pl_generic_controller_t *p);
static int get_temp(pl_generic_controller_t *p, int* temperature);
static int set_temp_mode(pl_generic_controller_t *p, enum pl_epdc_temp_mode mode);
static int set_power_state(pl_generic_controller_t *p, enum pl_epdc_power_state state);
static int get_resolution(pl_generic_controller_t *p, int* xres, int* yres);
// -----------------------------------------------------------------------------
// initialisation
// ------------------------------
int s1d13524_controller_setup(pl_generic_controller_t *p, s1d135xx_t *s1d135xx){
	assert(p != NULL);
	assert(s1d135xx != NULL);

//	if (s1d135xx->pins->hrdy != PL_GPIO_NONE)
//		LOG("Using HRDY GPIO");
//
//	if (s1d135xx->pins->hdc != PL_GPIO_NONE)
//		LOG("Using HDC GPIO");

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
	p->load_buffer = load_buffer;
	p->set_power_state = set_power_state;
	p->set_temp_mode = set_temp_mode;
	p->update_temp = update_temp;
	p->get_temp = get_temp;
	p->init = init_controller;
	p->wf_table = wf_table;
	p->hw_ref = s1d135xx;
	p->get_resolution = get_resolution;

	s1d135xx->flags.needs_update = 0;
	s1d135xx->hrdy_mask = S1D13524_STATUS_HRDY;
	s1d135xx->hrdy_result = 0;
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
		x = s1d135xx->read_reg(s1d135xx, S1D13524_REG_LINE_DATA_LENGTH);
		y = s1d135xx->read_reg(s1d135xx, S1D13524_REG_FRAME_DATA_LENGTH);
		*xres = x;
		*yres = y;
		return 0;
	}
	return -EINVAL;
}

static int wait_update_end(pl_generic_controller_t *p)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->wait_update_end(s1d135xx);
}

static int load_wflib(pl_generic_controller_t *p,  const char *filename)
{

	int stat = 0;
	s1d135xx_t *s1d135xx = p->hw_ref;

	uint16_t addr16[2];
	uint32_t addr32;
	uint16_t busy;

	assert(s1d135xx != NULL);

	addr16[0] = s1d135xx->read_reg(s1d135xx, S1D13541_REG_WF_ADDR_0);
	addr16[1] = s1d135xx->read_reg(s1d135xx, S1D13541_REG_WF_ADDR_1);
	addr32 = addr16[1];
	addr32 <<= 16;
	addr32 |= addr16[0];
/*
	addr16[0] = 0x108A;
	addr16[1] = 0;
*/
	s1d135xx->write_reg(s1d135xx, 0x0260, 0x8001);// 0x8001
//*
	stat = s1d135xx->load_wflib(s1d135xx, filename, addr32);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;
//*/
	stat = s1d135xx->wait_for_idle(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	s1d135xx->send_cmd_with_params(s1d135xx, S1D13524_CMD_RD_WF_INFO, addr16, ARRAY_SIZE(addr16));

	stat = s1d135xx->wait_for_idle(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	//LOG("Testing Busy");

	busy = s1d135xx->read_reg(s1d135xx, S1D135XX_REG_DISPLAY_BUSY);

	//LOG("Busy: 0x%x", busy);

	if (busy & S1D13541_WF_CHECKSUM_ERROR) {
		LOG("Waveform checksum error");
		return -EEPDC;
	}

	p->waveform_file_path = (char*) filename;

	return 0;
}

static int fill(pl_generic_controller_t *p, const struct pl_area *area, uint8_t grey)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->fill(s1d135xx, S1D13524_LD_IMG_4BPP, 4, area, grey);
}

static int load_png_image(pl_generic_controller_t *p, const char *path,
			       const struct pl_area *area)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);
	s1d135xx->cfa_overlay = p->cfa_overlay;
	s1d135xx->display_scrambling = p->display_scrambling;
	s1d135xx->xoffset = p->xoffset;
	s1d135xx->yoffset = p->yoffset;
	s1d135xx->xres = s1d135xx->read_reg(s1d135xx, S1D13524_REG_LINE_DATA_LENGTH);
	s1d135xx->yres = s1d135xx->read_reg(s1d135xx, S1D13524_REG_FRAME_DATA_LENGTH);

	return s1d135xx->load_png_image(s1d135xx, path, S1D13524_LD_IMG_4BPP, 4, (struct pl_area *) area);
}
static int load_buffer(pl_generic_controller_t *p, const char *buffer, const struct pl_area *area)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	s1d135xx->cfa_overlay = p->cfa_overlay;
	s1d135xx->display_scrambling = p->display_scrambling;
	s1d135xx->xres = s1d135xx->read_reg(s1d135xx, S1D13524_REG_LINE_DATA_LENGTH);
	s1d135xx->yres = s1d135xx->read_reg(s1d135xx, S1D13524_REG_FRAME_DATA_LENGTH);
	return s1d135xx->load_buffer(s1d135xx, buffer, S1D13524_LD_IMG_4BPP, 4, area);
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

	int stat = -1;

	s1d135xx->update_reg(s1d135xx, setting.addr, *(setting.val), bitmask);
	if ((*(setting.val) & bitmask) == (s1d135xx->read_reg(s1d135xx, setting.addr) & bitmask)) {
		stat = 0;	// success
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

static int configure_update(pl_generic_controller_t *p, int wfid, enum pl_update_mode mode, const struct pl_area *area){

	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->configure_update(s1d135xx, wfid, mode, area);
}

static int trigger_update(pl_generic_controller_t *p){
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	return s1d135xx->execute_update(s1d135xx);
}

static int clear_update(pl_generic_controller_t *p){
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat = 0;
	assert(s1d135xx != NULL);

	stat = p->configure_update(p, 0, PL_FULL_UPDATE, NULL);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	stat = p->trigger_update(p);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	return 0;
}

static int init_controller(pl_generic_controller_t *p, int use_wf_from_nvm){
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat = 0;
	assert(s1d135xx != NULL);

	stat = s1d135xx->init(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat){
		LOG("Failed to init epd controller");
		return stat;
	}

	stat = s1d135xx->load_init_code(s1d135xx, p->instruction_code_path);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat){
		LOG("Failed to load init code");
		return stat;
	}

	// check whether power save status bits are in "run mode"
	if(!(s1d135xx->read_reg(s1d135xx, 0x000a) & 0x0400))
	{
		printf("Error: System Status Register after INIT_SYS_RUN: %x\n", s1d135xx->read_reg(s1d135xx, 0x000a));
		return -EEPDC;
	}

	s1d135xx->set_registers(s1d135xx, p->regDefaults, p->regDefaultsCount);

	// Loading the init code turns the EPD power on as a side effect...
	stat = s1d135xx->set_epd_power(s1d135xx, 0);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	stat = set_power_state(p, PL_EPDC_RUN);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	stat = s1d135xx->init_gate_drv(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	stat = s1d135xx->wait_dspe_trig(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	stat = s1d13524_init_ctlr_mode(s1d135xx);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	p->xres = s1d135xx->read_reg(s1d135xx, S1D13524_REG_LINE_DATA_LENGTH);
	p->yres = s1d135xx->read_reg(s1d135xx, S1D13524_REG_FRAME_DATA_LENGTH);
	s1d135xx->xres = p->xres;
	s1d135xx->yres = p->yres;

	stat = p->set_temp_mode(p, p->temp_mode);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	LOG("Ready %dx%d", p->xres, p->yres);

	return 0;
}


static int set_temp_mode(pl_generic_controller_t *p, enum pl_epdc_temp_mode mode)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	int stat = 0;

	switch (mode) {
	case PL_EPDC_TEMP_MANUAL:
		s1d135xx->write_reg(s1d135xx, S1D13524_REG_TEMP_AUTO_RETRIEVE,
				   S1D13524_AUTO_RETRIEVE_OFF);
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		s1d135xx->write_reg(s1d135xx, S1D13524_REG_TEMP_AUTO_RETRIEVE,
				   S1D13524_AUTO_RETRIEVE_ON);
		break;
	case PL_EPDC_TEMP_INTERNAL:
		LOG("Unsupported temperature mode");
		stat = -EINVAL;
		break;
	default:
		assert_fail("Invalid temperature mode");
	}

	p->temp_mode = mode;

	return stat;
}

static int set_power_state(pl_generic_controller_t *p, enum pl_epdc_power_state state)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	int stat = 0;
	assert(s1d135xx != NULL);

	stat = s1d135xx->set_power_state(s1d135xx, state);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat)
		return stat;

	p->power_state = state;

	return 0;
}

static int update_temp(pl_generic_controller_t *p)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	int stat = 0;
	int new_temp;


	switch (p->temp_mode) {
	case PL_EPDC_TEMP_MANUAL:
		s1d135xx->write_reg(s1d135xx, S1D13524_REG_TEMP, p->manual_temp);
		new_temp = p->manual_temp;
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		new_temp = s1d135xx->read_reg(s1d135xx, S1D13524_REG_TEMP);
		break;
	case PL_EPDC_TEMP_INTERNAL:
		stat = -EINVAL;
		break;
	}

	if (stat)
		return stat;

#if VERBOSE_TEMPERATURE
	if (new_temp != p->measured_temp)
		LOG("Temperature: %d", new_temp);
#endif

	s1d135xx->measured_temp = new_temp;
	return 0;
}

static int get_temp(pl_generic_controller_t *p, int* temperature)
{
	s1d135xx_t *s1d135xx = p->hw_ref;
	assert(s1d135xx != NULL);

	int temp;

	temp = s1d135xx->read_reg(s1d135xx, S1D13524_REG_TEMP);
	*temperature = temp;
	return 0;
}

