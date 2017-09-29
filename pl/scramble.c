/*
 * scramble.c
 *
 *  Created on: 08.01.2015
 *      Author: sebastian.friebe
 */

#include <stdint.h>
#include "scramble.h"

static int calcPixelIndex(int gl, int sl, int slCount);

void scramble_array(uint8_t* source, uint8_t* target, int *glCount, int *slCount, int scramblingMode){

	int sl,gl;
	int targetIdx;
	int sourceIdx;
	int _glCount = *glCount;
	int _slCount = *slCount;
	int __glCount;
	int __slCount;

	if (scramblingMode == 0){
		// no need to scramble image data, just copy
		memcpy(target, source, _glCount * _slCount);
	}
	else {
		// need to scramble image data based on scrambling mode
		for(gl=0; gl< _glCount; gl++)
		{
			for(sl=0; sl< _slCount; sl++)
			{
				__glCount = _glCount;
				__slCount = _slCount;

				targetIdx = calcScrambledIndex(scramblingMode, gl, sl , &__glCount, &__slCount);
				sourceIdx = calcPixelIndex(gl, sl, _slCount);
				target[targetIdx] = source[sourceIdx];
			}
		}
		*glCount = __glCount;
		*slCount = __slCount;
	}
}

int calcScrambledIndex(int scramblingMode, int gl, int sl, int *glCount, int *slCount){
	// set starting values
	int newGlIdx = gl;
	int newSlIdx = sl;
	int _glCount = *glCount;
	int _slCount = *slCount;

	// source line scrambling for half nbr of gate lines and double nbr of source lines
	// scrambling between the scrambling resolution
	if (scramblingMode & SCRAMBLING_SOURCE_SCRAMBLE_MASK)
	{
		_glCount = _glCount/2;
		_slCount = _slCount*2;

		if(scramblingMode & SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_MASK)
		{
			newSlIdx = (newGlIdx%2) ? newSlIdx*2 : (newSlIdx*2+1);
			newGlIdx = newGlIdx/2;
		}
		else
		{
			newSlIdx = (newGlIdx%2) ? (newSlIdx*2+1) : newSlIdx*2;
			newGlIdx = newGlIdx/2;
		}
	}
	// gate line scrambling for half nbr of source lines and double nbr of gate lines
	else if (scramblingMode & SCRAMBLING_GATE_SCRAMBLE_MASK){

		_glCount = _glCount*2;
		_slCount = _slCount/2;

		if(scramblingMode & SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_MASK){
			// scrambling between image resolution and scrambling resolution
			// move every even source line to the next gate line
			// by bisect the source line index
			newGlIdx = (newGlIdx*2) + (newSlIdx+1)%2;
			newSlIdx = (newSlIdx/2);
		}
		else{
			// scrambling between image resolution and scrambling resolution
			// move every odd source line to the next gate line
			// by bisect the source line index
			newGlIdx = (newGlIdx*2) + newSlIdx%2;
			newSlIdx = (newSlIdx/2);
		}
	}

	// check for difference in source interlaced setting
	if (scramblingMode & SCRAMBLING_SOURCE_INTERLACED_MASK){
		if(scramblingMode & SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_MASK){
			newSlIdx = ((newSlIdx+1) % 2) ? ((newSlIdx/2)+_slCount/2) : (newSlIdx/2);
		}
		else{
			newSlIdx = ((newSlIdx) % 2) ? ((newSlIdx/2)+_slCount/2) : (newSlIdx/2);
		}
	}

	// mirror the second source line part
	if (scramblingMode & SCRAMBLING_SOURCE_MIRROW_MASK){
		if(newSlIdx >= _slCount/2){
			newSlIdx = (_slCount-1) - (newSlIdx - (_slCount/2)) ;
		}
	}

	// check for difference in source direction setting
	if (scramblingMode & SCRAMBLING_SOURCE_DIRECTION_MASK){
		newSlIdx = _slCount-newSlIdx-1;
	}

	// check for difference in source direction setting
	if (scramblingMode & SCRAMBLING_SOURCE_START_MASK){
		newSlIdx = (newSlIdx+_slCount/2)%_slCount;
	}

	// check for difference in gate direction setting
	if (scramblingMode & SCRAMBLING_GATE_DIRECTION_MASK){
		newGlIdx = _glCount-newGlIdx-1;
	}

	*glCount = _glCount;
	*slCount = _slCount;

	return calcPixelIndex(newGlIdx, newSlIdx, _slCount);
}

static int calcPixelIndex(int gl, int sl, int slCount)
{
	return gl*slCount+sl;
}

