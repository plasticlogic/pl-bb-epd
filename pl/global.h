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

