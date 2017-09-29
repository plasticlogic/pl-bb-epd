/*
 * s1d135xx_hv.c
 *
 *  Created on: 29.04.2015
 *      Author: sebastian.friebe
 */

#include "pl/assert.h"
#include "s1d135xx_hv.h"

static int s1d135xx_hv_driver_on(struct pl_hv_driver *p);
static int s1d135xx_hv_driver_off(struct pl_hv_driver *p);

static int s1d135xx_vcom_driver_on(pl_vcom_driver_t *p);
static int s1d135xx_vcom_driver_off(pl_vcom_driver_t *p);

static void s1d135xx_vcom_switch_enable_bypass(struct pl_vcom_switch *p, int switch_state);
static void s1d135xx_vcom_switch_disable_bypass(struct pl_vcom_switch *p);

// -----------------------------------------------------------------------------
// hv_driver - interface implementation
// ------------------------------
pl_hv_driver_t *s1d135xx_get_hv_driver(s1d135xx_t *s1d135xx){
	assert(s1d135xx != NULL);

	struct pl_hv_driver *p = hv_driver_new();

	p->hw_ref = s1d135xx;
	p->switch_on = s1d135xx_hv_driver_on;
	p->switch_off = s1d135xx_hv_driver_off;
	p->init = NULL;

	return p;
}

/**
 * @brief triggers power-on cycle of EPSON controller
 * @see pl_hv_driver_t
 * @param p pointer to a hv driver object
 * @return PASS(==0) / FAIL(!=0) information
 */
static int s1d135xx_hv_driver_on(struct pl_hv_driver *p){
	if (p == NULL) return -1;

	s1d135xx_t *s1d135xx = (s1d135xx_t *)p->hw_ref;
	return s1d135xx->set_epd_power(s1d135xx, 1);

}

/**
 * @brief triggers power-off cycle of EPSON controller
 * @see pl_hv_driver_t
 * @param p pointer to a hv driver object
 * @return PASS(==0) / FAIL(!=0) information
 */
static int s1d135xx_hv_driver_off(struct pl_hv_driver *p){
	if (p == NULL) return -1;

	s1d135xx_t *s1d135xx = (s1d135xx_t *)p->hw_ref;
	return s1d135xx->set_epd_power(s1d135xx, 0);
}


// -----------------------------------------------------------------------------
// vcom_driver - interface implementation
// ------------------------------
pl_vcom_driver_t *s1d135xx_get_vcom_driver(s1d135xx_t *s1d135xx){
	assert(s1d135xx != NULL);

	pl_vcom_driver_t *p = vcom_driver_new();
	p->hw_ref = s1d135xx;
	p->switch_on = s1d135xx_vcom_driver_on;
	p->switch_off = s1d135xx_vcom_driver_off;
	p->init = NULL;

	return p;
}

/**
 * @brief triggers power-on cycle of EPSON controller
 * @see pl_vcom_driver_t
 * @param p pointer to a vcom driver object
 * @return PASS(==0) / FAIL(!=0) information
 */
static int s1d135xx_vcom_driver_on(pl_vcom_driver_t *p){
	if (p == NULL) return -1;

	s1d135xx_t *s1d135xx = (s1d135xx_t *)p->hw_ref;
	return s1d135xx->set_epd_power(s1d135xx, 1);
}

/**
 * @brief triggers power-off cycle of EPSON controller
 * @see pl_vcom_driver_t
 * @param p pointer to a vcom driver object
 * @return PASS(==0) / FAIL(!=0) information
 */
static int s1d135xx_vcom_driver_off(pl_vcom_driver_t *p){
	if (p == NULL) return -1;

	s1d135xx_t *s1d135xx = (s1d135xx_t *)p->hw_ref;
	return s1d135xx->set_epd_power(s1d135xx, 0);
}



// -----------------------------------------------------------------------------
// vcom_switch - interface implementation
// ------------------------------
pl_vcom_switch_t *s1d135xx_get_vcom_switch(s1d135xx_t *s1d135xx){
	assert(s1d135xx != NULL);

	pl_vcom_switch_t *p = vcom_switch_new();
	p->hw_ref = s1d135xx;
	p->close = NULL;
	p->open = NULL;
	p->disable_bypass_mode = s1d135xx_vcom_switch_disable_bypass;
	p->enable_bypass_mode = s1d135xx_vcom_switch_enable_bypass;
	p->init = NULL;

	return p;
}


static void s1d135xx_vcom_switch_enable_bypass(struct pl_vcom_switch *p, int switch_state){
	s1d135xx_t *s1d135xx = (s1d135xx_t *)p->hw_ref;

	s1d135xx->update_reg(s1d135xx, 0x232, 0x0100 | (switch_state & 0x0001), 0x0101);
}
static void s1d135xx_vcom_switch_disable_bypass(struct pl_vcom_switch *p){
	s1d135xx_t *s1d135xx = (s1d135xx_t *)p->hw_ref;

	s1d135xx->update_reg(s1d135xx, 0x232, 0x0000, 0x0100);

}
