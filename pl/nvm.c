/*
 * nvm.c
 *
 *  Created on: 24.03.2015
 *      Author: sebastian.friebe
 */

#include <stdlib.h>
#include <pl/nvm.h>

#define LOG_TAG "nvm"
#include "pl/utils.h"
#include "epson/s1d13541-nvm.h"

/**
 * frees memory specified by a given pointer
 *
 * @param p pointer to the memory to be freed
 */
static void delete(struct pl_nvm *p);
static int read_wfdata(struct pl_nvm *nvm, uint8_t **data, int *count);
static int read_header(struct pl_nvm *nvm, int *isPgm);

// private functions
static int read_wfdata_S040_format(struct pl_nvm *nvm, uint8_t **data, int *count);
static int read_wfdata_plain_format(struct pl_nvm *nvm, uint8_t **data, int *count);
static int read_wfdata_generic_format(struct pl_nvm *nvm, uint8_t **data, int *count);

static int read_header_S040_format(struct pl_nvm *nvm, int *isPgm);
static int read_header_S1D13541_format(struct pl_nvm *nvm, int *isPgm);
static int read_header_generic_format(struct pl_nvm *nvm, int *isPgm);

static int get4Bytes(uint8_t *buffer, int addr);

// -----------------------------------------------------------------------------
// constructor
// ------------------------------
pl_nvm_t *pl_nvm_new(){
	pl_nvm_t *p = (pl_nvm_t *)malloc(sizeof(pl_nvm_t));;

	p->delete = delete;
	p->read_wfdata = read_wfdata;
	p->read_header = read_header;
	//remove p->pgm_header_and_wfdata = pgm_header_and_wfdata;

	return p;
}

// -----------------------------------------------------------------------------
// private functions
// ------------------------------
static void delete(struct pl_nvm *p){
	if (p != NULL){
		free(p);
		p = NULL;
	}
}


static int read_wfdata(struct pl_nvm *nvm, uint8_t **data, int *count){
	if (nvm == NULL){
		LOG("Abort: no nvm defined");
		return -1;
	}

	switch(nvm->nvm_format){
	case NVM_FORMAT_S040:
		return read_wfdata_S040_format(nvm, data, count);
		break;
	case NVM_FORMAT_EPSON:
		return read_wfdata_generic_format(nvm, data, count);
		break;
	case NVM_FORMAT_PLAIN:
		return read_wfdata_plain_format(nvm, data, count);
		break;
	default:
		LOG("Used nvm format (%d) does not support load waveform from nvm.", nvm->nvm_format);
		return -1;
	}
}

static int read_header(struct pl_nvm *nvm, int *isPgm){
	if (nvm == NULL){
		LOG("Abort: no nvm defined");
		return -1;
	}

	switch(nvm->nvm_format){
	case NVM_FORMAT_S040:
		return read_header_S040_format(nvm, isPgm);
		break;
	case NVM_FORMAT_S1D13541:
		return read_header_S1D13541_format(nvm, isPgm);
		break;
	case NVM_FORMAT_EPSON:
		return read_header_generic_format(nvm, isPgm);
		break;
	case NVM_FORMAT_PLAIN:
		// no header available
		break;
	default:
		LOG("Given nvm format not supported for header readout.");
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// private functions
// ------------------------------

static int read_wfdata_S040_format(struct pl_nvm *nvm, uint8_t **data, int *count){
	const int header_size = 294;
	const int offset_wf_data_length = 158;

	long unsigned int wf_data_length = 0;
	// read data, but it's in big-endian
	if (nvm->read(nvm, offset_wf_data_length, (uint8_t *)&wf_data_length, 4))
			return -1;

	// bring data in right order (little-endian)
	swap32(&wf_data_length);

	int byteCount = header_size + wf_data_length;

	*data = (uint8_t *)malloc(sizeof(uint8_t)* byteCount);

	if (nvm->read(nvm, 0, *data, byteCount))
		return -1;

	*count = byteCount;

	return 0;
}

static int read_wfdata_generic_format(struct pl_nvm *nvm, uint8_t **data, int *count){

	int wf_start = 0;
	int wf_len = 0;

	uint8_t buffer[nvm->size];

	if (nvm->read(nvm, 0, buffer, nvm->size))
		return -1;

	// check magic word
	if(buffer[NVM_MAGIC_ID_POS] != 0x50 || buffer[NVM_MAGIC_ID_POS+1] != 0x4C)
	{
		return -1;
	}

	// get wf start pos
	wf_start =  (int) buffer[NVM_WF_START_POS]     << 24;
	wf_start |= (int) buffer[NVM_WF_START_POS + 1] << 16;
	wf_start |= (int) buffer[NVM_WF_START_POS + 2] <<  8;
	wf_start |= (int) buffer[NVM_WF_START_POS + 3];

	// get wf length
	wf_len =  (int) buffer[NVM_WF_LEN_POS]     << 24;
	wf_len |= (int) buffer[NVM_WF_LEN_POS + 1] << 16;
	wf_len |= (int) buffer[NVM_WF_LEN_POS + 2] <<  8;
	wf_len |= (int) buffer[NVM_WF_LEN_POS + 3];

	*data = (uint8_t *)malloc(sizeof(uint8_t)*wf_len);

	// extract waveform data
	memcpy(*data, buffer + wf_start, wf_len);
	*count = wf_len;

	return 0;
}

static int read_wfdata_plain_format(struct pl_nvm *nvm, uint8_t **data, int *count){
	const int mtp_size = nvm->size;
	*count = mtp_size;

	*data = (uint8_t *)malloc(sizeof(uint8_t)*mtp_size);

	if (nvm->read(nvm, 0, *data, mtp_size))
		return -1;

	return 0;
}

static int read_header_S040_format(struct pl_nvm *nvm, int *isPgm){

	*isPgm = 0;
	return 0;
}

static int read_header_S1D13541_format(struct pl_nvm *nvm, int *isPgm){

	uint8_t buffer[nvm->size];

	if (nvm->read(nvm, 0, buffer, nvm->size))
		return -1;

	//memcpy(nvm->dispId, buffer, nvm->size);
	if(s1d13541_extract_eeprom_blob(nvm, buffer))
		return -1;

	if(atoi(nvm->dispId))
		*isPgm = 1;
	else
		*isPgm = 0;

	return 0;
}

static int read_header_generic_format(struct pl_nvm *nvm, int *isPgm){

	uint8_t buffer[nvm->size];
	nvm->read(nvm, 0x0, buffer, nvm->size);

	memcpy(nvm->dispId, buffer + NVM_DISPID_POS, NVM_DISPID_LEN);

	if(buffer[NVM_MAGIC_ID_POS] == 0x50 && buffer[NVM_MAGIC_ID_POS+1] == 0x4C)
	{
		*isPgm = 1;
	}

	memcpy(nvm->prodId, buffer + NVM_PRODID_POS, NVM_PRODID_LEN);
	memcpy(nvm->wfVers, buffer + NVM_WFVERS_POS,  NVM_WFVERS_LEN);
	memcpy(nvm->fplVers, buffer + NVM_FPLVERS_POS, NVM_FPLVERS_LEN);
	memcpy(nvm->nvmVers, buffer + NVM_NVMVERS_POS, NVM_NVMVERS_LEN);
	memcpy(nvm->feature1, buffer + NVM_FEATURE1_POS, NVM_FEATURE_LEN);
	memcpy(nvm->feature2, buffer + NVM_FEATURE2_POS, NVM_FEATURE_LEN);
	memcpy(nvm->feature3, buffer + NVM_FEATURE3_POS, NVM_FEATURE_LEN);
	memcpy(nvm->feature4, buffer + NVM_FEATURE4_POS, NVM_FEATURE_LEN);

	nvm->vcom = get4Bytes(buffer, NVM_VCOM_POS);

	return 0;
}

static int get4Bytes(uint8_t *buffer, int addr){

	int value = 0;

	value  = (int) (buffer[addr]   << 24);
	value |= (int) (buffer[addr+1] << 16);
	value |= (int) (buffer[addr+2] << 8);
	value |= (int) (buffer[addr+3]);

	return value;
}
/*//remove
static void set4Bytes(uint8_t *buffer, int value, int addr){

	buffer[addr]    = (value & 0xff000000) >> 24;
	buffer[addr+1]  = (value & 0x00ff0000) >> 16;
	buffer[addr+2]  = (value & 0x0000ff00) >> 8;
	buffer[addr+3]  = (value & 0x000000ff);
}
//*/
