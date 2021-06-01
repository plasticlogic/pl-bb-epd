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

	//printf("IT8951 write I2C: Start\n");

	uint16_t buffer[3 + count];

	regSetting_t reg;
	reg.addr = IT8951_TCON_BYPASS_I2C;
	reg.valCount = sizeof(buffer) / 2;

	if(count <= 1)
	{
		buffer[0] = 0x00; // 0 = read --> buffer size of 1 indicates contains only register address
	}
	else
	{
		buffer[0] = 0x01; // 1 = write
		count -= 1;
	}

	buffer[1] = i2c_addr;
	buffer[2] = (uint16_t) data[0];
	buffer[3] = count;

	int i = 0;
	if(buffer[0] == 0x01)
		for(i = 0; i < count; i++)
		{
			buffer[4 + i] = (uint16_t) data[1 + i];
			printf("Data: 0x%x\n", data[1 + i]);
		}

	reg.val = buffer;

	controller->send_cmd(controller, reg);

	//printf("IT8951 write I2C: End\n");

	return 0;
}

static int it8951_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr,
		uint8_t *data, uint8_t count, uint8_t flags){

	struct pl_generic_controller * controller = i2c->controller;
	it8951_t *it8951 = controller->hw_ref;
	struct pl_generic_interface *interface = it8951->interface;
	enum interfaceType *type = it8951->sInterfaceType;

	//printf("IT8951 read I2C: Start\n");

	int i = 0;
	for(i=0; i<count; i++)
	{
		usleep( 50000);
		TWord* value = IT8951ReadData(interface, type, 2);  //read data
		printf("Data: 0x%x\n", *value);

		data[i] = (uint8_t) *value;
	}

	//printf("IT8951 read I2C: End\n");

	return 0;
}

static int it8951_i2c_detect(struct pl_i2c *i2c){

	struct pl_generic_controller * controller = i2c->controller;

	printf("IT8951 detect I2C: Start\n");

	uint16_t buffer[5];

	regSetting_t reg;
	reg.addr = IT8951_TCON_BYPASS_I2C;
	reg.valCount = sizeof(buffer) / 2;
	buffer[0] = 0x01; // 0 = read, 1 = write
	buffer[1] = 0x68;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x11;

	reg.val = buffer;

	controller->send_cmd(controller, reg);

	printf("IT8951 detect I2C: End\n");

	return 0;
}



