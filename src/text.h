/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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

#ifdef __cplusplus
}
#endif

#endif /* TEXT_H */

