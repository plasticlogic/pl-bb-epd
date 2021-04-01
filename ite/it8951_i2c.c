/*
 * it8951_i2c.c
 *
 *  Created on: 31 Mar 2021
 *      Author: matti.haugwitz
 */

#include <ite/it8951_i2c.h>


static int it8951_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
		const uint8_t *data, uint8_t count, uint8_t flags);

static int it8951_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr,
		uint8_t *data, uint8_t count, uint8_t flags);

static int it8951_i2c_detect(struct pl_i2c *i2c);

int it8951_i2c_init(struct pl_generic_controller *controller, struct pl_i2c *i2c){

	i2c->controller = controller;
	i2c->read = it8951_i2c_read;
	i2c->write = it8951_i2c_write;
	i2c->detect = it8951_i2c_detect;

	return 0;
}


static int it8951_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
		const uint8_t *data, uint8_t count, uint8_t flags){

	struct pl_generic_controller * controller = i2c->controller;

	printf("IT8951 write I2C: Start\n");

	uint16_t buffer[4];

	regSetting_t reg;
	reg.addr = IT8951_TCON_BYPASS_I2C;
	reg.valCount = sizeof(buffer) / 2;
	buffer[0] = 0x00; // 0 = write
	buffer[1] = i2c_addr;

	int i = 0;
	for(i = 0; i < count; i++)
	{
		buffer[2 + i] = (uint16_t) data[i];
	}

	reg.val = buffer;

	controller->send_cmd(controller, reg);

	printf("IT8951 write I2C: End\n");

	return 0;
}

static int it8951_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr,
		uint8_t *data, uint8_t count, uint8_t flags){

	return 0;
}

static int it8951_i2c_detect(struct pl_i2c *i2c){

	return 0;
}



