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
 * s1d13541-nvm.c
 *
 *  Created on: 26 Apr 2016
 *      Author: matti.haugwitz
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <epson/s1d13541-nvm.h>
#include <pl/assert.h>

#define LOG_TAG "s1d13541-nvm"
#include <pl/utils.h>

//remove static int nvm_s1d13541_erase(struct pl_nvm * nvm);
//static int nvm_s1d13541_pgm(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len);
static int nvm_s1d13541_read(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len);
static int wait_for_ack(struct s1d135xx * p, uint16_t status, uint16_t mask);

void itoa(int n, char s[]);
void reverse(char s[]);

s1d13541_nvm_t * s1d13541_nvm_new(s1d135xx_t * p, pl_i2c_t * i2c, pl_hv_driver_t *pl_hv_driver, pl_hv_timing_t *pl_hv_timing)
{
	s1d13541_nvm_t *nvm = (s1d13541_nvm_t *)malloc(sizeof(s1d13541_nvm_t));

	nvm->s1d13541 = p;
	nvm->i2c = i2c;
	nvm->pl_hv_driver = pl_hv_driver;
	nvm->pl_hv_timing = pl_hv_timing;

	return nvm;
}


pl_nvm_t *s1d13541_get_nvm(s1d135xx_t * p, pl_i2c_t * i2c, pl_hv_driver_t *pl_hv_driver, pl_hv_timing_t *pl_hv_timing){
	assert(p != NULL);

	pl_nvm_t *nvm = pl_nvm_new();
	s1d13541_nvm_t *hw_ref = s1d13541_nvm_new(p, i2c, pl_hv_driver, pl_hv_timing);

	nvm->hw_ref = hw_ref;
	nvm->read = nvm_s1d13541_read;
//	nvm->pgm = nvm_s1d13541_pgm;

	nvm->size = 0x08;

	return nvm;
}


static int nvm_s1d13541_read(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len){

	s1d13541_nvm_t * s1d13541_nvm = nvm->hw_ref;
	s1d135xx_t *p = s1d13541_nvm->s1d13541;
	int i = 0, j = 0;
	uint16_t data = 0;
	uint16_t addr_ = 0;

	// wait for status: idle
	if(wait_for_ack(p, S1D13541_PROM_STATUS_IDLE, 0xffff))
		return -ETIME;

	for(i=0; i<len; i++)
	{
		for(j=0; j<2; j++)
		{
			// set read address
			addr_ = ((i*2+j) << 8) & 0x0f00;
			p->write_reg(p, S1D13541_PROM_ADR_PGR_DATA, addr_);

			// set read operation start trigger
			p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_READ_START);

			//wait for status: read mode start
			if(wait_for_ack(p, S1D13541_PROM_STATUS_READ_MODE, S1D13541_PROM_STATUS_READ_MODE))
				return -ETIME;

			//wait for status: read operation finished
			if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_READ_BUSY))
				return -ETIME;

			// set read operation start trigger
			data = p->read_reg(p, S1D13541_PROM_READ_DATA);
			if(j)
				blob[i] |= data & 0x0f;
			else
				blob[i] = data << 4 & 0xf0;

		}
	}

	// set read mode stop trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_READ_STOP);

	//wait for status: read mode stop
	if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_READ_MODE))
		return -ETIME;

	return 0;
}
/*
static int nvm_s1d13541_erase(struct pl_nvm * nvm){

	s1d13541_nvm_t * s1d13541_nvm = nvm->hw_ref;
	s1d135xx_t *p = s1d13541_nvm->s1d13541;
	pl_i2c_t *i2c = s1d13541_nvm->i2c;

	pl_hv_driver_t *hv_driver = s1d13541_nvm->pl_hv_driver;
	pl_hv_timing_t *hv_timing = s1d13541_nvm->pl_hv_timing;

	uint8_t i2c_data[2];
	uint8_t toffset_vsl_on_backup = 0;

	// wait for status: idle
	if(wait_for_ack(p, S1D13541_PROM_STATUS_IDLE, 0xffff))
		return -1;

	// set erase all mode start trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_ERASE_ALL_MODE_START);

	//wait for status: erase all mode start
	if(wait_for_ack(p, S1D13541_PROM_STATUS_ERASE_ALL_MODE, 0xffff))
		return -1;

	// wait 1.5ms
	usleep(1500);

	// turn on VME1

	// configure and switch hv on
	// Since erase does not work as long as VSourceNeg is supplied to the x541,
	// the VSourceNeg Voltage has to be delayed until the erase process has finished.
	// Maximum possible delay on max17135 is 255 ms.

	// backup VSourceNeg timing
	toffset_vsl_on_backup = hv_timing->toffset_vsl_on;
	// set VSourceNeg timing delay to 255ms
	hv_timing->toffset_vsl_on = 0xff;
	pl_pmic_t *pmic = (pl_pmic_t*) hv_timing->hw_ref;
	pmic->is_initialized = 1; // force pmic state
	if(hv_timing->set_timings(hv_timing))
		return -1;

	// configure digipot to 22V VGatePos
	i2c_data[0] = 0x10;
	i2c_data[1] = 0x72;
	i2c->write(i2c, HV_DIGIPOT_I2C_ADDR, i2c_data, 2, 0);
	// configure i2c gpio expander: define all outputs as low
	i2c_data[0] = HV_GPIO_EXPANDER_OUTPUT_REG;
	i2c_data[1] = 0x00;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// configure i2c gpio expander: define all ports as output
	i2c_data[0] = HV_GPIO_EXPANDER_CTRL_REG;
	i2c_data[1] = HV_GPIO_EXPANDER_ALL_PORTS_OUT;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// configure i2c gpio expander: set output P0 to high to enable SSR for VME1
	i2c_data[0] = HV_GPIO_EXPANDER_OUTPUT_REG;
	i2c_data[1] = HV_GPIO_EXPANDER_VME1_ON;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);

	// switch hv voltages on
	if(hv_driver->switch_on(hv_driver))
		return -1;

	// wait 1.5ms
	usleep(1500);

	// set erase all operation start trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_ERASE_ALL_OP_START);

	// wait for status: erase all operation start
	if(wait_for_ack(p, S1D13541_PROM_STATUS_ERASE_BUSY, S1D13541_PROM_STATUS_ERASE_BUSY))
		return -1;

	// wait 1.5ms
	usleep(1500);

	// set erase all operation stop trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_ERASE_ALL_OP_STOP);

	// wait for status: erase all operation stop
	if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_ERASE_BUSY))
		return -1;

	// wait 1.5ms
	usleep(1500);

	// turn off VME1
	// configure i2c gpio expander: define all outputs as low
	i2c_data[0] = HV_GPIO_EXPANDER_OUTPUT_REG;
	i2c_data[1] = 0x00;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// switch hv off
	if (hv_driver->switch_off(hv_driver))
		return -1;
	// restore VSourceNeg timing
	hv_timing->toffset_vsl_on = toffset_vsl_on_backup;
	if(hv_timing->set_timings(hv_timing))
		return -1;
	// restart digipot to reload default register values
	i2c_data[0] = 0xb0;
	i2c_data[1] = 0x00;
	i2c->write(i2c, HV_DIGIPOT_I2C_ADDR, i2c_data, 2, 0);

	// wait 1.5ms
	usleep(1500);

	// set erase all mode stop trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_ERASE_ALL_MODE_STOP);

	//wait for status: erase all mode stop
	if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_ERASE_ALL_MODE))
		return -1;

	return 0;
}



static int nvm_s1d13541_pgm(struct pl_nvm * nvm, unsigned int addr, uint8_t * blob, int len){

	// erase nvm
	if(nvm_s1d13541_erase(nvm))
	{
		LOG("Erasing PROM failed.");
		return -1;
	}

	// pgm nvm

	s1d13541_nvm_t * s1d13541_nvm = nvm->hw_ref;
	s1d135xx_t *p = s1d13541_nvm->s1d13541;
	pl_i2c_t *i2c = s1d13541_nvm->i2c;

	pl_hv_driver_t *hv_driver = s1d13541_nvm->pl_hv_driver;

	int i = 0, j = 0;
	uint16_t data = 0x0;
	uint8_t cmp_blob[len];
	uint8_t i2c_data[2];

	// wait for status: idle
	if(wait_for_ack(p, S1D13541_PROM_STATUS_IDLE, 0xffff))
		return -1;

	// set program mode start trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_PGM_MODE_START);

	//wait for status: erase all mode start
	if(wait_for_ack(p, S1D13541_PROM_STATUS_PGM_MODE, 0xffff))
		return -1;

	// wait 1.5ms
	usleep(1500);

	// turn on VME2
	// configure i2c gpio expander: define all outputs as low
	i2c_data[0] = HV_GPIO_EXPANDER_OUTPUT_REG;
	i2c_data[1] = 0x00;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// configure i2c gpio expander: define all ports as output
	i2c_data[0] = HV_GPIO_EXPANDER_CTRL_REG;
	i2c_data[1] = HV_GPIO_EXPANDER_ALL_PORTS_OUT;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// configure i2c gpio expander: set output P1 to high to enable SSR for VME2
	i2c_data[0] = HV_GPIO_EXPANDER_OUTPUT_REG;
	i2c_data[1] = HV_GPIO_EXPANDER_VME2_ON;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// switch hv on
	hv_driver->switch_on(hv_driver);

	// wait 1.5ms
	usleep(1500);

	for(i = 0; i<len; i++)
	{
		for(j=0; j<2; j++)
		{
			data = 0x0000;
			// write data
			if(j)
				data = blob[i] & 0x000f;
			else
				data =  blob[i] >> 4 & 0x000f;

			data |= ((i*2+j) << 8)    & 0x0f00; // write address

			// Set Data/Address REG
			p->write_reg(p, S1D13541_PROM_ADR_PGR_DATA, data);

			// set program operation start trigger
			p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_PGM_OP_START);

			//wait for status: pgm operation start
			if(wait_for_ack(p, S1D13541_PROM_STATUS_PGM_BUSY, S1D13541_PROM_STATUS_PGM_BUSY))
				return -1;

			// wait 1.5ms
			usleep(1500);

			// set program operation stop trigger
			p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_PGM_OP_STOP);

			//wait for status: program operation stop
			if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_PGM_BUSY))
				return -1;

			// wait 1.5ms
			usleep(1500);
		}
	}

	// turn off VME2
	// configure i2c gpio expander: define all outputs as low
	i2c_data[0] = HV_GPIO_EXPANDER_OUTPUT_REG;
	i2c_data[1] = 0x00;
	i2c->write(i2c, HV_GPIO_EXPANDER_I2C_ADDR, i2c_data, 2, 0);
	// switch hv off
	hv_driver->switch_off(hv_driver);

	// wait 1.5ms
	usleep(1500);

	// set program mode stop trigger
	p->write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_PGM_MODE_STOP);

	// wait for status: idle
	if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_PGM_MODE))
		return -1;

	// data verification
	nvm_s1d13541_read(nvm, addr, cmp_blob, len);

	for(i=0; i<8; i++)
		printf("%x \t %x\n", blob[i], cmp_blob[i]);

	if (memcmp(blob, cmp_blob, len) != 0)
	{
		LOG("NVM programming finished: verify after pgm failed!\n");
		return -1;
	}
	else
	{
		printf("NVM programming finished: verification successful!\n");
	}

	return 0;
}
*/
static int wait_for_ack (struct s1d135xx *p, uint16_t status, uint16_t mask)
{
	unsigned long timeout = 50000;
	uint16_t v = 0x0;

	while ((v = p->read_reg(p, S1D13541_PROM_STATUS) & mask) != status){
		--timeout;
		if (timeout == 0){
			LOG("PROM acknowledge timeout");
			return -ETIME;
		}
	}

	return 0;
}

/*
	* all values saved in big-endian format
	* VCom step size is 10mV

	Frontend Batch ID = [Addr 0x0 Bit 3..0] to [Addr 0x5 Bit 3..0]   ---> max. 16777215 (7 Digits required)
	SubCon Flag       = [Addr 0x6 Bit 3..0]
	Serial Display ID = [Addr 0x7 Bit 3..0] to [Addr 0xb Bit 3..0]   ---> max. 1048575 (5 Digits required)
	Waveform Rev.     = [Addr 0xc Bit 3..0] to [Addr 0xd Bit 3..2]   ---> max. 63
	VCom              = [Addr 0xd Bit 1..0] to [Addr 0xf Bit 3..0]   ---> max. 1023 = 10.23V

	Addr   | Bit Usage
	0x0    | Bit 3-0 Frontend Batch ID
	0x1    | Bit 3-0 Frontend Batch ID
	0x2    | Bit 3-0 Frontend Batch ID
	0x3    | Bit 3-0 Frontend Batch ID
	0x4    | Bit 3-0 Frontend Batch ID
	0x5    | Bit 3-0 Frontend Batch ID
    0x6    | Bit 3-0 SubCon Flag
	0x7    | Bit 3-0 Serial Display ID
	0x8    | Bit 3-0 Serial Display ID
	0x9    | Bit 3-0 Serial Display ID
	0xa    | Bit 3-0 Serial Display ID
	0xb    | Bit 3-0 Serial Display ID
	0xc    | Bit 3-0 Waveform Rev.
	0xd    | Bit 3-2 Waveform Rev. | Bit 1-0 VCom
	0xe    | Bit 3-0 VCom
	0xf    | Bit 3-0 VCom
 */
int s1d13541_generate_eeprom_blob(struct pl_nvm *nvm, uint8_t *data)
{
	int fe = 0;
	int sn = 0;
	int wf = 0;
	int vcom = 0;
	int subConFlag = 0;

	char c_fe[7+1] = {0,};
	char c_sn[7+1] = {0,};

	if(strlen(nvm->dispId) == 14)
	{
		strncpy(c_fe, nvm->dispId, 7);
		strncpy(c_sn, nvm->dispId + 7, 5);
	}
	else if(strlen(nvm->dispId) == 18)
	{
		strncpy(c_fe, nvm->dispId, 7);
		strncpy(c_sn, nvm->dispId + 13, 5);
		subConFlag = 1;
	}
	else
	{
		printf("Not able to generate the EEProm blob, due to wrong dispID length: %d\n", strlen(nvm->dispId));
		return -EEPDC;
	}

	fe = atoi(c_fe);
	sn = atoi(c_sn);
	wf = atoi(nvm->wfVers);

	data[0] = fe >> 16 & 0xff; // frontend batch
	data[1] = fe >> 8  & 0xff;
	data[2] = fe       & 0xff;

	data[3] = subConFlag << 4 & 0xff; // subcon flag

	data[3] |= sn >> 16 & 0x0f; 	  // serial number
	data[4] = sn >> 8  & 0xff;
	data[5] = sn       & 0xff;

	data[6] = wf << 2 & 0xfc;	// waveform version uses bits 7-2

	vcom = nvm->vcom/10;		  // vcom reduced to 10mV step size
	data[6] |= vcom >> 8 & 0x03;  // vcom uses bits 1-0 of data[6], 7-2 used by wf id
	data[7]  = vcom      & 0xff;  //




	return 0;
}

int s1d13541_extract_eeprom_blob(struct pl_nvm *nvm, uint8_t *data)
{
	int fe = 0;
	int sn = 0;
	int wf = 0;
	char c_fe[7+1] = {0,};
	char c_sn[7+1] = {0,};
	int subConFlag = 0;

	fe  = data[0] << 16;
	fe |= data[1] << 8;
	fe |= data[2];

	subConFlag = data[3] >> 4 & 0x0f;

	sn  = (data[3] & 0x0f) << 16 ;
	sn |= data[4] << 8;
	sn |= data[5];

	itoa(fe, c_fe);
	itoa(sn, c_sn);

	// check whether it is PL (0) or SubCon (1) Format
	if(subConFlag > 0)
	{
		char tmp[] = {"000000000000000000"}; // 18digits
		strncpy(nvm->dispId, tmp, strlen(tmp));
		memcpy(nvm->dispId + 7 - strlen(c_fe), c_fe, strlen(c_fe));
		memcpy(nvm->dispId + 11 + 7 - strlen(c_sn), c_sn, strlen(c_sn));
	}
	else
	{
		char tmp[] = {"00000000000000"}; // 14digits
		strncpy(nvm->dispId, tmp, strlen(tmp));
		memcpy(nvm->dispId + 7 - strlen(c_fe), c_fe, strlen(c_fe));
		memcpy(nvm->dispId + 7 + 5 - strlen(c_sn), c_sn, strlen(c_sn));
	}

	// waveform version
	wf = data[6] >> 2 & 0x3f;
	itoa(wf , nvm->wfVers);

	// vcom
	nvm->vcom  = data[6] << 8 & 0x300 ;
	nvm->vcom |= data[7]      & 0xff;
	nvm->vcom = nvm->vcom * 10; // since vcom was reduced to 10mV step size

	return 0;
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

