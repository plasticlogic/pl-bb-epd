/*
 * beaglebone-hv.c
 *
 *  Created on: 16 Apr 2015
 *      Author: matti.haugwitz
 */

#include <stdlib.h>
#include <beaglebone/beaglebone-hv.h>
#include <pl/assert.h>
#define LOG_TAG "beaglebone_hv"
#include <pl/utils.h>
#include <pl/gpio.h>
#include <src/pindef.h>
#include <beaglebone/beaglebone-gpio.h>


static void delete(pl_vcom_switch_t *p);
static void open_vcom(pl_vcom_switch_t *p);
static void close_vcom(pl_vcom_switch_t *p);
static void enable_vcom_bypass_mode(pl_vcom_switch_t *p, int switch_state);
static void disable_vcom_bypass_mode(pl_vcom_switch_t *p);

static void delete_hv_driver(pl_hv_driver_t *p);
static int beaglebone_hv_driver_on(struct pl_hv_driver *p);
static int beaglebone_hv_driver_off(struct pl_hv_driver *p);
static int beaglebone_wait_pok(struct pl_gpio *gpio);


/**
 * waits until POK signal becomes high or timeout is reached
 *
 * @param gpio expect pl_gpio structure
 * @return status
 */
static int beaglebone_wait_pok(struct pl_gpio *gpio)
{
	static const unsigned POLL_DELAY_MS = 5;
	unsigned timeout = 100;
	int pok = 0;

	assert(gpio);

	while (!pok) {

		usleep(POLL_DELAY_MS*1000);

		pok = gpio->get(PMIC_POK);

		if (timeout > POLL_DELAY_MS) {
			timeout -= POLL_DELAY_MS;
		} else {
			timeout = 0;

			if (!pok) {
				LOG("POK timeout");
				return -1;
			}
		}
	}

	return 0;
}

// -----------------------------------------------------------------------------
// hv_driver - interface implementation
// ------------------------------
/**
 * allocates a new pl_hv_driver structure and initialises them with beaglebone related settings
 *
 * @param gpio expect pl_gpio structure
 * @return pl_hv_driver structure
 */
pl_hv_driver_t *beaglebone_get_hv_driver(struct pl_gpio *gpio){
	assert(gpio != NULL);

	struct pl_hv_driver *p = hv_driver_new();

	p->hw_ref = gpio;
	p->switch_on = beaglebone_hv_driver_on;
	p->switch_off = beaglebone_hv_driver_off;
	p->init = NULL;
	p->delete = delete_hv_driver;

	return p;
}

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void delete_hv_driver(pl_hv_driver_t *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * enables hv driver, set "HV_EN" bit high
 *
 * @param p expects the pl_hv_driver structure
 */
static int beaglebone_hv_driver_on(struct pl_hv_driver *p){
	if (p == NULL) return -1;

	struct pl_gpio *gpio = (struct pl_gpio *)p->hw_ref;
	pl_gpio_set(gpio, PMIC_EN, 1);

	beaglebone_wait_pok(gpio);

	return 0;
}

/**
 * disables hv driver, set "HV_EN" bit low
 *
 * @param p expects the pl_hv_driver structure
 */
static int beaglebone_hv_driver_off(struct pl_hv_driver *p){
	if (p == NULL) return -1;

	struct pl_gpio *gpio = (struct pl_gpio *)p->hw_ref;
	pl_gpio_set(gpio, PMIC_EN, 0);
	return 0;
}


// -----------------------------------------------------------------------------
// vcom_switch - interface implementation
// ------------------------------

/**
 * allocates a new pl_vcom_switch structure and initialises them with beaglebone related settings
 *
 * @param gpio expect pl_gpio structure due to switch is controlled via gpio
 * @return pl_vcom_switch structure
 */
pl_vcom_switch_t *beaglebone_get_vcom_switch(struct pl_gpio *gpio){
	assert(gpio != NULL);

	struct pl_vcom_switch *p = vcom_switch_new();
	p->hw_ref = gpio;
	p->is_bypass = 0;

	p->delete = &delete;
	p->open = &open_vcom;
	p->close = &close_vcom;
	p->enable_bypass_mode = &enable_vcom_bypass_mode;
	p->disable_bypass_mode = &disable_vcom_bypass_mode;

	p->init = NULL;

	return p;
}

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void delete(pl_vcom_switch_t *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}

/**
 * opens the vcom switch
 *
 * @param p expects the pl_vcom_switch structure
 */
static void open_vcom(pl_vcom_switch_t *p)
{
	assert(p != NULL);

	struct pl_gpio *gpio = p->hw_ref;
	pl_gpio_set(gpio, VCOM_EN, 0);
}

/**
 * closes the vcom switch
 *
 * @param p expects the pl_vcom_switch structure
 */
static void close_vcom(pl_vcom_switch_t *p)
{
	assert(p != NULL);

	struct pl_gpio *gpio = p->hw_ref;
	pl_gpio_set(gpio, VCOM_EN, 1);
}

/**
 * Enables the vcom bypass mode, means "open"/"close" calls have no effect
 *
 * @param p expects the pl_vcom_switch structure
 * @param switch_state state of the vcom switch during bypass mode is active: 0->open, 1->closed
 */
static void enable_vcom_bypass_mode(pl_vcom_switch_t *p, int switch_state)
{
	assert(p != NULL);

	p->is_bypass = 0;

	if(switch_state)
	{
		close_vcom(p);
	}
	else
	{
		open_vcom(p);
	}

	p->is_bypass = 1;
}


/**
 * Disables the vcom bypass mode and sets the vcom switch to OPEN
 *
 * @param p expects the pl_vcom_switch structure
 */
static void disable_vcom_bypass_mode(pl_vcom_switch_t *p)
{
	assert(p != NULL);

	p->is_bypass = 0;

	open_vcom(p);
}

