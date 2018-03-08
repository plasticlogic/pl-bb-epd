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
 * color.c
 *
 *  Created on: 17.10.2017
 *      Author: robert.pohlink
 */

#define LOG_TAG "color"
#include <pl/utils.h>
#define VERBOSE 1

int rgbw_processing(uint32_t *src_width, uint32_t *src_height,
			      void *src_buf_virt, uint8_t *dst_buf_virt,
			      struct pl_area *src_update_region, cfa_overlay_t cfa_overlay)
{
	rgbw_pixel_t *src, *srcp;
	uint8_t *dst_line_start, *dst1, *dst2;
	int i,j;
	int dst_line_step = 0, dst_step = 0, dst_stride = 0;

	if(src_update_region == NULL){
		src_update_region = malloc(sizeof(struct pl_area));
		src_update_region->height = *src_height;
		src_update_region->width = *src_width;
		src_update_region->left = 0;
		src_update_region->top = 0;
#if VERBOSE
		LOG("Redefining area...");
#endif
	}

#if VERBOSE
	LOG("Starting CFA translation");
	LOG("Source buf @%p, destination buf @%p", src_buf_virt, dst_buf_virt);
	LOG("Source buffer %ux%u pixels, update region is (%u,%u)+%ux%u (userspace size)",
			*src_width, *src_height, src_update_region->left, src_update_region->top,
			src_update_region->width, src_update_region->height);
#endif
	src = (rgbw_pixel_t *)src_buf_virt + src_update_region->top * *src_width + src_update_region->left;
	dst_line_start = dst_buf_virt;

	dst_stride = 2*src_update_region->width;
	dst_line_start += 0;
	dst_line_step = dst_stride * 2;
	dst_step = 1;

#if VERBOSE
	LOG("rotate: %d, dst_stride = %d, dst_line_start = 0x%p, dst_line_step = %d, dst_step = %d",
			0, dst_stride, dst_line_start, dst_line_step, dst_step);
#endif

	for (i = 0; i < src_update_region->height; i++) {
		srcp = src;

		dst1 = dst_line_start;
		dst2 = dst_line_start + dst_stride;

		for (j = 0; j < src_update_region->width; j++) {
			rgbw_pixel_t val;
			val = *srcp;


			*dst1 = (uint8_t) get_rgbw_pixel_value(0, cfa_overlay, val);// val.g;//r << 8;
			*dst2 = (uint8_t) get_rgbw_pixel_value(2, cfa_overlay, val);//val.b;//(w & 0xFF) << 8;
			dst1 += dst_step;
			dst2 += dst_step;
			*dst1 = (uint8_t) get_rgbw_pixel_value(1, cfa_overlay, val);//val.r;//g;
			*dst2 = (uint8_t) get_rgbw_pixel_value(3, cfa_overlay, val);//val.w;//b;
			srcp++;
			dst1 += dst_step;
			dst2 += dst_step;

		}
		src += *src_width;

		dst_line_start += dst_line_step;
	}
	src_update_region->height *= 2;
	src_update_region->width *= 2;
	src_update_region->left *= 2;
	src_update_region->top *= 2;
	*src_width *=2;
	*src_height *= 2;
#if VERBOSE
	LOG("Dest buffer %ux%u pixels, update region is (%u,%u)+%ux%u (userspace size)",
				*src_width, *src_height, src_update_region->left, src_update_region->top,
				src_update_region->width, src_update_region->height);
#endif
	return 0;
}
