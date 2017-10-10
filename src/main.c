/*
 * BBepdcULD.c
 *
 *  Created on: 10.09.2014
 *      Author: sebastian.friebe
 *
 *  Main file for the project.
 *  Handling command line calls and controls the different EPDC operations.
 *
 *  Following operations are available via command line parameters:
 *  (most of them will require additional parameters to work)
 *
 * 		-start_epdc		: initializes the EPDC
 * 		-stop_epdc		: de-initializes the EPDC
 * 		-set_waveform   : send the waveform file to the EPDC
 * 		-set_vcom       : sets the target vcom
 * 		-update_image   : execute an image update
 *
 *
 */

#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <pl/generic_epdc.h>
#include <pl/generic_controller.h>
#include <pl/vcom.h>
#include <pl/i2c.h>
#include <pl/gpio.h>
#include <pl/hwinfo.h>
#include <pl/assert.h>
#include <pl/parser.h>
#include "pindef.h"
#include "hw_setup.h"
#define LOG_TAG "main"
#include <pl/utils.h>
#include "text.h"
/* remove
#include "epson/epson-s1d135xx.h"
*/

#define INTERNAL_USAGE
#define VERSION_INFO		"v1"


// ----------------------------------------------------------------------
// -  global variables
// ----------------------------------------------------------------------
static pl_generic_epdc_t *epdc;					//!< pointer to a generic globally used EPDC object
static hw_setup_t *hardware;

struct CmdLineOptions {
	char *operation;
	char *short_description;
	int (*execute_operation)(int argc, char **argv);
	void (*print_help_message)();

};

#define ERROR_ARGUMENT_COUNT_MISMATCH 	125

// ----------------------------------------------------------------------
// function declarations
// ----------------------------------------------------------------------
int initialize_environment();
int release_environment();
int readBinaryFile(const char *binaryPath, uint8_t **buffer);

int start_epdc(int load_nvm_content, int execute_clear);
int stop_epdc();
int set_vcom(int vcom);
int set_waveform(char *waveform, float *temperature);
int set_temperature(float temperature);
int update_image(char *path, const char* wfID, enum pl_update_mode mode, int vcom_switch, int updateCount, int waitTime);
int read_register(regSetting_t regSetting);
int write_register(regSetting_t regSetting, const uint32_t bitmask);
int send_cmd(regSetting_t regSetting);
//remove int pgm_nvm(const char *waveform);
int switch_hv(int state);
void debug_print_parameters(int argc, char **argv);
void print_application_help(int print_all);
int info();
int show_image(const char *dir, const char *file, int wfid);
int counter(const char* wf);
//remove int interface_data(	char* interface,int number_of_values,char values);
int slideshow(const char *path, const char* wf, int count);

int execute_help(int argc, char **argv);
int execute_start_epdc(int argc, char **argv);
int execute_stop_epdc(int argc, char **argv);
int execute_set_vcom(int argc, char **argv);
int execute_set_waveform(int argc, char **argv);
int execute_set_temperature(int argc, char **argv);
int execute_update_image(int argc, char **argv);
int execute_slideshow(int argc, char **argv);
int execute_counter(int argc, char **argv);
//remove int execute_interface_data(int argc, char**argv);
int execute_write_reg(int argc, char **argv);
int execute_read_reg(int argc, char **argv);
//remove int execute_pgm_nvm_binary(int argc, char **argv);
int execute_info(int argc, char **argv);
int execute_switch_hv(int argc, char **argv);
int execute_send_cmd(int argc, char **argv);
int print_versionInfo(int argc, char **argv);

void printHelp_start_epdc(int identLevel);
void printHelp_stop_epdc(int identLevel);
void printHelp_set_vcom(int identLevel);
void printHelp_set_waveform(int identLevel);
void printHelp_set_temperature(int identLevel);
void printHelp_update_image(int identLevel);
void printHelp_write_reg(int identLevel);
void printHelp_read_reg(int identLevel);
//remove void printHelp_pgm_nvm_binary(int identLevel);
void printHelp_info(int identLevel);
//remove void printHelp_interface_data(int identLevel);
void printHelp_slideshow(int identLevel);
void printHelp_counter(int identLevel);
void printHelp_switch_hv(int identLevel);
void printHelp_send_cmd(int identLevel);

struct CmdLineOptions supportedOperations[] = {
		{"-start_epdc",	 	"initializes the EPD controller", 			execute_start_epdc, 	printHelp_start_epdc},
		{"-stop_epdc", 		"de-initializes the EPD controller", 		execute_stop_epdc, 		printHelp_stop_epdc},
		{"-set_vcom", 		"sets com voltage", 						execute_set_vcom, 		printHelp_set_vcom},
		{"-set_waveform", 	"sets the waveform", 						execute_set_waveform, 	printHelp_set_waveform},
		{"-set_temperature","sets the temperature", 					execute_set_temperature,printHelp_set_temperature},
		{"-update_image", 	"updates the display", 						execute_update_image, 	printHelp_update_image},
		{"-slideshow",		"shows a slidshow of .png images",			execute_slideshow,		printHelp_slideshow},
		//remove 		{"-interface",		"shows a data of an interface",				execute_interface_data,	printHelp_interface_data},
		{"-count",			"shows a counting number",					execute_counter,		printHelp_counter},
#ifdef INTERNAL_USAGE
		{"-send_cmd", 		"sends a command of EPD controller", 		execute_send_cmd, 		printHelp_send_cmd},
		{"-write_reg", 		"writes to a register of EPD controller", 	execute_write_reg, 		printHelp_write_reg},
		{"-read_reg", 		"reads from a register of EPD controller", 	execute_read_reg, 		printHelp_read_reg},
		{"-info",			"displays general display informations",	execute_info, printHelp_info},
#endif
		{"-switch_hv",	    "switches hv on/off based on parameter",	execute_switch_hv,	    printHelp_switch_hv},
		//remove 		{"-pgm_nvm",		"programs the nvm with a binary file",		execute_pgm_nvm_binary,	printHelp_pgm_nvm_binary},
		{"--version", 		"displays version info", 					print_versionInfo, 		NULL},
		{"--help", 			"prints this help message", 				execute_help, 			NULL},
};

/**
 * Main sequence
 * Checks the command line parameter and executes selected operation.
 */
int main(int argc, char* argv[])
{

	// debug features
	printf("!\n");
	int stat = 0;
	char message[200];

	if (initialize_environment()){
		abort_msg("Initialization failed", ABORT_APPLICATION);
		return -1;
	}

	// parse input parameter
	if(argc > 1)
	{
		int operationIdx = 0;
		struct CmdLineOptions *matchedOperation = NULL;
		for(operationIdx=0; operationIdx<(sizeof(supportedOperations)/sizeof(struct CmdLineOptions)); operationIdx++){
			if (!strcmp(argv[1], supportedOperations[operationIdx].operation)){
				matchedOperation = &supportedOperations[operationIdx];
				break;
			}
		}

		// found operation, so execute it
		if (matchedOperation != NULL){

			// want to print a help message for this operation?
			if ((argc > 2) && !strcmp(argv[2],"--help")){
				if (matchedOperation->print_help_message != NULL){
					matchedOperation->print_help_message(0);
				}
			}

			// just execute the operation
			else {
				stat = matchedOperation->execute_operation(argc, argv);
				if (stat){

					if ((stat == ERROR_ARGUMENT_COUNT_MISMATCH) && (matchedOperation->print_help_message != NULL)){
						sprintf(message, "wrong number of arguments...");
						abort_msg(message, ABORT_APPLICATION);

						printf("Please check the help message below:\n\n");
						matchedOperation->print_help_message(0);
					}
					else {
						sprintf(message, "operation '%s' failed", matchedOperation->operation);
						abort_msg(message, ABORT_APPLICATION);
					}
				}
			}
		}

		else
		{
		  abort_msg("Specified operation not supported\n", ABORT_APPLICATION);
		  print_application_help(false);
		  return 1;
		}
	}
	else
	{
		abort_msg("Wrong number of parameter. Try: --help\n", ABORT_APPLICATION);
		print_application_help(false);
		return 1;
	}


	release_environment();

	return stat;
}

int initialize_environment(){
	int stat = 0;


	hardware = hw_setup_new();

#ifdef INTERNAL_USAGE
	stat |= hardware->init_from_configfile(hardware, "/boot/uboot/epdc/S115_T1.1/epdc.config");
#else
	stat |= hardware->init_from_configfile(hardware, "BBepdcULD.config");
#endif

	epdc = generic_epdc_new();

	epdc->hv = hv_new();

	epdc->hv->hvConfig = hardware->hvConfig;
	epdc->hv->hvDriver = hardware->hvDriver;
	epdc->hv->hvTiming = hardware->hvTiming;
	epdc->hv->vcomConfig = hardware->vcomConfig;
	epdc->hv->vcomDriver = hardware->vcomDriver;
	epdc->hv->vcomSwitch = hardware->vcomSwitch;

	epdc->nvm = hardware->nvm;
	epdc->controller = hardware->controller;
	epdc->default_vcom = hardware->default_vcom;

	return stat;
}

/**
 * Releases all allocated ressources as SPI and controller as well as hv hardware parts.
 * @return always 0;
 */
int release_environment(){
	// release spi device

	hardware->sInterface->close(hardware->sInterface);
	hardware->sInterface->delete(hardware->sInterface);

	epdc->delete(epdc);
	return 0;
}

// ----------------------------------------------------------------------
// operation functions
// ----------------------------------------------------------------------
int execute_help(int argc, char **argv){

	int print_all = false;

	if (argc >=3){
		if (strcmp(argv[2], "-all") == 0){
			print_all = true;
		}
	}

	print_application_help(print_all);
	return 0;
}

int execute_start_epdc(int argc, char **argv){

	int stat = 0;
	int executeClear = false;
	int initFromEEprom = false;

	if (argc >=4){
		executeClear = atoi(argv[3]);
	}

	if (argc >= 3)
	{
		initFromEEprom = atoi(argv[2]);
	}

	stat = start_epdc(initFromEEprom, executeClear);

	return stat;
}

int execute_stop_epdc(int argc, char **argv){

	int stat = 0;

	stat = stop_epdc();

	return stat;
}

int execute_set_vcom(int argc, char **argv){

	int stat;

	if(argc == 3)
	{
		stat = set_vcom(atof(argv[2])*1000);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_set_waveform(int argc, char **argv){

	int stat;
	float temperature;

	if(argc == 3)
	{
		stat = set_waveform(argv[2], NULL);
	}
	else if(argc == 4)
	{
		temperature = atof(argv[3]);
		stat = set_waveform(argv[2], &temperature);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_set_temperature(int argc, char **argv){

	int stat;
	float temperature;

	if(argc == 3)
	{
		temperature = atof(argv[2]);
		stat = set_temperature(temperature);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	return stat;
}

int execute_update_image(int argc, char **argv){

	int stat;

	char* wfID = "default";
	int mode = 0;
	int vcom_switch_enable = 1;
	int updateCount = 1;
	int waitTime = 0;

	if(argc >= 8) vcom_switch_enable = atol(argv[7]);

	if(argc >= 7) waitTime = atoi(argv[6]);

	if(argc >= 6) updateCount = atoi(argv[5]);

	if(argc >= 5) mode = atoi(argv[4]);

	if(argc >= 4) wfID = argv[3];

	if (argc >= 3) {
		stat = update_image(argv[2], wfID, (enum pl_update_mode) mode,
				vcom_switch_enable, updateCount, waitTime);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}
	return stat;
}

int execute_counter(int argc, char**argv){

	int stat;
	printf("%s\n",__func__);
	stat = counter(NULL);

	return stat;
}
/*//remove
int execute_interface_data(int argc, char**argv){
	int stat;
	printf("%s\n",__func__);
	char* interface = "/dev/can1";
	int number_of_values = 4;
	char values = 0x03;

	if(argc >= 5) values = atoi(argv[4]);
	if(argc >= 4) number_of_values = atoi(argv[3]);

	if(argc>=3){
		interface = argv[2];
		stat = interface_data(argv[2], number_of_values, values);
	}
	else
	{
		stat = interface_data(interface, number_of_values, values);
	}
	return stat;
}
*/
int execute_slideshow(int argc, char**argv){

	int stat;
	//slideshow path wfid waittime
	int waitTime = 700;
	char* wfID = NULL;
	if(argc >= 5) waitTime = atoi(argv[4]);
	if(argc >= 4) wfID = argv[3];

	if(argc>=3){
		stat = slideshow(argv[2],wfID, waitTime);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}
	return stat;
}

int execute_send_cmd(int argc, char **argv){

	int stat;
	regSetting_t regData;
	regData.valCount = 1;
	uint16_t *data;
	static const char sep[] = ",";
	unsigned int val, len;



	if(argc >= 5){
		regData.addr = (int)strtol(argv[2], NULL, 0);
		regData.valCount = (int)strtol(argv[3], NULL, 0);

		data = malloc(regData.valCount*sizeof(uint16_t));

		printf("value string: %s\n", argv[4]);
		int currentPosition = 0;
		int idx=0;

		for(idx=0; idx<regData.valCount; idx++){
			len = parser_read_word(argv[4] + currentPosition, sep, &val);
			currentPosition+=len;
			data[idx] = val;
			if (len <= 0)
				break;
		}

		regData.val = data;

		printf("found data values: ");

		for(idx=0; idx<regData.valCount;idx++){
			printf("0x%x,", ((uint16_t *)regData.val)[idx]);
		}
		printf("\n");
	}
	else if(argc >= 3)
	{
		regData.addr = (int)strtol(argv[2], NULL, 0);
		regData.valCount = 0;
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	stat = send_cmd(regData);
	if(regData.val)
		free(regData.val);
	return stat;
}

int execute_write_reg(int argc, char **argv){

	int stat;
	regSetting_t regData;
	regData.valCount = 1;
	uint16_t *data;
	static const char sep[] = ",";
	unsigned int val, len;
	uint32_t bitmask = 0xFFFFFFFF;

	if(argc >= 6){
		bitmask = (int)strtol(argv[5], NULL, 0);
	}

	if(argc >= 5){
		regData.addr = (int)strtol(argv[2], NULL, 0);
		regData.valCount = (int)strtol(argv[3], NULL, 0);

		data = malloc(regData.valCount*sizeof(uint16_t));

		printf("value string: %s\n", argv[4]);
		int currentPosition = 0;
		int idx=0;

		for(idx=0; idx<regData.valCount; idx++){
			len = parser_read_word(argv[4] + currentPosition, sep, &val);
			currentPosition+=len;
			data[idx] = val;
			if (len <= 0)
				break;
		}

		regData.val = data;

		printf("found data values: ");

		for(idx=0; idx<regData.valCount;idx++){
			printf("0x%x,", ((uint16_t *)regData.val)[idx]);
		}
		printf("\n");
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	stat = write_register(regData, bitmask);
	free(data);

	return stat;
}

int execute_read_reg(int argc, char **argv){

	int stat;
	regSetting_t regData;
	regData.valCount = 1;

	if(argc >= 4)
		regData.valCount = (int)strtol(argv[3], NULL, 0);

	if (argc >= 3) {
		regData.addr = (int)strtol(argv[2], NULL, 0);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	stat = read_register(regData);
	return stat;
}
/*//remove
int execute_pgm_nvm_binary(int argc, char **argv){

	assert(epdc->nvm != NULL);

	char* waveform;

	// necessary arguments
	if(argc >= 5)
	{
		strcpy(epdc->nvm->dispId, argv[2]);
		epdc->nvm->vcom = atoi(argv[3]);
		waveform = argv[4];
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	// optional arguments
	if(argc >= 6) { strcpy(epdc->nvm->prodId, argv[5]); }
	if(argc >= 7) { strcpy(epdc->nvm->wfVers, argv[6]); }
	if(argc >= 8) { strcpy(epdc->nvm->fplVers, argv[7]); }
	if(argc >= 9) { strcpy(epdc->nvm->nvmVers, argv[8]); }

	if(argc >= 10) { strcpy(epdc->nvm->feature1, argv[9]); }
	if(argc >= 11) { strcpy(epdc->nvm->feature2, argv[10]); }
	if(argc >= 12) { strcpy(epdc->nvm->feature3, argv[11]); }
	if(argc >= 13) { strcpy(epdc->nvm->feature4, argv[12]); }

	return pgm_nvm(waveform);
}
*/

int execute_info(int argc, char **argv){


	int stat = 0;

	stat = info();

	return stat;
}

int execute_switch_hv(int argc, char **argv){

	int stat;
	int state;

	if (argc >= 3) {
		state = (int)strtol(argv[2], NULL, 0);
	}
	else
	{
		return ERROR_ARGUMENT_COUNT_MISMATCH;
	}

	if (state < 0 || state > 1){
		LOG("Given HV state (%d) not supported.", state);
		return -1;
	}
	stat = switch_hv(state);
	return stat;
}

int print_versionInfo(int argc, char **argv){

	printf("BBepdcULD version = %s\n", VERSION_INFO);

	return 0;
}
// ----------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------

/**
 * Initializes the EPDC.
 *
 * @return status
 */
int start_epdc(int load_nvm_content, int execute_clear)
{

	int stat = 0;
	LOG("load_nvm_content?: %d", load_nvm_content);

	// initialize GPIOs
	if (pl_gpio_config_list(&(hardware->gpios), hardware->board_gpios, hardware->gpio_count)){
		LOG("GPIO init failed");
		return -1;
	}
	// enable VDD
	hardware->gpios.set(hardware->vddGPIO, 1);

	sleep(1);
	if (epdc->init(epdc, load_nvm_content))
		return -1;
	if (execute_clear){
		stat |= epdc->clear_init(epdc);
	}
	return stat;
};

/**
 * De-Initializes the EPDC
 *
 * @return status
 */
int stop_epdc()
{

	printf("stop\n");

	hardware->gpios.set(hardware->vddGPIO, 0);

	// de-configure Epson GPIOs
	if (pl_gpio_deconfigure_list(&(hardware->gpios), hardware->board_gpios, hardware->gpio_count)){
		LOG("GPIO deconfigure failed");
		return -1;
	}

	return 0;
}

/**
 * Sets target vcom
 * @param vcom the target vcom voltage
 * @return status
 */
int set_vcom(int vcom)
{

  printf("vcom %d\n", vcom);
  return epdc->set_vcom(epdc, vcom);
}

/**
 * Sends given waveform data to EPDC.
 * @param waveform path to a waveform file.
 * @param manual set temperature.
 * @return status
 */
int set_waveform(char *waveform, float *temperature)
{

  int do_update = 0;

  if (temperature != NULL && (epdc->controller->temp_mode == PL_EPDC_TEMP_MANUAL))
  {
	  printf("waveform %s, temperature %f\n", waveform, *temperature);
	  epdc->controller->manual_temp = (int)*temperature;
	  do_update = 1;
  }

  if (do_update || (epdc->controller->temp_mode != PL_EPDC_TEMP_MANUAL))
	  epdc->controller->update_temp(epdc->controller);

  if (epdc->controller->load_wflib(epdc->controller, waveform))
	return -1;


  return 0;
}

/**
 * Sends given temperature to EPDC if manual temperature mode is active.
 * @param temperature.
 * @return status
 */
int set_temperature(float temperature)
{


	if(epdc->controller->temp_mode == PL_EPDC_TEMP_MANUAL)
	{
	  printf("temperature %f\n", temperature);
	  epdc->controller->manual_temp = (int)temperature;
	  epdc->controller->update_temp(epdc->controller);
	}
	else
	{
	  LOG("Manual set temperature not possible.\n"
		  "Temperature mode is not set to \"MANUAL\".");
	  return -1;
	}

  return 0;
}

/**
 * Updates image.
 * @param path image path.
 * @param wfID refers to the waveform id.
 * @param mode refers to the update mode, i.e. 0=full update, 1=partial update.
 * @param vcomSwitchEnable enables vcom switch control via epdc, 1=enable, 0=bypass.
 * @param updateCount refers to the count of image updates to execute
 * @param waitTime [ms] refers to the time to wait after one image update
 * @return status
 */
int update_image(char *path, const char* wfID, enum pl_update_mode mode,
		int vcomSwitchEnable, int updateCount, int waitTime) {


	int cnt = 0;

	printf("path: %s\n", path);

	int wfId = pl_generic_controller_get_wfid(epdc->controller, wfID);
	printf("wfID: %d\n", wfId);
	printf("updateMode: %d\n", mode);
	printf("updateCount: %d\n", updateCount);
	printf("waitTime: %d\n", waitTime);
	printf("vcomSwitch: %d\n", vcomSwitchEnable);

	if (wfId == -1)
		return -1;

	if (epdc->controller->load_image(epdc->controller, path, NULL, 0, 0))
		return -1;

	if (epdc->hv->vcomSwitch != NULL){
		if (vcomSwitchEnable == 0){
			epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 0);
		} else {
			epdc->hv->vcomSwitch->disable_bypass_mode(epdc->hv->vcomSwitch);
		}
	}

	for(cnt=0; cnt < updateCount; cnt++)
	{
		if (epdc->update(epdc, wfId, mode, NULL))
			return -1;

		usleep(waitTime * 1000);
	}

	return 0;
}

/**
 * print epdc register values to the command line
 * @param regSetting regSettings structure
 * @return status
 */
int read_register(regSetting_t regSetting){


	uint16_t *data = malloc(sizeof(uint16_t)*regSetting.valCount);
	regSetting.val = data;
	if (regSetting.val == NULL)
		return -1;

	int stat = epdc->read_register(epdc, &regSetting );
	if (stat)
		return -1;

	printf("register addr    = 0x%04X:\n", regSetting.addr);
	printf("register data    = \n");
	dump_hex16(regSetting.val, regSetting.valCount);

	free(regSetting.val);
	return 0;
}

/**
* writes epdc register values
* @param regSetting regSettings structure
* @return status
*/
int write_register(regSetting_t regSetting, const uint32_t bitmask){


	if (regSetting.val == NULL)
		return -1;

	int stat = epdc->write_register(epdc, regSetting, bitmask);
	if (stat)
		return -1;

	return 0;
}

/**
* executes epdc command
* @param regSetting regSettings structure
* @return status
*/
int send_cmd(regSetting_t regSetting){


	if (regSetting.val == NULL)
		return -1;

	int stat = epdc->send_cmd(epdc, regSetting);
	if (stat)
		return -1;

	return 0;
}

/**
 * displays the general display informations.
 */
int info(){


	int stat = 0;
	int isPgm = 0;

	if(epdc->nvm->read_header(epdc->nvm, &isPgm))
		return -1;

	printf("NVM is programmed: %d\n", isPgm);
	printf("Display ID: %s\n", epdc->nvm->dispId);
	printf("Vcom: %d\n", epdc->nvm->vcom);
	printf("Product ID: %s\n", epdc->nvm->prodId);
	printf("Waveform Version: %s\n", epdc->nvm->wfVers);
	printf("FPL Version: %s\n", epdc->nvm->fplVers);
	printf("NVM Version: %s\n", epdc->nvm->nvmVers);
	printf("Feature 1: %s\n", epdc->nvm->feature1);
	printf("Feature 2: %s\n", epdc->nvm->feature2);
	printf("Feature 3: %s\n", epdc->nvm->feature3);
	printf("Feature 4: %s\n", epdc->nvm->feature4);

	return stat;
}

/**
 * switches the high voltages on/off.
 * @param state 0=off, 1=on.
 * @return status
 */
int switch_hv(int state){
	int stat;


	if (state == 1){
		if ((epdc->hv->hvDriver != NULL) && (epdc->hv->hvDriver->switch_on != NULL))
			stat = epdc->hv->hvDriver->switch_on(epdc->hv->hvDriver);
		if ((epdc->hv->vcomDriver != NULL) && (epdc->hv->vcomDriver->switch_on != NULL))
			stat = epdc->hv->vcomDriver->switch_on(epdc->hv->vcomDriver);
	}
	else{
		if ((epdc->hv->vcomDriver != NULL) && (epdc->hv->vcomDriver->switch_off != NULL))
			stat = epdc->hv->vcomDriver->switch_off(epdc->hv->vcomDriver);
		if ((epdc->hv->hvDriver != NULL) && (epdc->hv->hvDriver->switch_off != NULL))
			stat = epdc->hv->hvDriver->switch_off(epdc->hv->hvDriver);
	}

	return stat;
}

/**
 * opens and reads existing binary file into memory and returns the data pointer
 * @param binaryPath path to the binary file.
 * @param blob memory pointer to the data.
 * @return status
 */
int readBinaryFile(const char *binaryPath, uint8_t **blob){
	// read binary blob

	FILE *fs = fopen(binaryPath, "rb");
	int len=0;

	if (fs == NULL){
		LOG("Can't open specified binary file.");
		return -1;
	}

	fseek(fs, 0, SEEK_END);
	len = ftell(fs);
	fseek(fs, 0, SEEK_SET);

	// read file content to blob
	*blob = (uint8_t *)malloc(sizeof(uint8_t)*len);
	int bytecount = fread(*blob, sizeof(uint8_t), len, fs);
	fclose(fs);

	if (bytecount != len){
		LOG("Error during binary file reading.");
		return -1;
	}

	return bytecount;
}
/*//remove
int interface_data(	char* interface,		// device where to find the data (can?)
					int number_of_values, 	// how many values should be visible
					char values){			// wich values should be visisble not implemented
											// (showing the two most significant values)

	int fd, i;
	fd = open(interface, O_RDONLY);
	if(fd < 0){
		LOG("Can't open %s!\n", interface);
		return -1;
	}
	int wfid = 4;
	int fontsize, x, y, angle, w, h;
	struct pl_area area;
	area.height = 960;
	area.width = 1280;
	area.top = 0;
	area.left = 0;
	angle = 270;
	switch(number_of_values){
		case 1:
			fontsize = 300;
			x = 350;
			y = 100;
			w = 1280;
			h = 960;
			break;
		case 2:
			fontsize = 300;
			x = 50;
			y = 100;
			w = 640;
			h = 960;
			break;
		case 4:
			fontsize = 200;
			x = 20;
			y = 50;
			h = 480;
			w = 640;
			break;
		case 6:
			fontsize = 150;
			x = 20;
			y = 50;
			h = 480;
			w = 425;
			break;
		case 8:
			fontsize = 150;
			x = 20;
			y = 50;
			h = 480;
			w = 320;
			break;
		default:{
			LOG("The settings are rubbish!");
			return -1;
		}
	}

	epdc->clear_init(epdc);

	if (epdc->hv->vcomSwitch != NULL){
		epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 1);
	}

	char data[8] = {0,};
	data[0] = 0;
	data[1] = 2;
	data[2] = 4;
	data[3] = 8;
	data[4] = 16;
	data[5] = 32;
	data[6] = 64;
	data[7] = 128;
	while(1){
#if 0
		if(read(fd, data, size) != size 0){
#else
		if(0){
#endif
			return -1;
		}
		for(i=0; i<number_of_values; i++){
			char text[4] = {0,};
			sprintf(text, "%u", data[i]++);
			//printf("showing %s at %i\n", text, i);
			struct pl_area sub_area;
			sub_area.height = h;
			sub_area.width = w;
			sub_area.top = ((i>>1)%2)*h;
			sub_area.left = (i%2)*w;
			//printf("w: %i, h: %i, x: %i, y:  %i\n", sub_area.width, sub_area.height, sub_area.left, sub_area.top);
			if(show_text(epdc->controller, &sub_area, text, FONT0, angle, fontsize, x, y,i%2?1:0))
				return -1;
		}
		if (epdc->update(epdc, wfid,0, &area))
			return -1;
	}


	return 0;
}
//*/
// ----------------------------------------------------------------------
// Counter
// ----------------------------------------------------------------------
int counter(const char* wf)
{

	printf("%s\n",__func__);
	int wfid = 4;
	unsigned char count = 0;
	char counter[10] = {0,};

	if(wf != NULL){
		sscanf(wf, "%i", &wfid);
		LOG("Using Waveform %i" ,wfid);
	}
	struct pl_area area;
	area.height = 960;
	area.width = 1280;
	area.top = 0;
	area.left = 0;
//#if VERBOSE
	LOG("Running counter");
//#endif

	epdc->controller->fill(epdc->controller, &area, 0xFF);
	epdc->clear_init(epdc);

	if (epdc->hv->vcomSwitch != NULL){
		epdc->hv->vcomSwitch->enable_bypass_mode(epdc->hv->vcomSwitch, 1);
	}
	//struct timespec t;

	//*
	while (1) {
		//start_stopwatch(&t);
		sprintf(counter, "%u" ,count++);

		//read_stopwatch(&t, "start loop", 1);
		if(show_text(epdc->controller, &area, counter, FONT0, 270, 300, 50, 100,1))
			return -1;
		//read_stopwatch(&t, "show text", 1);
		if (epdc->update(epdc, wfid,PL_FULL_UPDATE_NOWAIT, NULL))
			return -1;
		//read_stopwatch(&t, "update", 1);
	}
	//*/
	return 0;
}


// ----------------------------------------------------------------------
// Slideshow
// ----------------------------------------------------------------------
int slideshow(const char *path, const char* wf, int waittime)
{


	DIR *dir;
	struct dirent *d;
	int wfid = -1;
	if(wf != NULL){
		sscanf(wf, "%i", &wfid);
		LOG("Using Waveform %i" ,wfid);
	}

	assert(path != NULL);
	int count = 10;
//#if VERBOSE
	LOG("Running slideshow");
//#endif
	//*
	while (count--) {

		if ((dir = opendir(path)) == NULL) {
			LOG("Failed to open directory [%s]", path);
			return -1;
		}

		while ((d=readdir(dir)) != NULL) {
			LOG("%s",d->d_name);
			if(d->d_name[0] == '.')
				continue;


			if (show_image(path, d->d_name, wfid)) {
				LOG("Failed to show image");
				return -1;
			}
			usleep(waittime);

		}
		closedir(dir);
	}
	//*/
	return 0;
}

int show_image(const char *dir, const char *file, int wfid)
{

	char path[256];

	LOG("Image: %s %s", dir, file);
	if(wfid <= -1 || wfid > 14){
		wfid = 2;// = pl_generic_controller_get_wfid(epdc->controller, wfID);
	}
	if (wfid < 0)
		return -1;
	if(dir!=NULL){
		LOG("Dir is not NULL: %s", dir);
		if (join_path(path, sizeof(path), dir, file))
			return -1;
		LOG("Show: %s", path);
		if (epdc->controller->load_image(epdc->controller, path, NULL, 0, 0))
				return -1;
	}else{
		LOG("Dir is NULL: %s", file);
		if (epdc->controller->load_image(epdc->controller, file, NULL, 0, 0))
				return -1;
	}
	if (epdc->update(epdc, wfid,0, NULL))
		return -1;
	return 0;
}

// ----------------------------------------------------------------------
// help messages
// ----------------------------------------------------------------------
void print_application_help(int print_all){

	printf("\n");
	printf("Usage:  BBepdcULD [operation] [parameter]   --> executes an operation \n");
	printf("        BBepdcULD [operation] --help        --> prints detailed help  \n");
	printf("        BBepdcULD [operation] --help -all   --> prints complete help at once \n");
	printf("\n");
	printf("Available Operations:\n");
	int operationIdx = 0;
	for(operationIdx=0; operationIdx<(sizeof(supportedOperations)/sizeof(struct CmdLineOptions)); operationIdx++){
		printf("\t%20s: \t%s\n", supportedOperations[operationIdx].operation, supportedOperations[operationIdx].short_description);
	}

	if (print_all == true){
		printf("\n");
		printf("Detailed Operation Description:\n");

		for(operationIdx=0; operationIdx<(sizeof(supportedOperations)/sizeof(struct CmdLineOptions)); operationIdx++){
			if (supportedOperations[operationIdx].print_help_message != NULL){
				printf("\n");
				printf("%20s: %*s %s\n", supportedOperations[operationIdx].operation, 3, " ", supportedOperations[operationIdx].short_description);
				supportedOperations[operationIdx].print_help_message(25);
			}
		}
	}
	printf("\n");
}

void printHelp_start_epdc(int identLevel){
	printf("%*s Activates and initializes the EPD controller.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -start_epdc [<nvm_flag> [ <clear_flag> ]]]\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<nvm_flag>    	   : \tif 0 = override of waveform and vcom enabled (default).\n", identLevel, " ");
	printf("%*s \t                       \tif 1 = override disabled, use settings from NV memory.\n", identLevel, " ");
	printf("%*s \t<clear_flag>         : \tif 0 = display will not be cleared. (default)\n", identLevel, " ");
	printf("%*s \t                       \tif 1 = display will be cleared with default clear operation.\n", identLevel, " ");
	printf("\n");
}

void printHelp_stop_epdc(int identLevel){
	printf("%*s De-initializes the EPD controller.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -stop_epdc\n", identLevel, " ");
	printf("\n");
}

void printHelp_set_vcom(int identLevel){
	printf("%*s Sets the Vcom voltage.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -set_vcom <voltage>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<voltage>  : \tcom voltage in volts.\n", identLevel, " ");
	printf("\n");
}

void printHelp_set_waveform(int identLevel){
	printf("%*s Sets the waveform used for later update operations.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -set_waveform <waveform> <temp>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<waveform> : \tpath to the waveform file.\n", identLevel, " ");
	printf("%*s \t<temp>     : \tTemperature in degree celsius.\n", identLevel, " ");
	printf("\n");
}

void printHelp_set_temperature(int identLevel){
	printf("%*s Sets the temperature.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -set_temperature <temp>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<temp>     : \tTemperature in degree celsius.\n", identLevel, " ");
	printf("\n");
}

void printHelp_update_image(int identLevel){
	printf("%*s Updates the display with a given image.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -update_image <image>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<image>               : \tpath to the image file.\n", identLevel, " ");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n", identLevel, " ");
	printf("%*s \t<updateMode>          : \tid of the used update mode.\n", identLevel, " ");
	printf("%*s \t<updateCount>         : \tcount of image updates to execute.\n", identLevel, " ");
	printf("%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n", identLevel, " ");
	printf("%*s \t<vcomSwitchEnable>    : \tautomatic vcom switch enable: 0=disable/1=enable.\n", identLevel, " ");
	printf("\n");
}

void printHelp_write_reg(int identLevel){
	printf("%*s Writes to a specified register of the EPD controller.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -write_reg <reg_addr> <datacount> <data> [bitmask]\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<reg_addr> : \tspecifies the address of the register where the data is written.\n", identLevel, " ");
	printf("%*s \t<datacount>: \tspecifies the amount of data portions to be written.\n", identLevel, " ");
	printf("%*s \t<data>     : \tThe data to be written. Data portions must be separated by commata (',').\n", identLevel, " ");
	printf("%*s \t             \tData should be given as one string without any spaces.\n", identLevel, " ");
	printf("%*s \t<bitmask>  : \toptional parameter, which can mask out bits from write operation.\n", identLevel, " ");
	printf("\n");

}

void printHelp_send_cmd(int identLevel){
	printf("%*s Sends a command with specified arguments to the EPD controller.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -send_cmd <cmd> <datacount> <data> [bitmask]\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<cmd> : \tspecifies the cmd to be executed.\n", identLevel, " ");
	printf("%*s \t<datacount>: \toptional paraetmer, specifies the amount of data portions to be written.\n", identLevel, " ");
	printf("%*s \t<data>     : \toptional parameter, arguments to be send. Data portions must be separated by commata (',').\n", identLevel, " ");
	printf("\n");

}

void printHelp_read_reg(int identLevel){

	printf("%*s Reads from a specified register of the EPD controller and prints the values to the shell.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -read_reg <reg_addr> <datacount>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<reg_addr> : \tspecifies the address of the register to be read.\n", identLevel, " ");
	printf("%*s \t<datacount>: \tspecifies the amount of data portions to be read.\n", identLevel, " ");
	printf("\n");
}
/*//remove
void printHelp_pgm_nvm_binary(int identLevel){
	printf("%*s Programs the specified binary to the specific NVM.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -pgm_nvm <displayId> <Vcom> <binary_path>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<display_id>  : specifies display id to be programmed into the nvm.\n", identLevel, " ");
	printf("%*s \t<Vcom>        : specifies the Vcom to be programmed into the nvm.\n", identLevel, " ");
	printf("%*s \t<binary_path> : specifies the path to the binary file to be programmed into the nvm.\n", identLevel, " ");
	printf("%*s \t<product_id>  : optional.\n", identLevel, " ");
	printf("%*s \t<waveform_vers>  : optional.\n", identLevel, " ");
	printf("%*s \t<fpl_vers>    : optional.\n", identLevel, " ");
	printf("%*s \t<nvm_vers>    : optional.\n", identLevel, " ");
	printf("%*s \t<feature1>    : optional.\n", identLevel, " ");
	printf("%*s \t<feature2>    : optional.\n", identLevel, " ");
	printf("%*s \t<feature3>    : optional.\n", identLevel, " ");
	printf("%*s \t<feature4>    : optional.\n", identLevel, " ");
	printf("\n");
}
*/
void printHelp_counter(int identLevel){
	printf("%*s Shows a counting number.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -count\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n", identLevel, " ");
	printf("%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n", identLevel, " ");
	printf("\n");
}

void printHelp_slideshow(int identLevel){
	printf("%*s Updates the display with a slideshow of a given path.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -slideshow <image path>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<image path>          : \tpath to the image files.\n", identLevel, " ");
	printf("%*s \t<wfID>                : \tid of the used waveform id.\n", identLevel, " ");
	printf("%*s \t<waitTime>            : \ttime to wait after each image update [ms].\n", identLevel, " ");
	printf("\n");
}

void printHelp_interface_data(int identLevel){
	printf("%*s Updates the display with a data of a given interface.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -interface <dev path>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<dev path>            : \tpath to the interface. Default is /dev/can1.\n", identLevel, " ");
	printf("%*s \t<number of values>    : \tvalues count to be displayed at a time.\n", identLevel, " ");
	printf("%*s \t<values>              : \tflags which values should be displayed.\n", identLevel, " ");
	printf("\n");
}

void printHelp_info(int identLevel){
	printf("%*s Displays general display informations.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -info\n", identLevel, " ");
	printf("\n");
}

void printHelp_switch_hv(int identLevel){
	printf("%*s Switches HV on/off based on parameter.\n", identLevel, " ");
	printf("\n");
	printf("%*s Usage: BBepdcULD -hv <state>\n", identLevel, " ");
	printf("\n");
	printf("%*s \t<state>  		: 0 = off; 1= on.\n", identLevel, " ");
	printf("\n");
}

void debug_print_parameters(int argc, char **argv)
{

	int paramIdx = 0;
	for(paramIdx = 0; paramIdx < argc; paramIdx++){
		printf("param %d: '%s'\n", paramIdx, argv[paramIdx]);
	}
}
