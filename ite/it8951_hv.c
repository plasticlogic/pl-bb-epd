/*
 * it8951_hv.c
 *
 *  Created on: 25 Jun 2020
 *      Author: matti.haugwitz
 */

#include <unistd.h>

#include <pl/hv.h>
#include <pl/pmic.h>
#include <pl/generic_interface.h>
#include <pl/i80.h>
#include <pl/spi_hrdy.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <ite/it8951.h>
#include <ite/it8951_hv.h>
#include <pl/utils.h>
#define LOG_TAG "it8951_hv"

static int set_vcom(struct pl_vcom_config *p, double vcomInMillivolt);
static int get_vcom(struct pl_vcom_config *p);
static int init_vcom(pl_vcom_config_t *p);

union tps65185_version {
	struct {
		char version :4;
		char minor :2;
		char major :2;
	} v;
	uint8_t byte;
};

//----------------------------------------------------------------------
//Implementation of ITE8951 as interface to PMIC (original interface to tp65185)
//------------------------------------------------------------------------------

pl_hv_driver_t *it8951_get_hv_driver(it8951_t *it8951) {
	assert(it8951 != NULL);

	struct pl_hv_driver *p = hv_driver_new();

	p->hw_ref = it8951;
	p->switch_on = it8951_hv_driver_on;
	p->switch_off = it8951_hv_driver_off;
	p->init = it8951_hv_init;

	return p;
}

static int it8951_hv_init(pl_vcom_config_t *p) {
	assert(p != NULL);

	pl_pmic_t *it8951_pmic = (pl_pmic_t*) p->hw_ref;
	assert(it8951_pmic != NULL);
	it8951_t *it8951 = (it8951_t*) it8951_pmic->hw_ref;
	assert(it8951 != NULL);

	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);

	enum interfaceType* type = it8951->sInterfaceType;

	if (*type == SPI_HRDY) {
		pl_spi_hrdy_t *spi = malloc(sizeof(pl_spi_hrdy_t));
		spi->hw_ref = bus->hw_ref;
		assert(spi != NULL);
	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		assert(i80 != NULL);
	} else {
		//error
	}

	return 0;
}

///**
// * @brief triggers power-on cycle of ITE controller
// * @see pl_hv_driver_t
// * @param p pointer to a hv driver object
// * @return PASS(==0) / FAIL(!=0) information
// */
static int it8951_hv_driver_on(struct pl_hv_driver *p) {

	it8951_t *it8951 = (it8951_t *) p->hw_ref;
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	enum interfaceType *type = it8951->sInterfaceType;
	int iresult = 0;

	IT8951WaitForReady(bus, type);

	if (*type == I80) {
		TWord buf[6];
		buf[0] = IT8951_TCON_BYPASS_I2C;
		buf[1] = 0x01;
		buf[2] = 0x68;
		buf[3] = 0x0A;
		buf[4] = 0x01;
		buf[5] = 0x00;
		IT8951WriteDataBurst(bus, type, buf, 12);
		TWord buf2[2];
		buf2[0] = USDEF_I80_CMD_POWER_CTR;
		buf2[1] = 0x01;
		IT8951WriteDataBurst(bus, type, buf2, 4);

	} else {
		IT8951WaitForReady(bus, type);

//		IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
//		IT8951WriteData(bus, type, 0x01); // I2C write command
//		IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
//		IT8951WriteData(bus, type, 0x09); // Power Up Sequence Register
//		IT8951WriteData(bus, type, 0x01); // Write Size
//		IT8951WriteData(bus, type, 0xC9); //

		IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
		IT8951WriteData(bus, type, 0x01); // I2C write command
		IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
		IT8951WriteData(bus, type, 0x0A); // Power Up Sequence Register
		IT8951WriteData(bus, type, 0x01); // Write Size
		IT8951WriteData(bus, type, 0x00); //
		IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
		IT8951WriteData(bus, type, 0x01);
	}

	return 0;
}

///**
// * @brief triggers power-off cycle of EPSON controller
// * @see pl_hv_driver_t
// * @param p pointer to a hv driver object
// * @return PASS(==0) / FAIL(!=0) information
// */
static int it8951_hv_driver_off(struct pl_hv_driver *p) {

	it8951_t *it8951 = (it8951_t *) p->hw_ref;
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	enum interfaceType *type = it8951->sInterfaceType;
	//printf("HV off managed by ITE \n");

//	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
//	IT8951WriteData(bus, type, 0x01); // I2C write command
//	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
//	IT8951WriteData(bus, type, 0x0C); // Power Up Sequence Register
//	IT8951WriteData(bus, type, 0x01); // Write Size
//	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	//Get current Register setting
	TWord data;
	//IT8951WaitForReady(bus, type);
	data = IT8951ReadReg(bus, type, 0x1e16);

	if (*type == I80) {
		TWord buf[2];
		buf[0] = USDEF_I80_CMD_POWER_CTR;
		buf[1] = 0x00;
		IT8951WriteDataBurst(bus, type, buf, 4);
	} else {
		IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
		IT8951WriteData(bus, type, 0x00);
	}

	//FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
	data &= ~(1 << 12); // switches GPIO5 of ITE (Power Up Pin) low
	//FLIP Bit 11 which corresponds to GPIO11/Pin 65 on ITE to enable VCom_Switch
	data &= ~(1 << 11); // switches GPIO5 of ITE (Power COM Pin) low

	IT8951WriteReg(bus, type, 0x1e16, data);

//	do {
//		data3 = IT8951ReadReg(bus, type, 0x1E14);
//		timeout--;
//	} while ((data3 & 0x20) && timeout);
//
//	if (data3 & 0x20) {
//		LOG("Failed to turn the EPDC power off");
//		return -EEPDC;
//	}
	return 0;
}

// -----------------------------------------------------------------------------
// vcom_config - interface implementation
// ------------------------------
pl_vcom_config_t *it8951_get_vcom_config(pl_pmic_t *it8951_pmic) {
	assert(it8951_pmic != NULL);

	struct pl_vcom_config *p = vcom_config_new();
	p->hw_ref = it8951_pmic;
	p->set_vcom = set_vcom;
	p->get_vcom = get_vcom;
	p->init = init_vcom;
	return p;
}

static int set_vcom(struct pl_vcom_config *p, double vcomInMillivolt) {
	assert(p != NULL);
	pl_pmic_t *it8951_pmic = (pl_pmic_t*) p->hw_ref;
	it8951_t *it8951 = (it8951_t*) it8951_pmic->hw_ref;

	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	enum interfaceType *type = it8951->sInterfaceType;

	//recalculate the VCom value utilizing the PL DAC structure
	int dac_value;
	struct vcom_cal *v = it8951_pmic->cal;
	dac_value = vcom_calculate(v, vcomInMillivolt);
	LOG("calculate: %i, %i", vcomInMillivolt, dac_value);

	if (dac_value < IT8951_HVPMIC_DAC_MIN)
		dac_value = IT8951_HVPMIC_DAC_MIN;
	else if (dac_value > IT8951_HVPMIC_DAC_MAX)
		dac_value = IT8951_HVPMIC_DAC_MAX;

	uint8_t vcomval_[2];
	vcomval_[0] = (uint8_t) dac_value;
	vcomval_[1] = (uint8_t) (dac_value >> 8);

	//set unused Register Bit of TPS65185 to 1, normal reset state
	vcomval_[1] |= (1 << 2);

	if (*type == I80) {
		//usleep(100000);
		IT8951WaitForReady(bus, type);
		TWord buf[3];
		buf[0] = USDEF_I80_CMD_VCOM_CTR;
		buf[1] = 0x01;
		buf[2] = (TWord) dac_value;
		IT8951WriteDataBurst(bus, type, buf, 6);

	}

	else {
		IT8951WaitForReady(bus, type);
		//Configure the VCom Value
		IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_VCOM_CTR);
		IT8951WriteData(bus, type, 0x01); // command parameter for setting the VCOM Value
		IT8951WriteData(bus, type, (TWord) dac_value);
		IT8951WaitForReady(bus, type);
	}

// Unfortunately Configure VCom Command turns PMIC on completely, so we ahve to set it back to standby manually
// Read GPIO/PMIC Registers
// GPIO/PMIC Register is 32 Bit, first 16 Bit are found at address 1E14 next 16bit can be found at next address 1e16
// address 1e16 actually holds the output values

//IT8951WaitForReady(bus, type);
//Get current Register setting
	TWord data2;
	data2 = IT8951ReadReg(bus, type, 0x1e16);

//FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
	data2 &= ~(1 << 12); // switches GPIO5 of ITE (Power Up Pin) low

	if (*type == I80) {
		TWord buf[3];
		buf[0] = USDEF_I80_CMD_POWER_CTR;
		buf[1] = 0x00;
		buf[2] = (TWord) dac_value;
		IT8951WriteDataBurst(bus, type, buf, 6);
	} else {
		IT8951WaitForReady(bus, type);
		IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
		IT8951WriteData(bus, type, 0x00);
		IT8951WaitForReady(bus, type);
	}

	IT8951WriteReg(bus, type, 0x1e16, data2); //-> Power Down ?
//
//	//usleep(8000);
//	//Write adjusted data to register
//	IT8951WriteReg(bus, type, 0x1e16, data); //-> Power Down ?

	return 0;
}

static int get_vcom(struct pl_vcom_config *p) {
	assert(p != NULL);
	pl_pmic_t *it8951_pmic = (pl_pmic_t*) p->hw_ref;
	it8951_t *it8951 = (it8951_t*) it8951_pmic->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	enum interfaceType *type = it8951->sInterfaceType;

//read VCom
	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_VCOM_CTR);
	IT8951WriteData(bus, type, 0x00); // command parameter for reading the VCOM Value
	IT8951WaitForReady(bus, type);
	TWord *result_ = IT8951ReadData(bus, type, 1);
	int value = (int) *result_;
	value = value * 4;
	return value;
}

/**
 * Initializes the underlying it8951 should not be necessary.
 * Just check whether object is available.
 *
 * @param p pointer to an vcom_config implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int init_vcom(pl_vcom_config_t *p) {
	assert(p != NULL);
	pl_pmic_t *it8951_pmic = (pl_pmic_t*) p->hw_ref;
	assert(it8951_pmic != NULL);
	it8951_t *it8951 = (it8951_t*) it8951_pmic->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);

	enum interfaceType* type = it8951->sInterfaceType;

	if (*type == SPI_HRDY) {
		//struct pl_spi_hrdy *spi = (struct pl_spi_hrdy)malloc(sizeof(struct pl_spi_hrdy));
		pl_spi_hrdy_t *spi = malloc(sizeof(pl_spi_hrdy_t));
		spi->hw_ref = bus->hw_ref;
		assert(spi != NULL);
	} else if (*type == I80) {
		pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
		assert(i80 != NULL);
	} else {
		//error
	}

	return 0;
}
