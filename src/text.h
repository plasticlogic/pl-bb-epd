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
 * File:   text.h
 * Author: robert.pohlink
 *
 * Created on February 2, 2016, 1:08 PM
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "assert.h"

#include <math.h>
#include <errno.h>

#ifndef TEXT_H
#define TEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#define FONT0 "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf"
#define FONT1 "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans-Bold.ttf"
#define FONT2 "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf"
#define FONT3 "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf"
#define FONT4 "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif-Bold.ttf"
#define FONT5 "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif.ttf"


int show_text(struct pl_generic_controller* controller, struct pl_area* area, const char* text, const char* font, float text_angle, int font_size, int x, int y, uint8_t invert);
int get_text_area(struct pl_area* area,  const char* text, const char* font,  float text_angle,  int font_size);

#ifdef __cplusplus
}
#endif

#endif /* TEXT_H */

