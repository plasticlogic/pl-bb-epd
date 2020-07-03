/*
 * it8951_hv.c
 *
 *  Created on: 25 Jun 2020
 *      Author: matti.haugwitz
 */

#include <unistd.h>

#include <pl/hv.h>
#include <pl/generic_interface.h>
#include <pl/i80.h>
#include <pl/gpio.h>
#include <pl/assert.h>
#include <ite/it8951.h>
#include <ite/it8951_hv.h>
#define LOG_TAG "it8951_hv"

static int set_vcom(struct pl_vcom_config *p, double vcomInMillivolt);
static int get_vcom(struct pl_vcom_config *p);
static int init_vcom(pl_vcom_config_t *p);


//----------------------------------------------------------------------
//Implementation of ITE8951 as interface to PMIC (original interface to tp65185)
//------------------------------------------------------------------------------

//pl_hv_driver_t *it8951_get_hv_driver(it8951_t *it8951){
//	assert(it8951 != NULL);
//
//	struct pl_hv_driver *p = hv_driver_new();
//
//	p->hw_ref = it8951;
//	p->switch_on = it8951_hv_driver_on;
//	p->switch_off = it8951_hv_driver_off;
//	p->init = NULL;
//
//	return p;
//}
//
///**
// * @brief triggers power-on cycle of ITE controller
// * @see pl_hv_driver_t
// * @param p pointer to a hv driver object
// * @return PASS(==0) / FAIL(!=0) information
// */
//static int it8951_hv_driver_on(struct pl_hv_driver *p){
//	if (p == NULL) return -EINVAL;
//	it8951_t *it8951 = (it8951_t *)p->hw_ref;
//	return it8951->set_epd_power(it8951, 1);
//
//}
//
///**
// * @brief triggers power-off cycle of EPSON controller
// * @see pl_hv_driver_t
// * @param p pointer to a hv driver object
// * @return PASS(==0) / FAIL(!=0) information
// */
//static int it8951_hv_driver_off(struct pl_hv_driver *p){
//	if (p == NULL) return -EINVAL;
//	it8951_t *it8951 = (it8951_t *)p->hw_ref;
//	return it8951->set_epd_power(it8951, 0);
//}

// -----------------------------------------------------------------------------
// vcom_config - interface implementation
// ------------------------------
pl_vcom_config_t *it8951_get_vcom_config(it8951_t *it8951){
	assert(it8951 != NULL);

	struct pl_vcom_config *p = vcom_config_new();
	p->hw_ref = it8951;
	p->set_vcom = set_vcom;
	p->get_vcom = get_vcom;
	p->init = init_vcom;
	return p;
}

static int set_vcom(struct pl_vcom_config *p, double vcomInMillivolt){
	assert(p != NULL);
	it8951_t *it8951 = (it8951_t*)p->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
	assert(i80 != NULL);

	//Configure the VCom Value
	IT8951WriteCmdCode(i80, USDEF_I80_CMD_VCOM_CTR);
	IT8951WriteData(i80, 0x01); // write VCOM
	IT8951WriteData(i80, (TWord) vcomInMillivolt);
	IT8951WaitForReady(i80);

	//sleep(10);
	//IT8951WriteCmdCode(i80, IT8951_TCON_STANDBY); //
	return 0;
}

static int get_vcom(struct pl_vcom_config *p){
	assert(p != NULL);
	it8951_t *it8951 = (it8951_t*)p->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
	assert(i80 != NULL);

	IT8951WriteCmdCode(i80, USDEF_I80_CMD_VCOM_CTR);
	IT8951WriteData(i80, 0x00); // read VCOM
	int value = (int) IT8951ReadData(i80);

	return value;
}

/**
 * Initializes the underlying it8951 should not be necessary.
 * Just check whether object is available.
 *
 * @param p pointer to an vcom_config implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int init_vcom(pl_vcom_config_t *p){
	assert(p != NULL);
	it8951_t *it8951 = (it8951_t*)p->hw_ref;
	assert(it8951 != NULL);
	pl_generic_interface_t *bus = it8951->interface;
	assert(bus != NULL);
	pl_i80_t *i80 = (pl_i80_t*) bus->hw_ref;
	assert(i80 != NULL);

	return 0;
}
