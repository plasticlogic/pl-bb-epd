/*
 * pmic-tps65185-hv.c
 *
 *  Created on: 16.04.2015
 *      Author: sebastian.friebe
 */

#include "hardware/pmic-tps65185-hv.h"

#include <stdlib.h>
#include <hardware/pmic-tps65185-hv.h>
#include <pl/assert.h>
#define LOG_TAG "tps65185_hv"
#include <pl/utils.h>

static int tps65185_hv_driver_on(struct pl_hv_driver *p);
static int tps65185_hv_driver_off(struct pl_hv_driver *p);
static void set_vsh(struct pl_hv_config *p, double valueInVolts);
static void set_vsl(struct pl_hv_config *p, double valueInVolts);
static int set_hv_timings(struct pl_hv_timing *p);
static int vcom_driver_on(pl_vcom_driver_t *p);
static int tps65185_vcom_config_set(struct pl_vcom_config *p, double vcomInMillivolt);
static int vcom_driver_off(pl_vcom_driver_t *p);
static int hv_driver_init(pl_hv_driver_t *p);
static int hv_config_init(pl_hv_config_t *p);
static int hv_timing_init(pl_hv_timing_t *p);
static int vcom_driver_init(pl_vcom_driver_t *p);
static int vcom_config_init(pl_vcom_config_t *p);
static int vcom_switch_init(pl_vcom_switch_t *p);
static void vcom_switch_open(pl_vcom_switch_t *p);
static void vcom_switch_close(pl_vcom_switch_t *p);
static void vcom_switch_disable_bypass(pl_vcom_switch_t *p);
static void vcom_switch_enable_bypass(pl_vcom_switch_t *p, int switch_state);

// -----------------------------------------------------------------------------
// hv_driver - interface implementation
// ------------------------------
pl_hv_driver_t *tps65185_get_hv_driver(pl_pmic_t *tps65185){
	assert(tps65185 != NULL);

	struct pl_hv_driver *p = hv_driver_new();

	p->hw_ref = tps65185;
	p->switch_on = tps65185_hv_driver_on;
	p->switch_off = tps65185_hv_driver_off;
	p->init = hv_driver_init;

	return p;
}

/**
 * Powers up the high voltages on the underlying Texas Instruments TPS65185 hardware.
 * Waits until the power OK signal is set.
 *
 * @param p pointer to an hv_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int tps65185_hv_driver_on(struct pl_hv_driver *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	int stat = tps65185->hv_enable(tps65185);
	stat |= tps65185->wait_pok(tps65185);

	return stat;
}

/**
 * Powers down the high voltages on the underlying Texas Instruments TPS65185 hardware.
 *
 * @param p pointer to an hv_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int tps65185_hv_driver_off(struct pl_hv_driver *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->hv_disable(tps65185);
}

/**
 * Initializes the underlying Texas Instruments TPS65185 hardware.
 *
 * @param p pointer to an hv_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int hv_driver_init(pl_hv_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->init(tps65185);
}

// -----------------------------------------------------------------------------
// hv_config - interface implementation
// ------------------------------
pl_hv_config_t *tps65185_get_hv_config(pl_pmic_t *tps65185){
	assert(tps65185 != NULL);

	pl_hv_config_t *p = hv_config_new();

	p->hw_ref = tps65185;
	p->set_vgh = NULL;		// not applicable, since vGate is defined by external circuit
	p->set_vgl = NULL;		// not applicable, since vGate is defined by external circuit
	p->set_vsh = set_vsh;
	p->set_vsl = set_vsl;
	p->init = hv_config_init;
	return p;
}

/**
 * Sets the target positive source voltage to the underlying pmic hardware.
 *
 * @param p pointer to a tps65185 implementation
 * @param valueInVolts new target voltage for VSH
 * @todo implement functionality and change return type to int, to mark possible errors
 */
static void set_vsh(struct pl_hv_config *p, double valueInVolts){
	assert(p != NULL);

	LOG("set_vsh - not yet implemented.");
}

/**
 * Sets the target negative source voltage to the underlying pmic hardware.
 *
 * @param p pointer to a tps65185 implementation
 * @param valueInVolts new target voltage for VSL
 * @todo implement functionality and change return type to int, to mark possible errors
 */
static void set_vsl(struct pl_hv_config *p, double valueInVolts){
	assert(p != NULL);

	LOG("set_vsl - not yet implemented.");
}

/**
 * Initializes the underlying Texas Instruments TPS65185 hardware.
 *
 * @param p pointer to an hv_config implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int hv_config_init(pl_hv_config_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->init(tps65185);
}
// -----------------------------------------------------------------------------
// hv_timing - interface implementation
// ------------------------------
pl_hv_timing_t *tps65185_get_hv_timing(pl_pmic_t *tps65185){
	assert(tps65185 != NULL);

	pl_hv_timing_t *p = hv_timing_new();

	p->hw_ref = tps65185;
	p->set_timings = set_hv_timings;
	p->init = hv_timing_init;
	return p;
}


/**
 * Updates the pmic timings of the underlying
 * Texas Instruments TPS65185 PMIC hardware with the internally stored timings.
 * After that, the timings will be applied to the real hardware.
 *
 * @param p pointer to the pl_hv_timing object
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int set_hv_timings(struct pl_hv_timing *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	tps65185->vgh_off_offset_time = p->toffset_vgh_off;
	tps65185->vgh_on_offset_time = p->toffset_vgh_on;
	tps65185->vgl_off_offset_time = p->toffset_vgl_off;
	tps65185->vgl_on_offset_time = p->toffset_vgl_on;
	tps65185->vsh_off_offset_time = p->toffset_vsh_off;
	tps65185->vsh_on_offset_time = p->toffset_vsh_on;
	tps65185->vsl_off_offset_time = p->toffset_vsl_off;
	tps65185->vsl_on_offset_time = p->toffset_vsl_on;

	return tps65185->apply_timings(tps65185);
}

/**
 * Initializes the underlying Texas Instruments TPS65185 hardware.
 * Applies the stored timings to the PMIC.
 *
 * @param p pointer to an hv_timing implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int hv_timing_init(pl_hv_timing_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	int stat = 0;
	stat |= tps65185->init(tps65185);
	stat |= set_hv_timings(p);
	return stat;
}
// -----------------------------------------------------------------------------
// vcom_config - interface implementation
// ------------------------------
pl_vcom_config_t *tps65185_get_vcom_config(pl_pmic_t *tps65185){
	assert(tps65185 != NULL);

	struct pl_vcom_config *p = vcom_config_new();
	p->hw_ref = tps65185;
	p->set_vcom = tps65185_vcom_config_set;
	p->init = vcom_config_init;
	return p;
}

static int tps65185_vcom_config_set(struct pl_vcom_config *p, double vcomInMillivolt){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->set_vcom_voltage(tps65185, (int)vcomInMillivolt);
}

/**
 * Initializes the underlying Texas Instruments TPS65185 hardware.
 *
 * @param p pointer to an vcom_config implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int vcom_config_init(pl_vcom_config_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->init(tps65185);
}
// -----------------------------------------------------------------------------
// vcom_driver - interface implementation
// ------------------------------
pl_vcom_driver_t *tps65185_get_vcom_driver(pl_pmic_t *tps65185){
	assert(tps65185 != NULL);

	pl_vcom_driver_t *p = vcom_driver_new();
	p->hw_ref = tps65185;
	p->switch_on = vcom_driver_on;
	p->switch_off = vcom_driver_off;
	p->init = vcom_driver_init;

	return p;
}

static int vcom_driver_on(pl_vcom_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->vcom_enable(tps65185);
}

static int vcom_driver_off(pl_vcom_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->vcom_disable(tps65185);
}

/**
 * Initializes the underlying Texas Instruments TPS65185 hardware.
 *
 * @param p pointer to an vcom_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int vcom_driver_init(pl_vcom_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->init(tps65185);
}

// -----------------------------------------------------------------------------
// vcom_switch - interface implementation
// ------------------------------
pl_vcom_switch_t *tps65185_get_vcom_switch(pl_pmic_t *tps65185){
	assert(tps65185 != NULL);

	pl_vcom_switch_t *p = vcom_switch_new();
	p->hw_ref = tps65185;
	p->close = vcom_switch_close;
	p->open = vcom_switch_open;
	p->disable_bypass_mode = vcom_switch_disable_bypass;
	p->enable_bypass_mode = vcom_switch_enable_bypass;
	p->init = vcom_switch_init;

	return p;
}

/**
 * Initializes the underlying Texas Instruments TPS65185 hardware.
 *
 * @param p pointer to an vcom_switch implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int vcom_switch_init(pl_vcom_switch_t *p){
	assert(p != NULL);
	pl_pmic_t *tps65185 = (pl_pmic_t*)p->hw_ref;
	assert(tps65185 != NULL);

	return tps65185->init(tps65185);
}


static void vcom_switch_open(pl_vcom_switch_t *p){
	assert(p != NULL);

	LOG("vcom_switch_open - not yet implemented.");
}

static void vcom_switch_close(pl_vcom_switch_t *p){
	assert(p != NULL);

	LOG("vcom_switch_close - not yet implemented.");
}

static void vcom_switch_disable_bypass(pl_vcom_switch_t *p){
	assert(p != NULL);

	LOG("vcom_switch_disable_bypass - not yet implemented.");
}

static void vcom_switch_enable_bypass(pl_vcom_switch_t *p, int switch_state){
	assert(p != NULL);

	LOG("vcom_switch_enable_bypass - not yet implemented.");
}
