
#ifndef INCLUDE_PL_NVM_H
#define INCLUDE_PL_NVM_H 1
/*
 * nvm.h -- nonvolatile memory interface abstraction layer
 *
 * Created on: 12.12.2014
 * Author: matti.haugwitz
 */

#include <stdint.h>
#include <pl/i2c.h>
#include <pl/hv.h>


// generic nvm format for freescale an epson (einc)
#define NVM_PRODID_POS 		0x70000
#define NVM_PRODID_LEN		0x00010
#define NVM_VCOM_POS		0x70010
#define NVM_VCOM_LEN		0x00010
#define NVM_WFVERS_POS 		0x70020
#define NVM_WFVERS_LEN		0x00020
#define NVM_FPLVERS_POS		0x70040
#define NVM_FPLVERS_LEN		0x00010
#define NVM_DISPID_POS 		0x70050
#define NVM_DISPID_LEN 		0x00030
#define NVM_NVMVERS_POS 	0x70080
#define NVM_NVMVERS_LEN		0x00010
#define NVM_FEATURE1_POS 	0x70090
#define NVM_FEATURE2_POS 	0x70094
#define NVM_FEATURE3_POS 	0x70098
#define NVM_FEATURE4_POS 	0x7009C
#define NVM_FEATURE_LEN		0x00004
#define NVM_WF_START_POS	0x700A0 // 4 bytes
#define NVM_WF_LEN_POS		0x700A4 // 4 bytes
#define NVM_MAGIC_ID_POS	0x70080
#define NVM_MAGIC_ID_LEN	0x00002
#define NVM_MAGIC_ID		0x504C //PL
#define NVM 0xffff;

#define NVM_EPSON_INIT_CODE_POS 0x00000
#define NVM_EPSON_INIT_CODE_LEN 0x0108A
#define NVM_EPSON_WF_POS 		0x0108A
#define NVM_EPSON_WF_LEN 		0x33F76
#define NVM_FREESCALE_WF_POS	0x35000
#define NVM_FREESCALE_WF_LEN	0x35000

enum nvm_format_version{
	NVM_FORMAT_PLAIN,
	NVM_FORMAT_S1D13541,
	NVM_FORMAT_S040,
	NVM_FORMAT_EPSON
};

/* nvm communication type */
enum nvm_type{
	i2c = 1,
	spi = 2
};


typedef struct pl_nvm {
	enum nvm_format_version nvm_format;		//!< specifies the data format stored in the NVM

//	int (*pgm)(struct pl_nvm *nvm, unsigned int addr, uint8_t *data, int count);
//	int (*pgm_header_and_wfdata)(struct pl_nvm *nvm, uint8_t *data, int count);

	int (*read)(struct pl_nvm *nvm, unsigned int addr, uint8_t *data, int count);
	int (*read_wfdata)(struct pl_nvm *nvm, uint8_t **data, int *count);
	int (*read_header)(struct pl_nvm *nvm, int *isPgm);

	// --- nvm header ---
	char prodId[NVM_PRODID_LEN];
	char dispId[NVM_DISPID_LEN];
	char wfVers[NVM_WFVERS_LEN];
	char fplVers[NVM_FPLVERS_LEN];
	char nvmVers[NVM_NVMVERS_LEN];
	char feature1[NVM_FEATURE_LEN];
	char feature2[NVM_FEATURE_LEN];
	char feature3[NVM_FEATURE_LEN];
	char feature4[NVM_FEATURE_LEN];
	int vcom;

	int size;       // nvm size
	void  *hw_ref;	//!< reference to any underlying hardware
	void (*delete)(struct pl_nvm *p);
} pl_nvm_t;


/**
 * allocates memory to hold a pl_nvm structure
 *
 * @return pointer to allocated memory
 */
pl_nvm_t *pl_nvm_new();

#endif /* INCLUDE_PL_NVM_H */
