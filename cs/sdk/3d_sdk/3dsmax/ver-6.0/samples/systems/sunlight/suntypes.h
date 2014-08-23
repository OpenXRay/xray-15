/*===========================================================================*\
	FILE: suntypes.h

	DESCRIPTION: Some type defineitions for the sunlight system.

	HISTORY: Adapted by John Hutchinson 10/08/97 
			

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/

typedef struct _interpJulianStruct { 
    double days; 
    int subday;
	long epoch;
} interpJulianStruct; 

typedef struct _uTimevect { 
    unsigned short i; 
    unsigned short j;
	unsigned short k;
} uTimevect; 
