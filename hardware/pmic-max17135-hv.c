/*
 * pmic-max17135-hv.c
 *
 *  Created on: 24.03.2015
 *      Author: sebastian.friebe
 */


#include <stdlib.h>
#include <hardware/pmic-max17135-hv.h>
#include <pl/assert.h>
#define LOG_TAG "max17135_hv"
#include <pl/utils.h>

static int max17135_hv_driver_on(struct pl_hv_driver *p);
static int max17135_hv_driver_off(struct pl_hv_driver *p);
static void set_vsh(struct pl_hv_config *p, double valueInVolts);
static void set_vsl(struct pl_hv_config *p, double valueInVolts);
static int set_hv_timings(struct pl_hv_timing *p);
static int vcom_driver_on(pl_vcom_driver_t *p);
static int max17135_vcom_config_set(struct pl_vcom_config *p, double vcomInMillivolt);
static int max17135_vcom_config_get(struct pl_vcom_config *p);
static int vcom_driver_off(pl_vcom_driver_t *p);
static int hv_driver_init(pl_hv_driver_t *p);
static int hv_config_init(pl_hv_config_t *p);
static int hv_timing_init(pl_hv_timing_t *p);
static int vcom_driver_init(pl_vcom_driver_t *p);
static int vcom_config_init(pl_vcom_config_t *p);

// -----------------------------------------------------------------------------
// hv_driver - interface implementation
// ------------------------------
pl_hv_driver_t *max17135_get_hv_driver(pl_pmic_t *max17135){
	assert(max17135 != NULL);

	struct pl_hv_driver *p = hv_driver_new();

	p->hw_ref = max17135;
	p->switch_on = max17135_hv_driver_on;
	p->switch_off = max17135_hv_driver_off;
	p->init = hv_driver_init;

	return p;
}

/**
 * Powers up the high voltages on the underlying Maxim MAX17135 hardware.
 * Waits until the power OK signal is set.
 *
 * @param p pointer to an hv_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int max17135_hv_driver_on(struct pl_hv_driver *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	int stat = max17135->hv_enable(max17135);
	stat |= max17135->wait_pok(max17135);

	return stat;
}

/**
 * Powers down the high voltages on the underlying Maxim MAX17135 hardware.
 *
 * @param p pointer to an hv_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int max17135_hv_driver_off(struct pl_hv_driver *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->hv_disable(max17135);
}

/**
 * Initializes the underlying Maxim MAX17135 hardware.
 *
 * @param p pointer to an hv_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int hv_driver_init(pl_hv_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->init(max17135);
}

// -----------------------------------------------------------------------------
// hv_config - interface implementation
// ------------------------------
pl_hv_config_t *max17135_get_hv_config(pl_pmic_t *max17135){
	assert(max17135 != NULL);

	pl_hv_config_t *p = hv_config_new();

	p->hw_ref = max17135;
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
 * @param p pointer to a max17135 implementation
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
 * @param p pointer to a max17135 implementation
 * @param valueInVolts new target voltage for VSL
 * @todo implement functionality and change return type to int, to mark possible errors
 */
static void set_vsl(struct pl_hv_config *p, double valueInVolts){
	assert(p != NULL);

	LOG("set_vsl - not yet implemented.");
}

/**
 * Initializes the underlying Maxim MAX17135 hardware.
 *
 * @param p pointer to an hv_config implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int hv_config_init(pl_hv_config_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);
	return max17135->init(max17135);
}
// -----------------------------------------------------------------------------
// hv_timing - interface implementation
// ------------------------------
pl_hv_timing_t *max17135_get_hv_timing(pl_pmic_t *max17135){
	assert(max17135 != NULL);

	pl_hv_timing_t *p = hv_timing_new();

	p->hw_ref = max17135;
	p->set_timings = set_hv_timings;
	p->init = hv_timing_init;
	return p;
}


/**
 * Updates the pmic timings of the underlying
 * Maxim MAX17135 PMIC hardware with the internally stored timings.
 * After that, the timings will be applied to the real hardware.
 *
 * @param p pointer to the pl_hv_timing object
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int set_hv_timings(struct pl_hv_timing *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	max17135->vgh_off_offset_time = p->toffset_vgh_off;
	max17135->vgh_on_offset_time = p->toffset_vgh_on;
	max17135->vgl_off_offset_time = p->toffset_vgl_off;
	max17135->vgl_on_offset_time = p->toffset_vgl_on;
	max17135->vsh_off_offset_time = p->toffset_vsh_off;
	max17135->vsh_on_offset_time = p->toffset_vsh_on;
	max17135->vsl_off_offset_time = p->toffset_vsl_off;
	max17135->vsl_on_offset_time = p->toffset_vsl_on;

	return max17135->apply_timings(max17135);
}

/**
 * Initializes the underlying Maxim MAX17135 hardware.
 * Applies the stored timings to the PMIC.
 *
 * @param p pointer to an hv_timing implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int hv_timing_init(pl_hv_timing_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	int stat = 0;
	stat |= max17135->init(max17135);
	stat |= set_hv_timings(p);
	return stat;
}
// -----------------------------------------------------------------------------
// vcom_config - interface implementation
// ------------------------------
pl_vcom_config_t *max17135_get_vcom_config(pl_pmic_t *max17135){
	assert(max17135 != NULL);

	struct pl_vcom_config *p = vcom_config_new();
	p->hw_ref = max17135;
	p->set_vcom = max17135_vcom_config_set;
	p->get_vcom = max17135_vcom_config_get;
	p->init = vcom_config_init;
	return p;
}

static int max17135_vcom_config_set(struct pl_vcom_config *p, double vcomInMillivolt){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->set_vcom_voltage(max17135, (int)vcomInMillivolt);
}

static int max17135_vcom_config_get(struct pl_vcom_config *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->get_vcom_voltage(max17135);
}
/**
 * Initializes the underlying Maxim MAX17135 hardware.
 *
 * @param p pointer to an vcom_config implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int vcom_config_init(pl_vcom_config_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->init(max17135);
}
// -----------------------------------------------------------------------------
// vcom_driver - interface implementation
// ------------------------------
pl_vcom_driver_t *max17135_get_vcom_driver(pl_pmic_t *max17135){
	assert(max17135 != NULL);

	pl_vcom_driver_t *p = vcom_driver_new();
	p->hw_ref = max17135;
	p->switch_on = vcom_driver_on;
	p->switch_off = vcom_driver_off;
	p->init = vcom_driver_init;

	return p;
}

static int vcom_driver_on(pl_vcom_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->vcom_enable(max17135);
}

static int vcom_driver_off(pl_vcom_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->vcom_disable(max17135);
}

/**
 * Initializes the underlying Maxim MAX17135 hardware.
 *
 * @param p pointer to an vcom_driver implementation
 * @return success of operation: 0 if passed; otherwise != 0
 */
static int vcom_driver_init(pl_vcom_driver_t *p){
	assert(p != NULL);
	pl_pmic_t *max17135 = (pl_pmic_t*)p->hw_ref;
	assert(max17135 != NULL);

	return max17135->init(max17135);
}
