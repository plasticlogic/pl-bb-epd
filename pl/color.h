/*
 * color.h
 *
 *  Created on: 17.10.2017
 *      Author: robert.pohlink
 */

#ifndef PL_COLOR_H_
#define PL_COLOR_H_

#include <pl/utils.h>

int rgbw_processing(uint32_t *src_width, uint32_t *src_height,
			      void *src_buf_virt, uint8_t *dst_buf_virt,
			      struct pl_area *src_update_region, cfa_overlay_t cfa_overlay);

#endif /* PL_COLOR_H_ */
