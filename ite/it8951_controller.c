/*
  Plastic Logic EPD project on BeagleBone

  Copyright (C) 2020 Plastic Logic

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
 * it8951_controller.c
 *
 *  Created on: 14 Feb 2020
 *      Author: matti.haugwitz
 */

#include <unistd.h>

#include <pl/parser.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <pl/scramble.h>
#include <pl/hv.h>
#define LOG_TAG "it8951_controller"
#include "pl/utils.h"

#include "ite/it8951_controller.h"

static int trigger_update(struct pl_generic_controller *controller);
static int clear_update(pl_generic_controller_t *p);
static int init_controller(struct pl_generic_controller *controller, int use_wf_from_nvm);
static int configure_update(struct pl_generic_controller *controller, int wfid, enum pl_update_mode mode, const struct pl_area *area);
static int fill(struct pl_generic_controller *controller, const struct pl_area *a, uint8_t grey);
static int load_wflib(struct pl_generic_controller *controller, const char *filename);
static int load_png_image(struct pl_generic_controller *controller, const char *path, const struct pl_area *area, int left,	int top);
static int wait_update_end(struct pl_generic_controller *controller);
static int read_register(struct pl_generic_controller *controller, const regSetting_t* setting);
static int write_register(struct pl_generic_controller *controller, const regSetting_t setting, const uint32_t bitmask);
static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting);
static int set_registers(struct pl_generic_controller *controller, const regSetting_t* map, int n);
static int set_temp_mode(struct pl_generic_controller *controller, enum pl_epdc_temp_mode mode);
static int update_temp(struct pl_generic_controller *controller);

int it8951_controller_setup(struct pl_generic_controller *controller, it8951_t *it8951)
{
	assert(controller != NULL);
	assert(it8951 != NULL);

	controller->wait_update_end = wait_update_end;
	controller->load_wflib = load_wflib;
	controller->fill = fill;
	controller->load_image = load_png_image;

	controller->configure_update = configure_update;
	controller->trigger_update = trigger_update;
	controller->clear_update = clear_update;
	controller->read_register = read_register;
	controller->write_register = write_register;
	controller->send_cmd = send_cmd;

	controller->init = init_controller;
	controller->hw_ref = it8951;

	controller->set_temp_mode = set_temp_mode;
	controller->update_temp = update_temp;

	return 0;
}


static int trigger_update(struct pl_generic_controller *controller)
{
	return 0;
}

static int clear_update(pl_generic_controller_t *p)
{
	return 0;
}

static int init_controller(struct pl_generic_controller *controller, int use_wf_from_nvm)
{
	return 0;
}

static int configure_update(struct pl_generic_controller *controller, int wfid, enum pl_update_mode mode, const struct pl_area *area)
{
	return 0;
}

static int fill(struct pl_generic_controller *controller, const struct pl_area *a, uint8_t grey)
{
	return 0;
}

static int load_wflib(struct pl_generic_controller *controller, const char *filename)
{
	return 0;
}

static int load_png_image(struct pl_generic_controller *controller, const char *path, const struct pl_area *area, int left,	int top)
{
	return 0;
}

static int wait_update_end(struct pl_generic_controller *controller)
{
	return 0;
}

static int read_register(struct pl_generic_controller *controller, const regSetting_t* setting)
{
	return 0;
}

static int write_register(struct pl_generic_controller *controller, const regSetting_t setting, const uint32_t bitmask)
{
	return 0;
}

static int send_cmd(pl_generic_controller_t *p, const regSetting_t setting)
{
	return 0;
}

static int set_registers(struct pl_generic_controller *controller, const regSetting_t* map, int n)
{
	return 0;
}

static int set_temp_mode(struct pl_generic_controller *controller, enum pl_epdc_temp_mode mode)
{
	return 0;
}

static int update_temp(struct pl_generic_controller *controller)
{
	return 0;
}

