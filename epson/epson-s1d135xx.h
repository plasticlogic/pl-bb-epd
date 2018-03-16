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
 * epson-s1d135xx.h -- Common Epson S1D135xx primitives
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_EPSON_S1D135XX_H
#define INCLUDE_EPSON_S1D135XX_H

#include <pl/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
struct pl_gpio;
struct pl_wflib;
struct pl_spi;

/* Set to 1 to enable verbose temperature log messages */
#define VERBOSE_TEMPERATURE                  0
#define S1D135XX_TEMP_MASK                   0x00FF
#define S1D135XX_PWR_CTRL_UP            0x8001
#define S1D135XX_PWR_CTRL_DOWN          0x8002
#define S1D135XX_PWR_CTRL_BUSY          0x0080
#define S1D135XX_PWR_CTRL_CHECK_ON      0x2200

enum s1d135xx_reg {
	S1D135XX_REG_REV_CODE              = 0x0002,
	S1D135XX_REG_SOFTWARE_RESET        = 0x0008,
	S1D135XX_REG_SYSTEM_STATUS         = 0x000A,
	S1D135XX_REG_I2C_CLOCK             = 0x001A,
	S1D135XX_REG_PERIPH_CONFIG         = 0x0020,
	S1D135XX_REG_HOST_MEM_PORT         = 0x0154,
	S1D135XX_REG_I2C_TEMP_SENSOR_VALUE = 0x0216,
	S1D135XX_REG_I2C_STATUS            = 0x0218,
	S1D135XX_REG_PWR_CTRL              = 0x0230,
	S1D135XX_REG_SEQ_AUTOBOOT_CMD      = 0x02A8,
	S1D135XX_REG_DISPLAY_BUSY          = 0x0338,
	S1D135XX_REG_INT_RAW_STAT          = 0x033A,
};

enum s1d135xx_rot_mode {
	S1D135XX_ROT_MODE_0   = 0,
	S1D135XX_ROT_MODE_90  = 1,
	S1D135XX_ROT_MODE_180 = 2,
	S1D135XX_ROT_MODE_270 = 3,
};

struct s1d135xx_pins {
	unsigned reset;
	unsigned cs0;
	unsigned hirq;
	unsigned hrdy;
	unsigned hdc;
	unsigned clk_en;
	unsigned vcc_en;
};

struct s1d135xx_update_cmd {
	uint16_t cmd_code;
	uint16_t params[5];
	size_t param_count;
};

typedef struct s1d135xx {
	const struct s1d135xx_pins *pins;
	struct pl_gpio *gpio;
	struct pl_generic_interface *interface;
	struct pl_i2c *i2c;
	uint16_t hrdy_mask;
	uint16_t hrdy_result;
	int measured_temp;
	unsigned xres;
	unsigned yres;
	int xoffset;
	int yoffset;
	int display_scrambling;
	cfa_overlay_t cfa_overlay;
	struct s1d135xx_update_cmd next_update_cmd;
	struct {
		uint8_t needs_update:1;
	} flags;


	void (*delete)(struct s1d135xx *this);
	int (*init)(struct s1d135xx *this);
	int (*wait_for_idle)(struct s1d135xx *this);
	void (*hard_reset)(struct s1d135xx *this);
	int (*soft_reset)(struct s1d135xx *this);

	uint16_t (*read_reg)(struct s1d135xx *this, uint16_t cmd);
	int (*write_reg)(struct s1d135xx *this, uint16_t cmd, uint16_t data);
	int (*update_reg)(struct s1d135xx *this, uint16_t cmd, uint16_t data, uint32_t bitmask);
	int (*read_spi)(struct s1d135xx *p, uint8_t *val, size_t size);
	int (*write_spi)(struct s1d135xx *this, uint8_t *data, size_t size);
	int (*set_registers)(struct s1d135xx *this, const regSetting_t* map, int n);
	void (*send_cmd_with_params)(struct s1d135xx *this, uint16_t cmd, const uint16_t *params, size_t n);
	int (*set_epd_power)(struct s1d135xx *this, int on);
	int (*set_power_state)(struct s1d135xx *this,  enum pl_epdc_power_state state);
	int (*check_prod_code)(struct s1d135xx *p, uint16_t code);
	int (*load_init_code)(struct s1d135xx *p, const char *filename);
	int (*wait_update_end)(struct s1d135xx *p);
	int (*configure_update)(struct s1d135xx *p, int wfid, enum pl_update_mode mode, const struct pl_area *area);
	int (*execute_update)(struct s1d135xx *p);
	int (*load_png_image)(struct s1d135xx *p, const char *path, uint16_t mode, unsigned bpp, struct pl_area *area, int top, int left);
	int (*load_buffer)(struct s1d135xx *p, const char *buffer, uint16_t mode, unsigned bpp, const struct pl_area *area);
	int (*pattern_check)(struct s1d135xx *p, uint16_t height, uint16_t width, uint16_t checker_size, uint16_t mode);
	int (*fill)(struct s1d135xx *p, uint16_t mode, unsigned bpp, const struct pl_area *a, uint8_t grey);
	int (*clear_init)(struct s1d135xx *p);
	int (*wait_dspe_trig)(struct s1d135xx *p);
	int (*init_gate_drv)(struct s1d135xx *p);
	int (*load_wflib)(struct s1d135xx *p, const char *filename, uint32_t addr);
	int (*init_clocks)(struct s1d135xx *p);
} s1d135xx_t;


s1d135xx_t *s1d13541_new(struct pl_gpio *gpios, struct pl_generic_interface *interface, struct pl_i2c* i2c, const struct s1d135xx_pins *pins);
s1d135xx_t *s1d13524_new(struct pl_gpio *gpios, struct pl_generic_interface *interface, struct pl_i2c* i2c, const struct s1d135xx_pins *pins);

int s1d13524_init_ctlr_mode(struct s1d135xx *p);


#endif /* INCLUDE_EPSON_S1D135XX_H */
