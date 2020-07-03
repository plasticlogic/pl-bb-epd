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
  Solomon ePDC command interpreter - solomon-ci
  Copyright (C) 2013 Plastic Logic Limited.  All rights reserved.
*/

#ifndef GLOBAL_H
#define	GLOBAL_H

//******************************************************************************
// Includes
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>


#ifdef	__cplusplus
extern "C" {
#endif

//******************************************************************************
// Constants
//******************************************************************************

#define FALSE                         ( 0 )
#define TRUE                          ( !FALSE )

//******************************************************************************
// Definitions
//******************************************************************************

typedef unsigned char   bool;
/*typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned long   uint32_t;
*/

//******************************************************************************
// Function Prototypes
//******************************************************************************



#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

