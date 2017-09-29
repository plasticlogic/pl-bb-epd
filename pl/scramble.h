/*
 * scramble.h
 *
 *  Created on: 08.01.2015
 *      Author: sebastian.friebe
 */

#ifndef SCRAMBLE_H_
#define SCRAMBLE_H_

/**
 * defines a structure representing a array scrambling configuration
 *
 * source_interlaced: defines whether the data will be interlaced or not
 *                    0 -> no interlacing:  S0, S1, S2, S3, S4, ...
 *                    1 -> interlacing:     S0, Sn, S1, Sn+1, S2, Sn+2, ...  (with n = slCount/2)
 *
 * source_direction: direction of source data
 * 					  0 -> upwards: 		S0, S1, S2, S3, ...
 * 					  1 -> downwards: 		Sn, Sn-1, Sn-2, Sn-3, ...        (with n = slCount)
 *
 * source_start: starting position for source outputs, either left (default) or right
 * 					  0 -> left:			S0 is output first
 * 					  1 -> right:			Sn is output first 				 (with n = slCount/2)
 *
 * gate_direction: direction of gate data
 * 					  0 -> upwards: 		G0, G1, G2, G3, ...
 * 					  1 -> downwards: 		Gn, Gn-1, Gn-2, Gn-3, ...		 (with n = glCount)
 *
 */

#define SCRAMBLING_SOURCE_START_BIT				0
#define SCRAMBLING_SOURCE_INTERLACED_BIT		1
#define SCRAMBLING_SOURCE_DIRECTION_BIT			2
#define SCRAMBLING_GATE_DIRECTION_BIT			3
#define SCRAMBLING_SOURCE_SCRAMBLE_BIT			4	// source line is connected to every 2nd pixel
#define SCRAMBLING_GATE_SCRAMBLE_BIT			5	// gate line is connected to every 2nd pixel
#define SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_BIT	6	// if is set the odd lines will be first
#define SCRAMBLING_SOURCE_MIRROW_BIT 			7
#define SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_BIT 8

#define SCRAMBLING_SOURCE_START_MASK			(1 << SCRAMBLING_SOURCE_START_BIT)
#define SCRAMBLING_SOURCE_INTERLACED_MASK 		(1 << SCRAMBLING_SOURCE_INTERLACED_BIT)
#define SCRAMBLING_SOURCE_DIRECTION_MASK		(1 << SCRAMBLING_SOURCE_DIRECTION_BIT)
#define SCRAMBLING_GATE_DIRECTION_MASK			(1 << SCRAMBLING_GATE_DIRECTION_BIT)
#define SCRAMBLING_SOURCE_SCRAMBLE_MASK			(1 << SCRAMBLING_SOURCE_SCRAMBLE_BIT)
#define SCRAMBLING_GATE_SCRAMBLE_MASK			(1 << SCRAMBLING_GATE_SCRAMBLE_BIT)
#define SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_MASK	(1 << SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_BIT)
#define SCRAMBLING_SOURCE_MIRROW_MASK 			(1 << SCRAMBLING_SOURCE_MIRROW_BIT)
#define SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_MASK (1 << SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_BIT)

/** copies data from source to target array while applying a scrambling algorithm
 * S0, Sn, S1, Sn+1, S2, Sn+2; with n = slCount/2;
 * no scrambling in gate direction
 * Expects data in source array as sourceline fast addressed and starting with gate=0 and source=0.
 */
void scramble_array(uint8_t* source, uint8_t* target, int *glCount, int *slCount, int scramblingMode);

int calcScrambledIndex(int scramblingMode, int gl, int sl, int *glCount, int *slCount);

#endif /* SCRAMBLE_H_ */
