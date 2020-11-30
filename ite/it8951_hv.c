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

	//printf("HV start managed by ITE \n");

//	if (IT8951WaitForReady(bus, type))
//		return -ETIME;

	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
	IT8951WriteData(bus, type, 0x01);
	IT8951WaitForReady(bus, type);

//Get current Register setting
	TWord data;
	data = IT8951ReadReg(bus, type, 0x1e16);

	//FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
	data |= (1 << 12); // switches GPIO5 of ITE (Power Up Pin) high

	//Write adjusted data to register
	IT8951WriteReg(bus, type, 0x1e16, data);

//	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
//	IT8951WriteData(bus, type, 0x01); // I2C write command
//	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
//	IT8951WriteData(bus, type, 0x01); // Power Up Sequence Register
//	IT8951WriteData(bus, type, 0x01); // Write Size
//	IT8951WriteData(bus, type, 0x2F); //
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
//
//	IT8951WriteData(bus, type, 0x01);
//	IT8951WaitForReady(bus, type);
//
//	//Poll the HV Good Pin on TI TPS65185, to wait for HV ready
//	uint16_t tmp;
//
//	int test;
//	do {
//		tmp = IT8951ReadReg(bus, type, 0x1e16);
//		usleep(250);
//		test = (tmp & 0x20);
//	} while (test != 0x20);
//
//	if ((tmp & 0x20) != 0x20) {
//		LOG("Failed to turn the EPDC power on");
//		return -EEPDC;
//	}

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

	if (IT8951WaitForReady(bus, type))
		return -ETIME;

	//Get current Register setting
	TWord data;
	data = IT8951ReadReg(bus, type, 0x1e16);

	//FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
	data &= ~(1 << 12); // switches GPIO5 of ITE (Power Up Pin) low
	//FLIP Bit 11 which corresponds to GPIO11/Pin 65 on ITE to enable VCom_Switch
	data &= ~(1 << 11); // switches GPIO5 of ITE (Power COM Pin) low

	//data &= ~(1 << 10); // wakeup Pin ---> faster when siwtched for multiple updates ?

	//data &= ~(1 << 13);

	IT8951WriteReg(bus, type, 0x1e16, data);

//	IT8951WaitForReady(bus, type);
//
//	IT8951WriteCmdCode(bus, type, IT8951_TCON_STANDBY);
//
//	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
//	IT8951WriteData(bus, type, 0x00); // I2C write command
//	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
//	IT8951WriteData(bus, type, 0x01); // Power Up Sequence Register
//	IT8951WriteData(bus, type, 0x01); // Write Size
//
//	TWord pmicEnableReg = IT8951ReadData(bus, type, 1); // Register Content (Maximal length for power Up Sequence) 27 6C -- 78
//
//	pmicEnableReg &= ~(1 << 6);
//
//	IT8951WaitForReady(bus, type);
//
//	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
//	IT8951WriteData(bus, type, 0x01); // I2C write command
//	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
//	IT8951WriteData(bus, type, 0x01); // Power Up Sequence Register
//	IT8951WriteData(bus, type, 0x01); // Write Size
//	IT8951WriteData(bus, type, pmicEnableReg);


//	IT8951WaitForReady(bus, type);
//
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
//	IT8951WriteData(bus, type, 0x00);

//
//	LOG("Try to turn the EPDC power off");
//
//	IT8951WaitForDisplayReady(bus, type);
//

//
//	//Write adjusted data to register
//
//	TWord data2;
//	int timeout = 2000;
//	data2 = IT8951ReadReg(bus, type, 0x1e16);
//
//	//FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
//	data2 &= ~(1 << 12); // switches GPIO5 of ITE (Power Up Pin) low
//
//	IT8951WriteReg(bus, type, 0x1e16, data2); //-> Power Down ?

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

	//Make sure TIs TPS65185 is not in sleep, but standby mode (so it can receive i2C command)
	//Get current Register setting of it8951 PMIC GPIOS
	//TWord data;
	//data = IT8951ReadReg(bus, type, 0x1e16);

	//FLIP Bit 10 which corresponds to GPIO10/Pin 64 on ITE
	//data |= (1 << 10); // switches GPIO10 of ITE (Wake Up Pin) high

//	//Write adjusted data to register
//	//IT8951WriteReg(bus, type, 0x1e16, data);// -> Power up ????
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
//	//usleep(8000);
//	IT8951WriteData(bus, type, 0x01);
//	IT8951WaitForReady(bus, type);

	//TODO: Not Working, WHYYYYY ???????
//	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
//	IT8951WriteData(bus, type, 0x00);
//	IT8951WaitForReady(bus, type);
//
//	union tps65185_version ver;
//	uint8_t dataTest;
//
//	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
//	usleep(8000);
//	IT8951WriteData(bus, type, 0x00); // I2C write command
//	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address0
//	IT8951WriteData(bus, type, 0x10); // Power Up Sequence Register
//	IT8951WriteData(bus, type, 0x01); // Write Size
//	usleep(8000);
//	//ver.byte = (int)IT8951ReadData(bus, type, 1);  //read data
//	ver.byte = (int)IT8951ReadData(bus, type, 2);  //read data
//
//	printf("Version: %d.%d.%d\n", ver.v.major, ver.v.minor, ver.v.version);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x01); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x02); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x03); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x03); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x04); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x05); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x06); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	// Set Power Up Sequence
	// Send I2C Command via ITE8951
	//LOG("Im here");
	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x09); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x78); // Register Content (Maximal length for power Up Sequence) 27 6C -- 78
////
////	TWord* usData1[] = { 0x01, 0x68, 0x09, 0x01, 0x78 };
////	IT8951WriteDataBurst(bus, type, usData1, 5);
	IT8951WaitForReady(bus, type);
//
//	// Set Power Up Sequence Timing
	// Send I2C Command via ITE8951
	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0A); // Power Up Timing Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); // Register Content (Maximal length for power Up Sequence) there was an ff in there for longer power up time ???????

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0B); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0C); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0D); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x00); //

	IT8951WaitForReady(bus, type);

	IT8951WriteCmdCode(bus, type, IT8951_TCON_BYPASS_I2C);
	IT8951WriteData(bus, type, 0x01); // I2C write command
	IT8951WriteData(bus, type, 0x68); // TPS65815 Chip Address
	IT8951WriteData(bus, type, 0x0E); // Power Up Sequence Register
	IT8951WriteData(bus, type, 0x01); // Write Size
	IT8951WriteData(bus, type, 0x78); //

	IT8951WaitForReady(bus, type);

	//Configure the VCom Value
	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_VCOM_CTR);
	IT8951WriteData(bus, type, 0x01); // command parameter for setting the VCOM Value
	IT8951WriteData(bus, type, (TWord) dac_value);
	IT8951WaitForReady(bus, type);

	// Unfortunately Configure VCom Command turns PMIC on completely, so we ahve to set it back to standby manually
	// Read GPIO/PMIC Registers
	// GPIO/PMIC Register is 32 Bit, first 16 Bit are found at address 1E14 next 16bit can be found at next address 1e16
	// address 1e16 actually holds the output values

	//Get current Register setting
	TWord data2;
	data2 = IT8951ReadReg(bus, type, 0x1e16);

	//FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
	data2 &= ~(1 << 12); // switches GPIO5 of ITE (Power Up Pin) low

	IT8951WriteCmdCode(bus, type, USDEF_I80_CMD_POWER_CTR);
	IT8951WriteData(bus, type, 0x00);
	IT8951WaitForReady(bus, type);
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
