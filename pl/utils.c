/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

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
 * utils.c -- random homeless functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

//#include <pl/endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "assert.h"
#define LOG_TAG "utils"
#include "utils.h"
#include "parser.h"
#include <libpng-1.2.51/png.h>

void swap32(void *x)
{
	uint8_t *b = x;
	uint8_t tmp;

	tmp = b[0];
	b[0] = b[3];
	b[3] = tmp;
	tmp = b[1];
	b[1] = b[2];
	b[2] = tmp;
}

void swap32_array(int32_t **x, uint16_t n)
{
	while (n--)
		swap32(*x++);
}

void swap16(void *x)
{
	uint8_t *b = x;
	uint8_t tmp;

	tmp = b[0];
	b[0] = b[1];
	b[1] = tmp;
}

void swap16_array(int16_t *x, uint16_t n)
{
	while (n--)
		swap16(x++);
}

int is_file_present(const char *path)
{

	return 1;
}

int join_path(char *path, size_t n, const char *dir, const char *file)
{
	return (snprintf(path, n, "%s/%s", dir, file) >= n) ? -1 : 0;
}

int open_image(const char *dir, const char *file, FILE *f,
	       struct pnm_header *hdr)
{
	char path[MAX_PATH_LEN];

	if (snprintf(path, MAX_PATH_LEN, "%s/%s", dir, file) >= MAX_PATH_LEN) {
		LOG("File path is too long, max=%d", MAX_PATH_LEN);
		return -1;
	}

/**
	if (f_open(f, path, FA_READ) != FR_OK) {
		LOG("Failed to open image file");
		return -1;
	}*/

	/**
	if (pnm_read_header(f, hdr) < 0) {
		LOG("Failed to parse PGM header");
		return -1;
	}*/

	return 0;
}

/* ----------------------------------------------------------------------------
 * Debug utilies
 */


void abort_now(const char *abort_msg, enum abort_error error_code);

void abort_now(const char *abort_msg, enum abort_error error_code)
{
	if (abort_msg != NULL)
		fprintf(stderr, "%s\r\n", abort_msg);
}

static void do_abort_msg(const char *file, unsigned line,
			 const char *error_str, const char *message,
			 enum abort_error error_code)
{
	/* Following conversion of line to a string is a workaround
	 * for a problem with fprintf(stderr, "%u", line) that only
	 * occurs when NOT debugging and prevents further code execution
	 * (possibly a heap size issue?)
	 */
	char temp[16];
	sprintf(temp, "%u", line);
	fprintf(stderr, "%s, line %s: %s\n", file, temp, message);

	abort_now(error_str, error_code);
}

void do_abort_msg_assert(const char *file, unsigned line, const char *message)
{
	do_abort_msg(file, line, "Assertion failed\n", message, ABORT_ASSERT);
}

void do_abort_msg_error(const char *file, unsigned line, const char *message, enum abort_error error_code)
{
	do_abort_msg(file, line, "Fatal error\n", message, error_code);
}

void dump_hex(const void *data, uint16_t len)
{
	static const char hex[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	char s[] = "[XXXX] XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX";
	char *cur;
	uint16_t i;

	if (!len)
		return;

	for (i = 0, cur = s; i < len; ++i) {
		const uint8_t byte = ((const uint8_t *)data)[i];

		if (!(i & 0xF)) {
			uint16_t addr = i;
			uint16_t j;

			if (i)
				puts(s);

			cur = s + 4;

			for (j = 4; j; --j) {
				*cur-- = hex[addr & 0xF];
				addr >>= 4;
			}

			cur = s + 7;
		}

		*cur++ = hex[byte >> 4];
		*cur++ = hex[byte & 0xF];
		++cur;
	}

	i %= 16;

	if (i) {
		cur = s + 6 + (i * 3);
		*cur++ = '\n';
		*cur++ = '\0';
	}

	puts(s);
}

void dump_hex16(const void *data, uint16_t len)
{
	static const char hex[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	char s[] = "[XXXX] XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX";
	char *cur;
	uint16_t i;

	if (!len)
		return;

	for (i = 0, cur = s; i < len; ++i) {
		const uint16_t word = ((const uint16_t *)data)[i];

		if (!(i & 0xF)) {
			uint16_t addr = i;
			uint16_t j;

			if (i)
				puts(s);

			cur = s + 4;

			for (j = 4; j; --j) {
				*cur-- = hex[addr & 0xF];
				addr >>= 4;
			}

			cur = s + 7;
		}

		*cur++ = hex[(word >> 12) & 0xF];
		*cur++ = hex[(word >> 8) & 0xF];
		*cur++ = hex[(word >> 4) & 0xF];
		*cur++ = hex[word & 0xF];
		++cur;
	}

	i %= 16;

	if (i) {
		cur = s + 6 + (i * 5);
		*cur++ = '\n';
		*cur++ = '\0';
	}

	puts(s);
}

int read_png(const char* file_name, png_byte ** image_ptr, int * width, int * height)
{
  int ERROR = -1;

  LOG("filename %s", file_name);

  png_structp png_ptr;
  png_infop info_ptr;
  FILE *fp;

  if ((fp = fopen(file_name, "rb")) == NULL)
     return (ERROR);

  /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return (ERROR);
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return (ERROR);
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return (ERROR);
   }

   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   // read the header
  png_read_info(png_ptr, info_ptr);

  *width = (int)png_get_image_width(png_ptr, info_ptr);
  *height = (int) png_get_image_height(png_ptr, info_ptr);
  int _bit_depth = (int) png_get_bit_depth(png_ptr, info_ptr);
  int _channels = (int) png_get_channels(png_ptr, info_ptr);

  LOG("width %d, height %d, bit_depth %d, channels %d", *width, *height, _bit_depth, _channels);

  png_bytep row_pointers[*height];

  int row;
  /* Clear the pointer array */
  for (row = 0; row < (*height); row++)
     row_pointers[row] = NULL;

  for (row = 0; row < (*height); row++)
     row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr,
        info_ptr));

  /* Now it's time to read the image.  One of these methods is REQUIRED */
  /* Read the entire image in one go */
  png_read_image(png_ptr, row_pointers);

  png_read_end(png_ptr, info_ptr);

  // copy rows to buffer
  png_byte * image_buffer;
  image_buffer = malloc((*height)*(*width)*sizeof(png_byte));

  int h, w;
  for(h=0; h<(*height); h++)
    for (w=0; w<(*width); w++)
    {
        image_buffer[h*(*width)+w] = (png_byte) *(row_pointers[h]+_channels*w);
        //image[h*_width+w] = (uint8_t) *(row_pointers[h]+w);
    }

  *image_ptr = image_buffer;

  /* Clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

  /* Close the file */
  fclose(fp);

  return 0;
}

int read_rgbw_png(const char* file_name, rgbw_pixel_t ** image_ptr, int * width, int * height)
{
  int ERROR = -1;

  LOG("filename %s", file_name);

  png_structp png_ptr;
  png_infop info_ptr;
  FILE *fp;

  if ((fp = fopen(file_name, "rb")) == NULL)
     return (ERROR);

  /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return (ERROR);
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return (ERROR);
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return (ERROR);
   }

   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   // read the header
  png_read_info(png_ptr, info_ptr);

  *width = (int)png_get_image_width(png_ptr, info_ptr);
  *height = (int) png_get_image_height(png_ptr, info_ptr);
  int _bit_depth = (int) png_get_bit_depth(png_ptr, info_ptr);
  int _channels = (int) png_get_channels(png_ptr, info_ptr);
  int color_type = (int) png_get_color_type(png_ptr, info_ptr);
  uint8_t color_offset;
  switch(color_type){
	  case 2:{
		  color_offset = 3;
		  break;
	  }
	  case 6:
	  default:{
		  color_offset = 4;
		  break;
	  }
  }

  LOG("width %d, height %d, bit_depth %d, channels %d, color type: %d", *width, *height, _bit_depth, _channels, color_type);

  png_bytep row_pointers[*height];

  int row;
  /* Clear the pointer array */
  for (row = 0; row < (*height); row++)
     row_pointers[row] = NULL;

  for (row = 0; row < (*height); row++)
     row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr,
        info_ptr));

  /* Now it's time to read the image.  One of these methods is REQUIRED */
  /* Read the entire image in one go */
  png_read_image(png_ptr, row_pointers);

  png_set_expand(png_ptr);

  png_read_end(png_ptr, info_ptr);

  // copy rows to buffer
  rgbw_pixel_t * image_buffer;
  image_buffer = malloc((*height)*(*width)*sizeof(rgbw_pixel_t));

  int h, w;
  for(h=0; h<(*height); h++)
    for (w=0; w<(*width); w++)
    {
    	image_buffer[h*(*width)+w].r = (uint8_t) *(row_pointers[h]+0+color_offset*w);
    	image_buffer[h*(*width)+w].g = (uint8_t) *(row_pointers[h]+1+color_offset*w);
    	image_buffer[h*(*width)+w].b = (uint8_t) *(row_pointers[h]+2+color_offset*w);
    	if(color_type & 4){
    		image_buffer[h*(*width)+w].w = (uint8_t) *(row_pointers[h]+3+color_offset*w);

    	}else{
			image_buffer[h*(*width)+w].w = (uint8_t) (((image_buffer[h*(*width)+w].r * 299) + (image_buffer[h*(*width)+w].g * 587) + (image_buffer[h*(*width)+w].b * 114)) / 1000);
		}
    	/*
    	if(w==0 || w==1){
    		LOG("W: %i, 0x%02X%02X%02X%02X", h, image_buffer[h*(*width)+w].r,image_buffer[h*(*width)+w].g,image_buffer[h*(*width)+w].b,image_buffer[h*(*width)+w].w);
    	}
    	//*/
    	/*
		if(w==0 && h==0){
			LOG("W: %i, 0x%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X", h, *(row_pointers[h]+0),*(row_pointers[h]+1),*(row_pointers[h]+2),*(row_pointers[h]+3),*(row_pointers[h]+4),*(row_pointers[h]+5),*(row_pointers[h]+6),*(row_pointers[h]+7), *(row_pointers[h]+8),*(row_pointers[h]+9),*(row_pointers[h]+10),*(row_pointers[h]+11),*(row_pointers[h]+12),*(row_pointers[h]+13),*(row_pointers[h]+14),*(row_pointers[h]+15));
			LOG("W: %i, 0x%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X", h, *(row_pointers[h]+16),*(row_pointers[h]+17),*(row_pointers[h]+18),*(row_pointers[h]+19),*(row_pointers[h]+20),*(row_pointers[h]+21),*(row_pointers[h]+22),*(row_pointers[h]+23), *(row_pointers[h]+24),*(row_pointers[h]+25),*(row_pointers[h]+26),*(row_pointers[h]+27),*(row_pointers[h]+28),*(row_pointers[h]+29),*(row_pointers[h]+30),*(row_pointers[h]+31));
		}
		//*/

    }

  *image_ptr = image_buffer;

  /* Clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

  /* Close the file */
  fclose(fp);

  return 0;
}

int read_register_settings_from_file(const char* filename, regSetting_t** ptr_to_settings){

	static const char sep[] = ", ";
	FILE *file;
	unsigned int val;
	size_t currentValueCount;

	int settingsCount = 0;
	size_t mem_alloc_item_count = 10;

	file = fopen(filename, "r");
	if (file == NULL) {
		printf("Specified register override file '%s' could not be opened.\n", filename);
		return -1;
	}

	size_t count = 81;
	char* line = malloc(count * sizeof(char));
	int len;

	regSetting_t *settings = (regSetting_t *)calloc(mem_alloc_item_count, sizeof(regSetting_t));
	if (settings==NULL){
		printf("Couldn't allocate enough memory.\n");
		return -2;
	}

	while ((len = getline(&line, &count, file))>=0) {

		if (line[len - 1] == '\n') {
			line[len - 1] = '\0';
		}
		if (len < 0) {
			if (!feof(file))
				LOG("Failed to read line");
			break;
		}

		if ((line[0] == '\0') || (line[0] == '#')) {
			continue;
		}

		// check if we need to allocate more memory
		if (settingsCount >= mem_alloc_item_count){
			mem_alloc_item_count *= 2;
			settings = (regSetting_t *)realloc(settings, mem_alloc_item_count*sizeof(regSetting_t));
			if (settings==NULL){
				printf("Couldn't allocate enough memory.\n");
				return -2;
			}
		}

		len = parser_read_word(line, sep, &(settings[settingsCount].addr));
		int positionInLine = len;
		if (len <= 0){
			printf("Error reading address.\n");
			return -3;
		}

		len = parser_read_word(line + positionInLine, sep, &(settings[settingsCount].valCount));
		positionInLine += len;
		if (len <= 0){
			printf("Error reading value count.\n");
			return -3;
		}

		uint16_t *data = malloc(settings[settingsCount].valCount*sizeof(uint16_t));
		settings[settingsCount].val = data;
		for(currentValueCount=0; currentValueCount<settings[settingsCount].valCount; currentValueCount++){
			len = parser_read_word(line + positionInLine, sep, &val);
			positionInLine += len;
			if (len <= 0){
				printf("Error reading value.\n");
				return -3;
			}

			data[currentValueCount] =  val;
		}

		settingsCount++;
	}

	// free memory of line read from file
	free(line);
	fclose(file);

	*ptr_to_settings = settings;
	return settingsCount;
}

/**
 * @brief Print out all register settings from the register object
 *
 * @param settings pointer to  list of register settings
 * @param n number of register settings to be dumped
 */
void dump_register_settings(regSetting_t *settings, int n){
	int i,j;

	for(i = 0; i< n; i++){
		printf("%d %x : %i ", i, settings[i].addr, settings[i].valCount);
		for(j = 0; j<settings[i].valCount; j++){
			printf(": %x ", settings[i].val[j]);
		}
		printf("\n");
	}
}

/**
 * @brief Enhances strcpy function with additional max character count. Since sometimes is it
 * necessary to copy strings with "unknown" length to a size limited target.
 * The parameter maxStrLen means the pure string len without "\0"!
 * If the string length from "to" exceeds the maxStrLen count, the first maxStrLen characters plus "\0" will be copied.
 * The function returns "0" if parameter max was not exceeded and otherwise "-1";
 *
 */
int maxstrcpy(char* to, char* from, size_t maxStrLen)
{
	if(strlen(from) <= maxStrLen)
	{
		strcpy(to, from);
	}
	else
	{
		memcpy(to, from, maxStrLen);
		to[maxStrLen] = '\0';
		return -1;
	}

	return 0;
}

/**
 * @brief Enhances strcpy function with additional max character count.
 * A strandard strcpy will be done if the string length plus "\0" does not exceed the maxMemSize.
 * If the string length plus "\0" exceeds the maxMemSize only the count of maxMemSize characters will be copied even without the "\0".
 * The usecase of this function is i.e. the storing of informations inside a nvm structure.
 * The data fields have known max. sizes. That means the "\0" sign will not be stored if the string length is same as
 * max field size. Otherwise if the string is shorter than the max field size, "\0" defines where the string ends.
 *
 */
int maxstr2memcpy(char* to, char* from, size_t maxMemSize)
{
	if(strlen(from) < maxMemSize)
	{
		strcpy(to, from);
	}
	else
	{
		memcpy(to, from, maxMemSize);
	}

	if (strlen(to) > maxMemSize)
	{
		return -1;
	}

	return 0;
}

void start_stopwatch(struct timespec* starttime){
#if 1
	struct timespec clock_resolution;
	clock_getres(CLOCK_REALTIME, &clock_resolution);
	//printf("Resolution  von CLOCK_REALTIME ist %ld Sekunden, %ld Nanosekunden\n",
	//	clock_resolution.tv_sec, clock_resolution.tv_nsec);
	clock_gettime(CLOCK_REALTIME, starttime);
#endif
}

unsigned long long read_stopwatch(struct timespec* starttime, char* label, int reset){
	struct timespec readtime;
	unsigned long long elapsedTime = 0;
#if 1

	clock_gettime(CLOCK_REALTIME, &readtime);
	elapsedTime = ((readtime.tv_sec * 1000000000L) + readtime.tv_nsec)
				- ((starttime->tv_sec * 1000000000L) + starttime->tv_nsec);
	printf("Action %s took %lld us\n",label!=NULL?label:"", (unsigned long long)elapsedTime/1000);
	if (reset) clock_gettime(CLOCK_REALTIME, starttime);
#endif
	return elapsedTime;
}

uint8_t get_rgbw_pixel_value(uint8_t pixel_position, cfa_overlay_t cfa_overlay, rgbw_pixel_t pixel){
	if(cfa_overlay.r_position == pixel_position)
		return pixel.r;
	if(cfa_overlay.g_position == pixel_position)
		return pixel.g;
	if(cfa_overlay.b_position == pixel_position)
		return pixel.b;
	if(cfa_overlay.w_position == pixel_position)
		return pixel.w;
	return -1;
}
