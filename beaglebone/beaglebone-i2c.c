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
 * beaglebone-i2c.c
 *
 *      Authors:
 *           Nick Terry <nick.terry@plasticlogic.com>
 *   		 Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *   		 sebastian.friebe
 *   		 matti.haugwitz
 */

#include <beaglebone/beaglebone-i2c.h>
#include <unistd.h>
#include <pl/gpio.h>
#include <pl/i2c.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#define LOG_TAG "beaglebone-i2c"
#include <pl/utils.h>
#include <pl/assert.h>

static int beaglebone_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
			    const uint8_t *data, uint8_t count, uint8_t flags);
static int beaglebone_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t *data,
			   uint8_t count, uint8_t flags);


/**
 * Initialisation of the I2C Module.
 * Which i2c interface is determined at compile time.
 *
 * @param channel number of the beagelbone i2c bus /dev/i2c-?
 * @param i2c pl_i2c structure
 * @return status
 */
int beaglebone_i2c_init(uint8_t channel, struct pl_i2c *i2c)
{
	int bufferSize = 100;
	char userlandI2CDevice[bufferSize];
	snprintf(userlandI2CDevice, bufferSize, "/dev/i2c-%d", channel);
	if ( ( i2c->priv = open(userlandI2CDevice , O_RDWR, 0 ) ) == -1 )
	{
		char errorStr[bufferSize];
		snprintf(errorStr, bufferSize, "Failed to open userland spi device (%s)\n", userlandI2CDevice);
		fprintf( stderr,  errorStr);
		return -EPERM;
	}

	i2c->read = beaglebone_i2c_read;
	i2c->write = beaglebone_i2c_write;
	return 0;
}

/**
 * Write bytes to specified device - optional start and stop
 * First byte has to be preloaded for start to complete
 *
 * @param i2c pl_i2c structure
 * @param i2c_addr i2c address
 * @param data pointer to an array of values
 * @param count length of the array
 * @param flags not available yet
 * @return status
 */
static int beaglebone_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
			    const uint8_t *data, uint8_t count, uint8_t flags)
{

	errno = 0;
	if (ioctl(i2c->priv, I2C_SLAVE, i2c_addr) < 0) {
	        LOG("Failed to acquire bus access and/or talk to slave.\n");
	        /* ERROR HANDLING; you can check errno to see what went wrong */
	        return -errno;
	}
	int result = write(i2c->priv, data, count);
	if (result != count){
		return -ENODATA;
	}
	return 0;
}

/**
 * Read bytes from specified device - optional start and stop
 * There is an issue with receiving a single byte. We must
 * issue the stop before we even get the byte. Suspect emptying the
 * rx data buffer triggers the next read operation.
 *
 * @param i2c pl_i2c structure
 * @param i2c_addr i2c address
 * @param data pointer to an array of values
 * @param count length of the array
 * @param flags not available yet
 * @return status
 */
static int beaglebone_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t *data,
			   uint8_t count, uint8_t flags)
{
	errno = 0;
	if (ioctl(i2c->priv, I2C_SLAVE, i2c_addr) < 0) {
	        LOG("Failed to acquire bus access and/or talk to slave.\n");
	        /* ERROR HANDLING; you can check errno to see what went wrong */
	        return -errno;
	}

	int result = read(i2c->priv, data, count);
	if (result != count){
		return -ENODATA;
	}
	return 0;

}

