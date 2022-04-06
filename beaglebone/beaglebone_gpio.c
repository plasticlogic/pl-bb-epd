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
 * beaglebone-gpio.c
 *
 *  Created on: 21.07.2014
 *      Author: sebastian.friebe, matti.haugwitz
 */

#include <beaglebone/beaglebone_gpio.h>
#define LOG_TAG "beaglebone-gpio"
#include <pl/utils.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define GPIO_PORT(_gpio) (((_gpio) >> 8) & 0xFF)
#define GPIO_PIN(_gpio) ((_gpio) & 0xFF)

static int beaglebone_gpio_export(unsigned gpio, uint16_t flags);
static int beaglebone_gpio_unexport(unsigned gpio);
static int beaglebone_gpio_get(unsigned gpio);
static void beaglebone_gpio_set(unsigned gpio, int value);

/**
 * initialises gpio structure which is part of the hw_setup structure
 *
 * @param gpio pl_gpio structure
 * @return returns 0
 */
int beaglebone_gpio_init(struct pl_gpio *gpio)
{
	gpio->config = beaglebone_gpio_export;
	gpio->deconfigure = beaglebone_gpio_unexport;
	gpio->get = beaglebone_gpio_get;
	gpio->set = beaglebone_gpio_set;

	return 0;
}

/**
 * export and configure the beaglebone gpios (direction, initial value, ...).
 *
 * @param gpio gpio number.
 * @param flags configuration flags defined by enum pl_gpio_flags.
 * @return status.
 */
static int beaglebone_gpio_export(unsigned gpio, uint16_t flags)
{

#if PL_GPIO_DEBUG
	LOG("config gpio=0x%04X (%d.%d)", gpio, (port + 1),
	pl_gpio_log_flags(flags);
#endif


#if GPIO_CHECK_PARAMETERS
	if (pl_gpio_check_flags(flags))
		return -EINVAL;
#endif

	static const char GPIO_SYSFS[] = "/sys/class/gpio";
		static const size_t BUFFER_SIZE = 60;
		char buf[BUFFER_SIZE];
		FILE *f;
		size_t len;
		ssize_t n_wr;
		char *direction;
		char *value;

#if PL_GPIO_DEBUG
		LOG("configure gpio%d %d_%d",  gpio, GPIO_PORT(gpio), GPIO_PIN(gpio));
#endif

		/* check whether pin is already exported */
		snprintf(buf, BUFFER_SIZE, "%s/gpio%d/value", GPIO_SYSFS, gpio);

		f = fopen(buf, "r");

		// check whether the pin is already configured or not
		if (f == NULL) {
			// pin is not configured so far
			/* export pin */

			f = fopen("/sys/class/gpio/export", "w");

			if (f == NULL) {
				LOG("Failed to open GPIO export file");
				return -EPERM;
			}

			len = snprintf(buf, BUFFER_SIZE, "%d", gpio);
			n_wr = fwrite(buf, len, 1, f);
			fclose(f);

			if (n_wr != 1) {
				LOG("Failed to write to GPIO file");
				return -EIO;
			}
		}
		else {
			// pin is configured already, so just set direction and inactive value
			fclose(f);
#if PL_GPIO_DEBUG
			LOG("GPIO already configured");
#endif
		}

		/* set direction */
		snprintf(buf, BUFFER_SIZE, "%s/gpio%d/direction", GPIO_SYSFS, gpio);
		f = fopen(buf, "w");

		if (f == NULL) {
			LOG("Failed to open GPIO direction file");
			return -ENOENT;
		}

		if (flags & PL_GPIO_OUTPUT){
			direction = "out";
		} else {
			direction = "in";
		}
		n_wr = fwrite(direction, strlen(direction), 1, f);
		fclose(f);

		if (n_wr != 1) {
			//LOG_ERRNO("Failed to write to GPIO direction file");
			return -EIO;
		}

		/* set active high/low */
		snprintf(buf, BUFFER_SIZE, "%s/gpio%d/value", GPIO_SYSFS, gpio);
		f = fopen(buf, "w");

		if (f == NULL) {
			LOG("Failed to open GPIO active_low file");
			return -EIO;
		}

		if (flags & PL_GPIO_INIT_L){
			value = "0";
		} else {
			value = "1";
		}
		n_wr = fwrite(value, strlen(value), 1, f);
		fclose(f);

		/* set inactive */
		if (flags & PL_GPIO_OUTPUT){
			//vGPIODDsetInactive(gpio);
		}

	return 0;
}

/**
 * unexport the beaglebone gpios.
 *
 * @param gpio gpio number.
 * @return status.
 */
int beaglebone_gpio_unexport(unsigned gpio){
	static const size_t BUFFER_SIZE = 60;
	char buf[BUFFER_SIZE];
	FILE *f;
	size_t len;
	ssize_t n_wr;
	static const char GPIO_SYSFS[] = "/sys/class/gpio";

#if PL_GPIO_DEBUG
	LOG("configure gpio%d %d_%d",  gpio,  GPIO_PORT(gpio), GPIO_PIN(gpio));
#endif

	/* check whether pin is already exported */
	snprintf(buf, BUFFER_SIZE, "%s/gpio%d/value", GPIO_SYSFS, gpio);
	f = fopen(buf, "r");

	// check whether the pin is already configured or not
	if (f != NULL) {
		// pin is configured, so do the unexport now
		f = fopen("/sys/class/gpio/unexport", "w");

		if (f == NULL) {
			LOG("Failed to open GPIO export file");
			return -EPERM;
		}

		len = snprintf(buf, BUFFER_SIZE, "%d", gpio);
		n_wr = fwrite(buf, len, 1, f);
		fclose(f);

		if (n_wr != 1) {
			LOG("Failed to write to GPIO file");
			return -EIO;
		}
	}

	return 0;
}

/**
 * get gpio value.
 *
 * @param gpio gpio number.
 * @return gpio value.
 */
int beaglebone_gpio_get(unsigned gpio)
{
	 char azBuf[40];
	  char cData;
	  int iFile;

	  sprintf( azBuf, "/sys/class/gpio/gpio%d/value", gpio );
	  iFile = open( azBuf, O_RDONLY, 0 );
	  read( iFile, &cData, 1 );
	  close( iFile );

	  return (cData-'0')%48;
}

/**
 * set gpio value.
 *
 * @param gpio gpio number.
 * @param value gpio value.
 */
void beaglebone_gpio_set(unsigned gpio, int value)
{
	int BUFFER_SIZE = 40;
	char azBuf[BUFFER_SIZE];
	  int iFile;

	  // mask out other bits then LSB
	  value &= 0x01;

	  sprintf( azBuf, "/sys/class/gpio/gpio%d/value", gpio );

	  iFile = open( azBuf, O_WRONLY, 0 );
	  int len = snprintf(azBuf, BUFFER_SIZE, "%d", value);
	  write( iFile, azBuf, len );
	  close( iFile );
}
