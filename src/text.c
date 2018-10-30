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
 * text.c -- plain text print app
 *
 * Authors:
 *   Robert Pohlink <robert.pohlink@plasticlogic.com>
 *
 */
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "assert.h"
#include <pl/generic_controller.h>


#include <math.h>
// let's use freetype api ;)
#include <ft2build.h>
#include FT_FREETYPE_H

#define LOG_TAG "text"
#include "utils.h"
#include "pl/types.h"


unsigned char* image;

void
draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, struct pl_area* area, int invert) {
	FT_Int i, j, p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;
	char a,b;
	if(invert){
		a = 0xFF;
		b = 0x00;
	}else{
		a = 0x00;
		b = 0xFF;
	}
	for (i = x, p = 0; i < x_max; i++, p++) {
		for (j = y, q = 0; j < y_max; j++, q++) {
			if (i < 0 || j < 0 || i >= area->width || j >= area->height)
				continue;
			image[j*area->width + i] = bitmap->buffer[q * bitmap->width + p]?b:a;
		}
	}
}

int show_ft_bitmap(struct pl_generic_controller* controller, struct pl_area* area, uint8_t invert) {
	return controller->load_buffer(controller,(char*) image, area);
}

int get_text_area(struct pl_area* area,  const char* text, const char* font,  float text_angle,  int font_size){
		FT_Library library;
		FT_Face face;

		FT_GlyphSlot slot;
		FT_Matrix matrix; /// transformation matrix
		FT_Vector pen; /// untransformed origin
		FT_Error error;

		double angle;
		int target_height;
		int n, num_chars;

		num_chars = strlen(text);
		angle = (text_angle / 360) * 3.14159 * 2;
		target_height = font_size;

		error = FT_Init_FreeType(&library); // initialize library
		if(error) return error;

		error = FT_New_Face(library, font, 0, &face); // create face object
		if(error) return error;

		error = FT_Set_Pixel_Sizes(face, 0, font_size); // set character size
		if(error) return error;

		slot = face->glyph;

		// set up matrix
		matrix.xx = (FT_Fixed) (cos(angle) * 0x10000L);
		matrix.xy = (FT_Fixed) (-sin(angle) * 0x10000L);
		matrix.yx = (FT_Fixed) (sin(angle) * 0x10000L);
		matrix.yy = (FT_Fixed) (cos(angle) * 0x10000L);

		pen.x = 0;
		pen.y = target_height * 5 / 4;

		for (n = 0; n <= num_chars; n++) {

			// set transformation
			FT_Set_Transform(face, &matrix, &pen);

			// load glyph image into the slot (erase previous one)
			error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
			if(!error){
				FT_Bitmap* bitmap = &slot->bitmap;
				FT_Int x = slot->bitmap_left;
				FT_Int y = target_height - slot->bitmap_top;
				FT_Int x_max = x + bitmap->width;
				FT_Int y_max = y + bitmap->rows;

				// convert position
				if(x_max > area->width){
					//LOG("x:Text doesn't fit area: have: %ix%i and need %ix%i", area->width, area->height, x_max, y_max);
					area->width = x_max;
				}
				if(y_max > area->height){
					//LOG("y:Text doesn't fit area: have: %ix%i and need %ix%i", area->width, area->height, x_max, y_max);
					area->height = y_max;
				}
			}
			// increment pen position
			pen.x += slot->advance.x;
			pen.y += slot->advance.y;

		}

		if(error) return error;
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		return 0;

}

int show_text(struct pl_generic_controller* controller, struct pl_area* area, const char* text,
		const char* font, float text_angle, int font_size, int x, int y, uint8_t invert) {
	int i,h,w;
	FT_Library library;
	FT_Face face;

	FT_GlyphSlot slot;
	FT_Matrix matrix; /// transformation matrix
	FT_Vector pen; /// untransformed origin
	FT_Error error;

	double angle;
	int target_height;
	int n, num_chars;
	controller->get_resolution(controller, &w, &h);
	image = (unsigned char*) malloc(w*h);
	for(i=0; i<(h * w); i++) image[i] = invert?0xFF:0x00;
	num_chars = strlen(text);
	angle = (text_angle / 360) * 3.14159 * 2;
	target_height = font_size;
	error = FT_Init_FreeType(&library); // initialize library
	if(error) return error;

	error = FT_New_Face(library, font, 0, &face); // create face object
	if(error) return error;

	error = FT_Set_Pixel_Sizes(face, 0, font_size); // set character size
	if(error) return error;

	slot = face->glyph;

	// set up matrix
	matrix.xx = (FT_Fixed) (cos(angle) * 0x10000L);
	matrix.xy = (FT_Fixed) (-sin(angle) * 0x10000L);
	matrix.yx = (FT_Fixed) (sin(angle) * 0x10000L);
	matrix.yy = (FT_Fixed) (cos(angle) * 0x10000L);

	pen.x = 0;
	pen.y = target_height;

	for (n = 0; n < num_chars; n++) {
		// set transformation
		FT_Set_Transform(face, &matrix, &pen);

		// load glyph image into the slot (erase previous one)
		error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);

		if (error)
			continue; // ignore errors

		// now, draw to our target surface (convert position)
		draw_bitmap(&slot->bitmap,
				slot->bitmap_left,
				target_height - slot->bitmap_top, area, invert);
		// increment pen position
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	error = show_ft_bitmap(controller, area, invert);

	if(error) return error;
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	free(image);
	image = NULL;
	return 0;
}
