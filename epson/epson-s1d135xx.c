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
 * epson-s1d135xx.c -- Common Epson S1D135xx primitives
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "s1d13524.h"
#include "s1d13541.h"

#include <pl/generic_interface.h>
#include <pl/gpio.h>
#include <pl/i2c.h>
#include <stdlib.h>
#include <unistd.h>
#include <pl/assert.h>
#include <unistd.h>

#define LOG_TAG "s1d135xx"
#include <pl/utils.h>
#include <pl/parser.h>
#include <pl/scramble.h>
#include <pl/color.h>

/* Set to 1 to enable verbose update and EPD power on/off log messages */
#define VERBOSE 0

#define DATA_BUFFER_LENGTH              0x4B000 //512 //
#define S1D135XX_WF_MODE(_wf)           (((_wf) << 8) & 0x0F00)
#define S1D135XX_XMASK                  0x0FFF
#define S1D135XX_YMASK                  0x0FFF
#define S1D135XX_INIT_CODE_CHECKSUM_OK  (1 << 15)

enum s1d135xx_cmd {
	S1D135XX_CMD_INIT_SET = 0x00, /* to load init code */
	S1D135XX_CMD_RUN = 0x02,
	S1D135XX_CMD_STBY = 0x04,
	S1D135XX_CMD_SLEEP = 0x05,
	S1D135XX_CMD_INIT_STBY = 0x06, /* init then standby */
	S1D135XX_CMD_INIT_ROT_MODE = 0x0B,
	S1D135XX_CMD_READ_REG = 0x10,
	S1D135XX_CMD_WRITE_REG = 0x11,
	S1D135XX_CMD_BST_RD_SDR = 0x1C,
	S1D135XX_CMD_BST_WR_SDR = 0x1D,
	S1D135XX_CMD_BST_END_SDR = 0x1E,
	S1D135XX_CMD_LD_IMG = 0x20,
	S1D135XX_CMD_LD_IMG_AREA = 0x22,
	S1D135XX_CMD_LD_IMG_END = 0x23,
	S1D135XX_CMD_WAIT_DSPE_TRG = 0x28,
	S1D135XX_CMD_WAIT_DSPE_FREND = 0x29,
	S1D135XX_CMD_UPD_INIT = 0x32,
	S1D135XX_CMD_UPDATE_FULL = 0x33,
	S1D135XX_CMD_UPDATE_FULL_AREA = 0x34,
	S1D135XX_CMD_UPDATE_PART = 0x35,
	S1D135XX_CMD_UPDATE_PART_AREA = 0x36,
	S1D135XX_CMD_EPD_GDRV_CLR = 0x37,
};

// interface functions
static void s1d135xx_hard_reset(struct s1d135xx* p);
static int s1d135xx_soft_reset(struct s1d135xx *p);
static int s1d135xx_wait_idle(struct s1d135xx *p);
static uint16_t s1d135xx_read_reg(struct s1d135xx *p, uint16_t reg);
static int s1d135xx_write_reg(struct s1d135xx *p, uint16_t reg, uint16_t val);
static int s1d135xx_read_spi(struct s1d135xx *p, uint8_t *val, size_t size);
static int s1d135xx_write_spi(struct s1d135xx *p, uint8_t *val, size_t size);
static int s1d135xx_update_reg(struct s1d135xx *p, uint16_t reg, uint16_t val,
		const uint32_t bitmask);
static int s1d135xx_set_registers(struct s1d135xx *p, const regSetting_t* map,
		int n);
static void s1d135xx_cmd(struct s1d135xx *p, uint16_t cmd,
		const uint16_t *params, size_t n);
static int s1d135xx_set_epd_power(struct s1d135xx *p, int on);
static int s1d135xx_set_power_state(struct s1d135xx *p,
		enum pl_epdc_power_state state);
static int s1d135xx_load_init_code(struct s1d135xx *p, const char *filename);
static int s1d135xx_wait_update_end(struct s1d135xx *p);
static int s1d135xx_configure_update(struct s1d135xx *p, int wfid,
		enum pl_update_mode mode, const struct pl_area *area);
static int s1d135xx_execute_update(struct s1d135xx *p);
static int s1d135xx_load_buffer(struct s1d135xx *p, const char *buffer,
		uint16_t mode, unsigned bpp, const struct pl_area *area);
static int s1d135xx_load_png_image(struct s1d135xx *p, const char *path,
		uint16_t mode, unsigned bpp, struct pl_area *area, int top, int left);
static int s1d135xx_pattern_check(struct s1d135xx *p, uint16_t height,
		uint16_t width, uint16_t checker_size, uint16_t mode);
static int s1d135xx_fill(struct s1d135xx *p, uint16_t mode, unsigned bpp,
		const struct pl_area *a, uint8_t grey);
static int s1d135xx_wait_dspe_trig(struct s1d135xx *p);
static int s1d135xx_init_gate_drv(struct s1d135xx *p);
static int s1d135xx_load_wflib(struct s1d135xx *p, const char *filename,
		uint32_t addr);
// controller specific interface functions
static int s1d13541_check_prod_code(struct s1d135xx *p, uint16_t ref_code);
static int s1d13524_check_prod_code(struct s1d135xx *p, uint16_t ref_code);
static int s1d13541_clear_init(struct s1d135xx *p);
static int s1d13524_clear_init(struct s1d135xx *p);
static int s1d13541_init_controller(struct s1d135xx *p);
static int s1d13524_init_controller(struct s1d135xx *p);
static int s1d13541_init_clocks(struct s1d135xx *p);
static int s1d13524_init_clocks(struct s1d135xx *p);

// private functions
//static void memory_padding(uint8_t *source, uint8_t *target, int s_gl, int s_sl, int t_gl, int t_sl);
static void memory_padding(uint8_t *source, uint8_t *target,
		int source_gatelines, int source_sourcelines, int target_gatelines,
		int target_sourcelines, int gate_offset, int source_offset);
static void memory_padding_area(uint8_t *source, uint8_t *target,
		int source_gatelines, int source_sourcelines, int gate_offset,
		int source_offset, struct pl_area* source_area, int top, int left);
static int check_prod_code(struct s1d135xx *p, uint16_t ref_code);
static int get_hrdy(struct s1d135xx *p);
static int wflib_wr(void *ctx, const uint8_t *data, size_t n);
static int do_fill(struct s1d135xx *p, const struct pl_area *area, unsigned bpp,
		uint8_t g);
static int transfer_file(struct pl_generic_interface *interface, FILE *file);
static void transfer_data(struct pl_generic_interface *interface,
		const uint8_t *data, size_t n);
static void send_cmd_area(struct s1d135xx *p, uint16_t cmd, uint16_t mode,
		const struct pl_area *area);
static void send_cmd_cs(struct s1d135xx *p, uint16_t cmd);
static void send_cmd(struct s1d135xx *p, uint16_t cmd);
static void send_params(struct pl_generic_interface *interface,
		const uint16_t *params, size_t n);
static void send_param(struct pl_generic_interface *interface, uint16_t param);
static void set_cs(struct s1d135xx *p, int state);
static void set_hdc(struct s1d135xx *p, int state);

/**
 * allocates, configures and returns a new s1d135xx structure based epson s1d13524 epdc
 *
 * @param gpios pl_gpio structure
 * @param spi pl_spi structure
 * @param pins s1d135xx_pins structure
 * @return pl_spi structure
 */
s1d135xx_t *s1d13524_new(struct pl_gpio *gpios,
		struct pl_generic_interface *interface, struct pl_i2c *i2c,
		const struct s1d135xx_pins *pins) {

	assert(gpios != NULL);
	assert(pins != NULL);
	assert(interface != NULL);

	s1d135xx_t *p = (s1d135xx_t *) malloc(sizeof(s1d135xx_t));

	p->gpio = gpios;
	p->interface = interface;
	p->pins = pins;
#if 1
	p->i2c = i2c;
#else
	epson_i2c_init(p, i2c);
#endif
	p->hard_reset = s1d135xx_hard_reset;
	p->soft_reset = s1d135xx_soft_reset;
	p->wait_for_idle = s1d135xx_wait_idle;
	p->read_reg = s1d135xx_read_reg;
	p->write_reg = s1d135xx_write_reg;
	p->read_spi = s1d135xx_read_spi;
	p->write_spi = s1d135xx_write_spi;
	p->update_reg = s1d135xx_update_reg;
	p->set_registers = s1d135xx_set_registers;
	p->send_cmd_with_params = s1d135xx_cmd;
	p->set_epd_power = s1d135xx_set_epd_power;
	p->set_power_state = s1d135xx_set_power_state;
	p->load_init_code = s1d135xx_load_init_code;
	p->wait_update_end = s1d135xx_wait_update_end;
	p->configure_update = s1d135xx_configure_update;
	p->execute_update = s1d135xx_execute_update;
	p->load_buffer = s1d135xx_load_buffer;
	p->load_png_image = s1d135xx_load_png_image;
	p->pattern_check = s1d135xx_pattern_check;
	p->fill = s1d135xx_fill;
	p->wait_dspe_trig = s1d135xx_wait_dspe_trig;
	p->init_gate_drv = s1d135xx_init_gate_drv;
	p->load_wflib = s1d135xx_load_wflib;

	// controller specific
	p->check_prod_code = s1d13524_check_prod_code;
	p->clear_init = s1d13524_clear_init;
	p->init = s1d13524_init_controller;
	p->init_clocks = s1d13524_init_clocks;
	return p;
}

/**
 * allocates, configures and returns a new s1d135xx structure based epson s1d13541 epdc
 *
 * @param gpios pl_gpio structure
 * @param spi pl_spi structure
 * @param pins s1d135xx_pins structure
 * @return pl_spi structure
 */
s1d135xx_t *s1d13541_new(struct pl_gpio *gpios,
		struct pl_generic_interface *interface, struct pl_i2c *i2c,
		const struct s1d135xx_pins *pins) {

	assert(gpios != NULL);
	assert(pins != NULL);
	assert(interface != NULL);

	s1d135xx_t *p = (s1d135xx_t *) malloc(sizeof(s1d135xx_t));
	p->gpio = gpios;
	p->interface = interface;
	p->pins = pins;
#if 1
	p->i2c = i2c;
#else
	epson_i2c_init(p, i2c);
#endif

	p->hard_reset = s1d135xx_hard_reset;
	p->soft_reset = s1d135xx_soft_reset;
	p->wait_for_idle = s1d135xx_wait_idle;
	p->read_reg = s1d135xx_read_reg;
	p->write_reg = s1d135xx_write_reg;
	p->update_reg = s1d135xx_update_reg;
	p->set_registers = s1d135xx_set_registers;
	p->send_cmd_with_params = s1d135xx_cmd;
	p->set_epd_power = s1d135xx_set_epd_power;
	p->set_power_state = s1d135xx_set_power_state;
	p->load_init_code = s1d135xx_load_init_code;
	p->wait_update_end = s1d135xx_wait_update_end;
	p->configure_update = s1d135xx_configure_update;
	p->execute_update = s1d135xx_execute_update;
	p->load_png_image = s1d135xx_load_png_image;
	p->pattern_check = s1d135xx_pattern_check;
	p->fill = s1d135xx_fill;
	p->wait_dspe_trig = s1d135xx_wait_dspe_trig;
	p->init_gate_drv = s1d135xx_init_gate_drv;
	p->load_wflib = s1d135xx_load_wflib;

	// controller specific
	p->check_prod_code = s1d13541_check_prod_code;
	p->clear_init = s1d13541_clear_init;
	p->init = s1d13541_init_controller;
	p->init_clocks = s1d13541_init_clocks;
	return p;
}

/**
 * does a hard reset by low active reset pin
 *
 * @param p s1d135xx structure
 */
static void s1d135xx_hard_reset(struct s1d135xx* p) {

	if (p->pins->reset == PL_GPIO_NONE) {
		LOG("Warning: no hard reset");
		return;
	}

	pl_gpio_set(p->gpio, p->pins->reset, 0);
	usleep(4000);
	pl_gpio_set(p->gpio, p->pins->reset, 1);
	usleep(10000);
}

/**
 * does a soft reset by 0x0008 register
 *
 * @param p s1d135xx structure
 */
static int s1d135xx_soft_reset(struct s1d135xx *p) {
	s1d135xx_write_reg(p, S1D135XX_REG_SOFTWARE_RESET, 0xFF);

	return s1d135xx_wait_idle(p);
}

/**
 * wait for idle, by checking either hrdy pin or system status register 0x000A
 *
 * @param p s1d135xx structure
 */
static int s1d135xx_wait_idle(struct s1d135xx *p) {
	unsigned long timeout = 50000; // ca. 20s
	/*
	 if(p->interface->interface_type == PARALLEL){
	 //LOG("PARALLEL");
	 timeout = 2;
	 }
	 //*/
	while (!get_hrdy(p)) {
		--timeout;
		if (timeout == 0) {
			LOG("HRDY timeout");
			return -ETIME;
		}
	}
	return 0;
}

/**
 * read register value
 *
 * @param p s1d135xx structure
 * @param reg register address
 * @return register value
 */
static uint16_t s1d135xx_read_reg(struct s1d135xx *p, uint16_t reg) {

	uint16_t val;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_READ_REG);
	send_param(p->interface, reg);
	p->interface->read_bytes(p->interface, (uint8_t *) &val, sizeof(uint16_t));

	if (p->interface->mSpi)
		p->interface->read_bytes(p->interface, (uint8_t *) &val,
				sizeof(uint16_t));

	set_cs(p, 1);

	if (p->interface->mSpi)
		val = be16toh(val);

	return val;
}

/**
 * write register value
 *
 * @param p s1d135xx structure
 * @param reg register address
 * @return register value
 * @return status
 */
static int s1d135xx_write_reg(struct s1d135xx *p, uint16_t reg, uint16_t val) {

	const uint16_t params[] = { reg, val };

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_params(p->interface, params, ARRAY_SIZE(params));
	set_cs(p, 1);

	return 0;
}

/**
 * read bytes from epson spi interface
 *
 * @param p s1d135xx structure
 * @param val pointer to data buffer
 * @return size buffer size
 * @return status
 */
static int s1d135xx_read_spi(struct s1d135xx *p, uint8_t *val, size_t size) {

	unsigned int i;
	uint16_t _val = 0;
	const uint16_t params[] = { 0x0202, 0x0000 };

	set_cs(p, 0);
	for (i = 0; i < size; i++) {
		send_cmd(p, S1D135XX_CMD_WRITE_REG);
		send_params(p->interface, params, ARRAY_SIZE(params));

		send_cmd(p, S1D135XX_CMD_READ_REG);
		send_param(p->interface, 0x0200);
		p->interface->read_bytes(p->interface, (uint8_t *) &_val,
				sizeof(uint16_t));
		p->interface->read_bytes(p->interface, (uint8_t *) &_val,
				sizeof(uint16_t));
		_val = be16toh(_val);
		val[i] = (uint8_t) _val;
	}
	set_cs(p, 1);

	return 0;
}

/**
 * write bytes to epson spi interface
 *
 * @param p s1d135xx structure
 * @param val pointer to data buffer
 * @return size buffer size
 * @return status
 */
static int s1d135xx_write_spi(struct s1d135xx *p, uint8_t *val, size_t size) {
	unsigned int i;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(p->interface, 0x0202);
	for (i = 0; i < size; i++) {
		send_param(p->interface, ((uint16_t) val[i]) | 0x0100);
	}
	set_cs(p, 1);

	return 0;
}

/**
 * update register
 *
 * @param p s1d135xx structure
 * @param reg register address
 * @param val register value
 * @param bitmask register bitmask
 * @return status
 */
static int s1d135xx_update_reg(struct s1d135xx *p, uint16_t reg, uint16_t val,
		const uint32_t bitmask) {

	uint16_t current_val = s1d135xx_read_reg(p, reg);
	uint16_t new_val = (val & bitmask) | (current_val & (~bitmask));

	s1d135xx_write_reg(p, reg, new_val);

	return 0;
}

/**
 * set register
 *
 * @param p s1d135xx structure
 * @param map pointer to regSetting structure
 * @param regSetting count
 * @return status
 */
static int s1d135xx_set_registers(struct s1d135xx *p, const regSetting_t* map,
		int n) {
	int stat = -1;
	unsigned int reg, val;
	int settingIdx;

	if (map != NULL) {
		stat = 0;
		for (settingIdx = 0; settingIdx < n; settingIdx++) {
			// Assume failure
			stat = -1;

			reg = map[settingIdx].addr;
			val = ((uint16_t *) map[settingIdx].val)[0];
			s1d135xx_write_reg(p, reg, val);
			if (val == s1d135xx_read_reg(p, reg)) {
				stat = 0;	// success
			}

			if (stat < 0) {
				break;
			}
		}
	}

	return stat;
}

static void s1d135xx_cmd(struct s1d135xx *p, uint16_t cmd,
		const uint16_t *params, size_t n) {
	set_cs(p, 0);
	send_cmd(p, cmd);
	send_params(p->interface, params, n);
	set_cs(p, 1);
}

static int s1d135xx_set_epd_power(struct s1d135xx *p, int on) {
	uint16_t arg = on ? S1D135XX_PWR_CTRL_UP : S1D135XX_PWR_CTRL_DOWN;
	uint16_t tmp, timeout = 400;

#if VERBOSE
	LOG("EPD power o%s", on ? "n" : "ff");
#endif

	if (s1d135xx_wait_idle(p))
		return -ETIME;

	s1d135xx_write_reg(p, S1D135XX_REG_PWR_CTRL, arg);

	do {
		tmp = s1d135xx_read_reg(p, S1D135XX_REG_PWR_CTRL);
		usleep(250);
		timeout--;
	} while ((tmp & S1D135XX_PWR_CTRL_BUSY) && timeout);

	if (on && ((tmp & S1D135XX_PWR_CTRL_CHECK_ON) !=
	S1D135XX_PWR_CTRL_CHECK_ON)) {
		LOG("Failed to turn the EPDC power on");
		return -EEPDC;
	}

	return 0;
}

static int s1d135xx_set_power_state(struct s1d135xx *p,
		enum pl_epdc_power_state state) {
	const struct s1d135xx_pins *pins = p->pins;
	int stat = -1;

	set_cs(p, 1);
	set_hdc(p, 1);
	pl_gpio_set(p->gpio, pins->vcc_en, 1);
	pl_gpio_set(p->gpio, pins->clk_en, 1);

	if (s1d135xx_wait_idle(p))
		return -ETIME;

	switch (state) {
	case PL_EPDC_RUN:
		send_cmd_cs(p, S1D135XX_CMD_RUN);
		stat = s1d135xx_wait_idle(p);
		break;

	case PL_EPDC_STANDBY:
		send_cmd_cs(p, S1D135XX_CMD_STBY);
		stat = s1d135xx_wait_idle(p);
		break;

	case PL_EPDC_SLEEP:
		send_cmd_cs(p, S1D135XX_CMD_STBY);
		stat = s1d135xx_wait_idle(p);
		pl_gpio_set(p->gpio, pins->clk_en, 0);
		break;

	case PL_EPDC_OFF:
		send_cmd_cs(p, S1D135XX_CMD_SLEEP);
		stat = s1d135xx_wait_idle(p);
		pl_gpio_set(p->gpio, pins->clk_en, 0);
		pl_gpio_set(p->gpio, pins->vcc_en, 0);
		set_hdc(p, 0);
		set_cs(p, 0);
		break;
	}

	return stat;
}

static int s1d135xx_load_init_code(struct s1d135xx *p,
		const char *init_code_path) {
	FILE *init_code_file;
	uint16_t checksum;
	int stat;
	init_code_file = fopen(init_code_path, "rb");

	if (init_code_file == NULL) {
		LOG("Couldn't open initialization code file.");
		return -ENOENT;
	}

	if (p->wait_for_idle(p))
		return -ETIME;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_INIT_SET);
	stat = transfer_file(p->interface, init_code_file);
	set_cs(p, 1);
	fclose(init_code_file);

	if (stat < 0) {
		LOG("Failed to transfer init code file");
		return -ENODATA;
	}

	if (p->wait_for_idle(p))
		return -ETIME;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_INIT_STBY);
	send_param(p->interface, 0x0500);
	set_cs(p, 1);
	sleep(1);

	if (p->wait_for_idle(p))
		return -ETIME;

	checksum = p->read_reg(p, S1D135XX_REG_SEQ_AUTOBOOT_CMD);

	if (!(checksum & (uint16_t) S1D135XX_INIT_CODE_CHECKSUM_OK)) {
		LOG("Init code checksum error");
		return -EEPDC;
	}

	return 0;
}

static int s1d135xx_wait_update_end(struct s1d135xx *p) {
	if (p->wait_for_idle(p))
		return -ETIME;
	if (s1d135xx_wait_dspe_trig(p))
		return -ETIME;
	send_cmd_cs(p, S1D135XX_CMD_WAIT_DSPE_FREND);
	return p->wait_for_idle(p);
}

static int s1d135xx_configure_update(struct s1d135xx *p, int wfid,
		enum pl_update_mode mode, const struct pl_area *area) {
	assert(p != NULL);
	struct s1d135xx_update_cmd *cmd = &p->next_update_cmd;

	// configure update command
	switch (mode) {
	case PL_FULL_UPDATE:
	case PL_FULL_UPDATE_NOWAIT:
		cmd->cmd_code = S1D135XX_CMD_UPDATE_FULL;
		break;
	case PL_PART_UPDATE:
	case PL_PART_UPDATE_NOWAIT:
		cmd->cmd_code = S1D135XX_CMD_UPDATE_PART;
		break;
	case PL_FULL_AREA_UPDATE:
	case PL_FULL_AREA_UPDATE_NOWAIT:
		cmd->cmd_code = S1D135XX_CMD_UPDATE_FULL_AREA;
		break;
	case PL_PART_AREA_UPDATE:
	case PL_PART_AREA_UPDATE_NOWAIT:
		cmd->cmd_code = S1D135XX_CMD_UPDATE_PART_AREA;
		break;
	}

	// configure update parameter
	if (mode == PL_FULL_UPDATE || mode == PL_PART_UPDATE) {
		cmd->param_count = 1;
		cmd->params[0] = S1D135XX_WF_MODE(wfid);
	} else if ((mode == PL_FULL_AREA_UPDATE || mode == PL_PART_AREA_UPDATE)
			&& area != NULL) {
		cmd->param_count = 5;
		cmd->params[0] = S1D135XX_WF_MODE(wfid);
		cmd->params[1] = (area->left & S1D135XX_XMASK);
		cmd->params[2] = (area->top & S1D135XX_YMASK);
		cmd->params[3] = (area->width & S1D135XX_XMASK);
		cmd->params[4] = (area->height & S1D135XX_YMASK);
	} else {
		abort_msg("Area update is selected but no area coordinates specified!",
				ABORT_APPLICATION);
		return -EINVAL;
	}
#if VERBOSE
	LOG("%s: stat: %i", __func__, 0);
#endif

	return 0;

}

static int s1d135xx_execute_update(struct s1d135xx *p) {
	assert(p != NULL);
	struct s1d135xx_update_cmd *cmd = &p->next_update_cmd;
	int stat = 0;
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	p->send_cmd_with_params(p, cmd->cmd_code, cmd->params, cmd->param_count);

	return stat;
}

static int s1d135xx_load_buffer(struct s1d135xx *p, const char *buffer,
		uint16_t mode, unsigned bpp, const struct pl_area *area) {
	assert(p != NULL);
	int stat = 0;

	int height = 0;
	int width = 0;

	int memorySize;

	if (area == NULL) {
		height = p->xres;
		width = p->yres;
		memorySize = p->yres * p->xres;
	} else {
		memorySize = area->width * area->height;
		height = area->height;
		width = area->width;
	}
	// scramble image
	if (area)
		LOG("AREA: L: %i, T: %i, H: %i, W: %i", area->left, area->top,
				area->height, area->width);
	uint8_t *scrambledPNG = malloc(height * width);
	scramble_array((uint8_t*) buffer, scrambledPNG, &height, &width,
			p->display_scrambling);

	// adapt image to memory

	uint8_t *memoryBuffer = malloc(p->yres * p->xres);
	memory_padding(scrambledPNG, memoryBuffer, height, width, p->yres, p->xres,
			p->yoffset, p->xoffset);

	// memory optimisation - if 4 bit per pixel mode is used
	if (bpp == 4) {
		memorySize /= 2;
		int bitIdx;
		for (bitIdx = 0; bitIdx < memorySize; bitIdx++) {
			memoryBuffer[bitIdx] = (memoryBuffer[bitIdx * 2 + 1] & 0xF0)
					| (memoryBuffer[bitIdx * 2] >> 4);
		}
	}

	set_cs(p, 0);

	if (area != NULL) {
		send_cmd_area(p, S1D135XX_CMD_LD_IMG_AREA, mode, area);
	} else {
		send_cmd(p, S1D135XX_CMD_LD_IMG);
		send_param(p->interface, mode);
	}

	set_cs(p, 1);
	/*
	 if (p->wait_for_idle(p))
	 return -ETIME;
	 //*/
	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(p->interface, S1D135XX_REG_HOST_MEM_PORT);

	transfer_data(p->interface, memoryBuffer, memorySize);

	set_cs(p, 1);

	if (memoryBuffer)
		free(memoryBuffer);

	if (stat < 0)
		return -EEPDC;
	/*
	 if (p->wait_for_idle(p))
	 return -1;
	 //*/
	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);
	return p->wait_for_idle(p);

}

static int s1d135xx_load_png_image(struct s1d135xx *p, const char *path,
		uint16_t mode, unsigned bpp, struct pl_area *area, int top, int left) {
	assert(p != NULL);

	int v_yres, height = 0;
	int v_xres, width = 0;

	int memorySize = p->yres * p->xres;
	uint8_t *scrambledPNG;
	if (p->mediaType == 0) {
		LOG("BW");
		uint8_t *pngBuffer;
		// read png image
		if (read_png(path, &pngBuffer, &width, &height))
			return -ENOENT;
		//if the image does fit the screen rotated, rotate
		if (!p->display_scrambling) {
			if (height == p->xres && width == p->yres) {
				rotate_8bit_image(&height, &width, pngBuffer);
			}
			v_xres = p->xres - (2 * p->xoffset);
			v_yres = p->yres - p->yoffset;
		} else {
			if (p->display_scrambling & SCRAMBLING_GATE_SCRAMBLE_MASK) {
				v_xres = p->xres * 2;
				v_yres = p->yres / 2;
				if (area) {
					area->left /= 2;
					area->top *= 2;
					area->width /= 2;
					area->height *= 2;
				}
				if (height == (v_xres) && width == (v_yres)
						&& (height != width)) {
					rotate_8bit_image(&height, &width, pngBuffer);
					LOG("BWS %ix%i -> %ix%i", height, width, p->yres, p->xres);
				}
			} else if (p->display_scrambling & SCRAMBLING_SOURCE_SCRAMBLE_MASK) {
				v_xres = p->xres / 2;
				v_yres = p->yres * 2;
				if (area) {
					area->left *= 2;
					area->top /= 2;
					area->width *= 2;
					area->height /= 2;
				}
				if (height == (v_xres) && width == (v_yres)
						&& (height != width)) {
					rotate_8bit_image(&height, &width, pngBuffer);
					LOG("BWS2 %ix%i -> %ix%i", height, width, p->yres, p->xres);
				}
			}
		}

		// scramble image
		scrambledPNG = malloc(max(height, p->yres) * max(width, p->xres));
		scramble_array(pngBuffer, scrambledPNG, &height, &width,
				p->display_scrambling);
		if (pngBuffer)
			free(pngBuffer);
	}
	else if (p->mediaType == 2) {
		LOG("ACEP");

		uint8_t *pngBuffer;
		// read png image
		if (read_rgb_png_to_iridis(path, &pngBuffer, &width, &height))
			return -ENOENT;

		// scramble image
		scrambledPNG = malloc(max(height, p->yres) * max(width, p->xres));
		scramble_array(pngBuffer, scrambledPNG, &height, &width,
				p->display_scrambling);
		if (pngBuffer)
			free(pngBuffer);
	}
	else {
		LOG("CFA");
		rgbw_pixel_t *pngBuffer;
		// read png image
		if (read_rgbw_png(path, &pngBuffer, &width, &height))
			return -ENOENT;

		// apply cfa filter to resolution
		v_xres = (p->xres - (2 * p->xoffset)) / 2;
		v_yres = (p->yres - p->yoffset) / 2;

		if (!p->display_scrambling) {
//*
			if (area) {
				area->left *= 2;
				area->top *= 2;
				area->width *= 2;
				area->height *= 2;
			}
//*/
			if (height == v_xres && width == v_yres && (height != width)) {
				rotate_rgbw_image(&height, &width, pngBuffer);
				LOG("CFA %ix%i -> %ix%i", height, width, p->yres, p->xres);
			}
		} else {
			if (p->display_scrambling & SCRAMBLING_GATE_SCRAMBLE_MASK) {
				v_xres = v_xres * 2;
				v_yres = v_yres / 2;
				if (area) {
					area->top *= 4;
					area->height *= 4;
				}
				if (height == (v_xres) && width == (v_yres)
						&& (height != width)) {
					rotate_rgbw_image(&height, &width, pngBuffer);
				}
			} else if (p->display_scrambling & SCRAMBLING_SOURCE_SCRAMBLE_MASK) {
				v_xres = v_xres / 2;
				v_yres = v_yres * 2;
				if (area) {
					area->left *= 4;
					//area->top /= 2;
					area->width *= 4;
					//area->height /= 2;

				}
				if (height == (v_xres) && width == (v_yres)
						&& (height != width)) {
					rotate_rgbw_image(&height, &width, pngBuffer);
				}
			}
		}

		// scramble image
		scrambledPNG = malloc(4 * max(height, p->yres) * max(width, p->xres));
		uint8_t *colorBuffer = malloc(
				4 * max(height, p->yres) * max(width, p->xres));
		rgbw_processing((uint32_t*) &width, (uint32_t*) &height, pngBuffer,
				colorBuffer, (struct pl_area*) (area) ? NULL : area,
				p->cfa_overlay);
		scramble_array(colorBuffer, scrambledPNG, &height, &width,
				p->display_scrambling);
		free(colorBuffer);
		if (pngBuffer)
			free(pngBuffer);
	}
//*
	if (area == NULL) {
//		if (height > v_yres || width > v_xres) {
//			area = malloc(sizeof(struct pl_area));
//			area->height = min(height, v_yres);
//			area->width = min(width, v_xres);
//			area->left = ((int) (v_xres - width) < 0) ? 0 : v_xres - width;
//			area->top = ((int) (v_yres - height) < 0) ? 0 : v_yres - height;
//			//*
//			if (p->cfa_overlay.r_position != -1) {
//				area->left *= 2;
//				area->top *= 2;
//				area->width *= 2;
//				area->height *= 2;
//			}
//
//		}

	} else {
		if (area->height + area->top > p->yres
				|| area->width + area->left > p->xres) {
			//crop image if bigger than screen
			area->height = min(area->height+area->top, p->yres) - area->top;
			area->width = min(area->width+area->left, p->xres) - area->left;
		}
	}
//*/
	// adapt image to memory

	uint8_t *memoryBuffer = malloc(max(height, p->yres) * max(width, p->xres));
	if (area == NULL) {
		memory_padding(scrambledPNG, memoryBuffer, height, width, p->yres,
				p->xres, p->yoffset, p->xoffset);
	} else {
		memory_padding_area(scrambledPNG, memoryBuffer, height, width,
				p->yoffset, p->xoffset, area, top, left);
		memorySize = (area->height + p->yoffset) * (area->width + p->xoffset);
	}

	// memory optimisation - if 4 bit per pixel mode is used
	if (bpp == 4) {
		memorySize /= 2;
		int bitIdx;
		for (bitIdx = 0; bitIdx < memorySize; bitIdx++) {
			memoryBuffer[bitIdx] = (memoryBuffer[bitIdx * 2 + 1] & 0xF0)
					| (memoryBuffer[bitIdx * 2] >> 4);
		}
	}
	set_cs(p, 0);

	if (area != NULL) {
		if (top || left) {
			// for scrambling or cfa overlay shift by 2 always
			if ((p->display_scrambling & SCRAMBLING_SOURCE_SCRAMBLE_MASK)
					|| (p->display_scrambling & SCRAMBLING_GATE_SCRAMBLE_MASK)
					|| !(p->cfa_overlay.r_position == -1)) {
				top = (top / 2) * 2;
				left = (left / 2) * 2;
			}

			area->top = top;
			area->left = left;
		}
		area->top += p->yoffset;
		area->left += p->xoffset;
		send_cmd_area(p, S1D135XX_CMD_LD_IMG_AREA, mode, area);
	} else {
		send_cmd(p, S1D135XX_CMD_LD_IMG);
		send_param(p->interface, mode);
	}

	set_cs(p, 1);

	if (p->wait_for_idle(p))
		return -ETIME;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(p->interface, S1D135XX_REG_HOST_MEM_PORT);

	//if (area == NULL)
	transfer_data(p->interface, memoryBuffer, memorySize);

	set_cs(p, 1);

	if (memoryBuffer)
		free(memoryBuffer);
	if (scrambledPNG)
		free(scrambledPNG);

//*
	if (p->wait_for_idle(p))
		return -ETIME;
//*/
	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

	return p->wait_for_idle(p);
}

#if 0
/**
 * This function pads the target (memory) with offset source and gate lines if needed.
 * The source content will be placed in the right lower corner, while the left upper space is containing the offset lines.
 */
static void memory_padding(uint8_t *source, uint8_t *target, int s_gl, int s_sl, int t_gl, int t_sl)
{
	int sl, gl;
	int gl_offset = t_gl - s_gl;
	int sl_offset = t_sl - s_sl;

	for (gl=0; gl<s_gl; gl++)
	for(sl=0; sl<s_sl; sl++)
	{
		target [(gl+gl_offset)*t_sl+(sl+sl_offset)] = source [gl*s_sl+sl];
	}
}
#else

/**
 * This function pads the target (memory) with offset source and gate lines if needed.
 * If no offset is defined (o_gl=-1, o_sl=-1) the source content will be placed in the right lower corner,
 * while the left upper space is containing the offset lines.
 */
static void memory_padding(uint8_t *source, uint8_t *target,
		int source_gatelines, int source_sourcelines, int target_gatelines,
		int target_sourcelines, int gate_offset, int source_offset) {
	int sourceline, gateline;
	int _gateline_offset = 0;
	int _sourceline_offset = 0;

	if (gate_offset > 0)
		_gateline_offset = gate_offset;
	else
		_gateline_offset = target_gatelines - source_gatelines;

	if (source_offset > 0)
		_sourceline_offset = source_offset;
	else
		_sourceline_offset = target_sourcelines - source_sourcelines;

	for (gateline = 0; gateline < source_gatelines; gateline++)
		for (sourceline = 0; sourceline < source_sourcelines; sourceline++) {
			int source_index = gateline * source_sourcelines + sourceline;
			int target_index = (gateline + _gateline_offset)
					* target_sourcelines + (sourceline + _sourceline_offset);
			if (!(source_index < 0 || target_index < 0)) {
				target[target_index] = source[source_index];
				source[source_index] = 0x00;
			}
		}
}

static void memory_padding_area(uint8_t *source, uint8_t *target,
		int source_gatelines, int source_sourcelines, int gate_offset,
		int source_offset, struct pl_area* source_area, int top, int left) {
	int sourceline, gateline;
#if VERBOSE
	LOG("%s: source_gatelines %i, source_sourcelines %i, gate_offset %i, source_offset %i, source_area %p, top %i, left %i", __func__, source_gatelines, source_sourcelines, gate_offset, source_offset, source_area, top, left);
	LOG("%s: AREA: L: %i, T: %i, H: %i, W: %i", __func__, source_area->left, source_area->top, source_area->height, source_area->width);
#endif
	for (gateline = source_area->top;
			gateline < source_area->top + source_area->height; gateline++) {
		for (sourceline = source_area->left;
				sourceline < source_area->left + source_area->width;
				sourceline++) {
			int source_index = (gateline/*+source_area->top*/)
					* (source_sourcelines/*+source_area->left*/) + sourceline;
			int target_index = (gateline - source_area->top)
					* source_area->width + (sourceline - source_area->left);
			if (!(source_index < 0 || target_index < 0)) {
				target[target_index] = source[source_index];
				source[source_index] = 0x80;
			}
		}
	}
}

#endif

static int s1d135xx_pattern_check(struct s1d135xx *p, uint16_t height,
		uint16_t width, uint16_t checker_size, uint16_t mode) {
	uint16_t i = 0, j = 0, k = 0;
	uint16_t val = 0;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_LD_IMG);
	send_param(p->interface, mode);
	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -ETIME;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(p->interface, S1D135XX_REG_HOST_MEM_PORT);

	for (i = 0; i < height; i++) {
		k = i / checker_size;
		for (j = 0; j < width; j += 2) {
			val = (k + (j / checker_size)) % 2 ? 0xFFFF : 0x0;
			send_param(p->interface, val);
		}
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -ETIME;

	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

	return 0;
}

static int s1d135xx_load_wflib(struct s1d135xx *p, const char *filename,
		uint32_t addr) {
	FILE *file;
	file = fopen(filename, "rb");
	if (file == NULL) {
		LOG("Error during waveform file loading.");
		return -ENOENT;
	}

	uint16_t params[4];
	fseek(file, 0, SEEK_END);
	uint32_t size2 = ftell(file) / 2;
	fseek(file, 0, SEEK_SET);

	if (p->wait_for_idle(p))
		return -ETIME;

	params[0] = addr & 0xFFFF;
	params[1] = (addr >> 16) & 0xFFFF;
	params[2] = size2 & 0xFFFF;
	params[3] = (size2 >> 16) & 0xFFFF;
	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_BST_WR_SDR);
	send_params(p->interface, params, ARRAY_SIZE(params));
	set_cs(p, 1);

	int bufferSize = 256;
	uint8_t dataBuf[bufferSize];
	int n;
	int totalCount = 0;
	while ((n = fread(dataBuf, sizeof(uint8_t), bufferSize, file)) > 0) {
		wflib_wr(p, dataBuf, n);
		totalCount += n;
	}

	if (p->wait_for_idle(p))
		return -ETIME;

	send_cmd_cs(p, S1D135XX_CMD_BST_END_SDR);
	fclose(file);
	return p->wait_for_idle(p);
}

static int s1d135xx_init_gate_drv(struct s1d135xx *p) {
	send_cmd_cs(p, S1D135XX_CMD_EPD_GDRV_CLR);

	return p->wait_for_idle(p);
}

static int s1d135xx_wait_dspe_trig(struct s1d135xx *p) {
	send_cmd_cs(p, S1D135XX_CMD_WAIT_DSPE_TRG);

	return p->wait_for_idle(p);
}

static int s1d135xx_fill(struct s1d135xx *p, uint16_t mode, unsigned bpp,
		const struct pl_area *a, uint8_t grey) {
	struct pl_area full_area;
	const struct pl_area *fill_area;

	set_cs(p, 0);

	if (a != NULL) {
		send_cmd_area(p, S1D135XX_CMD_LD_IMG_AREA, mode, a);
		fill_area = a;
	} else {
		send_cmd(p, S1D135XX_CMD_LD_IMG);
		send_param(p->interface, mode);
		full_area.top = 0;
		full_area.left = 0;
		full_area.width = p->xres;
		full_area.height = p->yres;
		fill_area = &full_area;
	}

	set_cs(p, 1);

	if (p->wait_for_idle(p))
		return -ETIME;

	return do_fill(p, fill_area, bpp, grey);
}

// -----------------------------------------------------------------------------
// controller specific interface functions
// ------------------------------
static int s1d13541_check_prod_code(struct s1d135xx *p, uint16_t ref_code) {
	return check_prod_code(p, ref_code);
}

static int s1d13524_check_prod_code(struct s1d135xx *p, uint16_t ref_code) {
	uint16_t rev;
	uint16_t conf;
	int stat = 0;
	set_cs(p, 0);
	stat = check_prod_code(p, ref_code);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;
	set_cs(p, 1);
	rev = p->read_reg(p, 0x0000);
	conf = p->read_reg(p, 0x0004);

	LOG("Rev: %04X, conf: %04X", rev, conf);

	uint16_t expected_conf = (p->interface->mSpi) ? 0x001F : 0x001E;

	if ((rev != 0x0100)
			|| (conf != expected_conf /*001F for serial connection/1E for parallel */)) {
		LOG("Invalid rev/conf values");
		return -EEPDC;
	}

	return 0;
}

static int s1d13541_clear_init(struct s1d135xx *p) {
	send_cmd_cs(p, S1D135XX_CMD_UPD_INIT);

	if (p->wait_for_idle(p))
		return -ETIME;

	return s1d135xx_wait_dspe_trig(p);
}

static int s1d13524_clear_init(struct s1d135xx *p) {
	static const uint16_t params[] = { 0x0500 };
	int stat = 0;
	p->send_cmd_with_params(p, 0x32, params, ARRAY_SIZE(params));

	stat = p->wait_for_idle(p);
	if (stat < 0)
		return stat;

	stat = s1d13524_init_ctlr_mode(p);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;

#if 1 /* ToDo: find out why the first image state goes away */
	stat = p->fill(p, S1D13524_LD_IMG_4BPP, 4, NULL, 0xFF);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;
#endif

	return 0;
}

static int s1d13541_init_controller(struct s1d135xx *p) {
	int stat = 0;

	// enable display 3V3 - its a workaround, since the epson ic is powerd by the display 3V3
	uint8_t data[2] = {0x01, 0x20};
	p->i2c->write(p->i2c, 0x68, data, sizeof(data), 0);

	p->hrdy_mask = S1D13541_STATUS_HRDY;
	p->hrdy_result = S1D13541_STATUS_HRDY;
	p->measured_temp = -127;
	p->hard_reset(p);

	stat = p->soft_reset(p);
	if (stat < 0)
		return stat;

	stat = p->check_prod_code(p, S1D13541_PROD_CODE);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;

	stat = s1d13541_init_clocks(p);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;

	return 0;
}

static int s1d13524_init_controller(struct s1d135xx *p) {
	int stat = 0;
	p->hard_reset(p);

	stat = p->soft_reset(p);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;

	stat = p->check_prod_code(p, S1D13524_PROD_CODE);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;

	p->write_reg(p, S1D135XX_REG_I2C_STATUS, S1D13524_I2C_DELAY);

	stat = s1d13524_init_clocks(p);
#if VERBOSE
	LOG("%s: stat: %i", __func__, stat);
#endif
	if (stat < 0)
		return stat;

	return 0;
}

static int s1d13541_init_clocks(struct s1d135xx *p) {

	p->write_reg(p, S1D135XX_REG_I2C_CLOCK, S1D13541_I2C_CLOCK_DIV);
	p->write_reg(p, S1D13541_REG_CLOCK_CONFIG,
	S1D13541_INTERNAL_CLOCK_ENABLE);

	return p->wait_for_idle(p);
}

static int s1d13524_init_clocks(struct s1d135xx *p) {

	static const uint16_t params[] = {
	S1D13524_PLLCFG0, S1D13524_PLLCFG1,
	S1D13524_PLLCFG2, S1D13524_PLLCFG3, };

	p->send_cmd_with_params(p, S1D13524_CMD_INIT_PLL, params,
			ARRAY_SIZE(params));

	if (p->wait_for_idle(p))
		return -ETIME;

	p->write_reg(p, S1D13524_REG_POWER_SAVE_MODE, 0x0);
	p->write_reg(p, S1D135XX_REG_I2C_CLOCK, S1D13524_I2C_CLOCK_DIV);

	return p->wait_for_idle(p);
}

// -----------------------------------------------------------------------------
// public functions
// ------------------------------
int s1d13524_init_ctlr_mode(struct s1d135xx *p) {

	static const uint16_t par[] = {
	S1D13524_CTLR_AUTO_WFID, (S1D13524_CTLR_NEW_AREA_PRIORITY |
	S1D13524_CTLR_PROCESSED_SINGLE), };

	p->send_cmd_with_params(p, S1D13524_CMD_INIT_CTLR_MODE, par,
			ARRAY_SIZE(par));

	return p->wait_for_idle(p);
}

// -----------------------------------------------------------------------------
// private functions
// ------------------------------
static int check_prod_code(struct s1d135xx *p, uint16_t ref_code) {

	uint16_t prod_code;

	prod_code = p->read_reg(p, S1D135XX_REG_REV_CODE);

	LOG("Product code: 0x%04X", prod_code);

	if (prod_code != ref_code) {
		LOG("Invalid product code, expected 0x%04X", ref_code);
		return -EEPDC;
	}

	return 0;
}

static int get_hrdy(struct s1d135xx *p) {

	uint16_t status;

	if (p->pins->hrdy != PL_GPIO_NONE)
		return pl_gpio_get(p->gpio, p->pins->hrdy);

	status = s1d135xx_read_reg(p, S1D135XX_REG_SYSTEM_STATUS);

	return ((status & p->hrdy_mask) == p->hrdy_result);
}

static int do_fill(struct s1d135xx *p, const struct pl_area *area, unsigned bpp,
		uint8_t g) {

	uint16_t val16 = 0;
	uint16_t lines;
	uint16_t pixels = 0;
	// Only 16-bit transfers for now...
	assert(!(area->width % 2));

	switch (bpp) {
	case 1:
	case 2:
		LOG("Unsupported bpp");
		return -EINVAL;
	case 4:
		val16 = g & 0xF0;
		val16 |= val16 >> 4;
		val16 |= val16 << 8;
		pixels = area->width / 4;
		break;
	case 8:
		val16 = g | (g << 8);
		pixels = area->width / 2;
		break;
	default:
		assert_fail("Invalid bpp");
	}

	lines = area->height;

	if (s1d135xx_wait_idle(p))
		return -ETIME;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(p->interface, S1D135XX_REG_HOST_MEM_PORT);

	while (lines--) {
		uint16_t x = pixels;

		while (x--)
			send_param(p->interface, val16);
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -ETIME;

	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

	return s1d135xx_wait_idle(p);
}

static int wflib_wr(void *ctx, const uint8_t *data, size_t n) {

	struct s1d135xx *p = ctx;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(p->interface, S1D135XX_REG_HOST_MEM_PORT);
	transfer_data(p->interface, data, n);
	set_cs(p, 1);

	return 0;
}

static int transfer_file(struct pl_generic_interface *interface, FILE *file) {

	uint8_t data[DATA_BUFFER_LENGTH];

	for (;;) {
		size_t count;

		if ((count = fread(data, sizeof(uint8_t), DATA_BUFFER_LENGTH, file))
				< 0) {
			if (ferror(file)) {
				LOG("Error during instruction code file reading.");
				return -ENOENT;
			}
		}
		if (!count)
			break;

		transfer_data(interface, data, count);
	}

	return 0;
}

static void transfer_data(struct pl_generic_interface *interface,
		const uint8_t *data, size_t n) {
	// for SPI swap the bytes of the word
	if (interface->mSpi) {
		uint16_t *data16 = (uint16_t *) data;
		unsigned int wordIdx;
		for (wordIdx = 0; wordIdx < n / 2; wordIdx++) {
			data16[wordIdx] = htobe16(data16[wordIdx]);
		}
		wordIdx = 0;
	}

	const unsigned int chunkSize = 4096;
	// transfer full chunks of data
	while (n > chunkSize) {
		interface->write_bytes(interface, (uint8_t*) data,
				sizeof(uint8_t) * chunkSize);
		data += chunkSize;
		n -= chunkSize;
	}

	// transfer rest bytes
	if (n) {
		interface->write_bytes(interface, (uint8_t*) data, sizeof(uint8_t) * n);
	}
}

static void send_cmd_area(struct s1d135xx *p, uint16_t cmd, uint16_t mode,
		const struct pl_area *area) {

	const uint16_t args[] = { mode, (area->left & S1D135XX_XMASK), (area->top
			& S1D135XX_YMASK), (area->width & S1D135XX_XMASK), (area->height
			& S1D135XX_YMASK), };

	send_cmd(p, cmd);
	send_params(p->interface, args, ARRAY_SIZE(args));
}

static void send_cmd_cs(struct s1d135xx *p, uint16_t cmd) {

	set_cs(p, 0);
	send_cmd(p, cmd);
	set_cs(p, 1);
}

static void send_cmd(struct s1d135xx *p, uint16_t cmd) {

	if (p->interface->mSpi)
		cmd = htobe16(cmd);	// for serial comm

	set_hdc(p, 0);
	p->interface->write_bytes(p->interface, (uint8_t *) &cmd, sizeof(uint16_t));
	set_hdc(p, 1);
}

static void send_params(struct pl_generic_interface *interface,
		const uint16_t *params, size_t n) {

	size_t i;

	for (i = 0; i < n; ++i)
		send_param(interface, params[i]);
}

static void send_param(struct pl_generic_interface *interface, uint16_t param) {

	if (interface->mSpi)
		param = htobe16(param);	// for serial comm

	interface->write_bytes(interface, (uint8_t *) &param, sizeof(uint16_t));
}

static void set_cs(struct s1d135xx *p, int state) {
	//LOG("CS: %i", state);
	const unsigned cs = p->pins->cs0;
	if (cs != PL_GPIO_NONE)
		pl_gpio_set(p->gpio, p->pins->cs0, state);
}

static void set_hdc(struct s1d135xx *p, int state) {

	const unsigned hdc = p->pins->hdc;

	if (hdc != PL_GPIO_NONE)
		pl_gpio_set(p->gpio, hdc, state);
}

