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
 * color.h
 *
 *  Created on: 17.10.2017
 *      Author: robert.pohlink
 */

#ifndef PL_COLOR_H_
#define PL_COLOR_H_

#include <pl/utils.h>
#include <errno.h>

int rgbw_processing(uint32_t *src_width, uint32_t *src_height,
			      void *src_buf_virt, uint8_t *dst_buf_virt,
			      struct pl_area *src_update_region, cfa_overlay_t cfa_overlay);

#endif /* PL_COLOR_H_ */
