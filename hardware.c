/*
 * hardware.c
 *
 *  Created on: 15 Nov 2012
 *      Author: jacko
 */

#include "hardware.h"

//an 8-bit quick multiply
u8 mul(u8 x, u8 y) {
	u16 i = 0;
	if(x & 1) i += y;
	i >>= 1;
	if(x & 2) i += y;
	i >>= 1;
	if(x & 4) i += y;
	i >>= 1;
	if(x & 8) i += y;
	i >>= 1;
	if(x & 16) i += y;
	i >>= 1;
	if(x & 32) i += y;
	i >>= 1;
	if(x & 64) i += y;
	i >>= 1;
	if(x & 128) i += y;
	return (u8)(i >> 1);  
}

