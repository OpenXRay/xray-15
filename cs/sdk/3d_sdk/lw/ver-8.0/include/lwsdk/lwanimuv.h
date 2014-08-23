/*
 * LWSDK Header File
 * Copyright 2003,  NewTek, Inc.
 *
 * LWANIMUV.H -- LightWave Animation UV
 *
 * Jamie L. Finch
 * Senile Programmer
 */
#ifndef LWSDK_ANIMUV_H
#define LWSDK_ANIMUV_H

#include <lwsdk/lwrender.h>

#define LWANIMUV_HCLASS  "AnimUVHandler"
#define LWANIMUV_ICLASS  "AnimUVInterface"
#define LWANIMUV_VERSION 4

typedef struct st_LWAnimUVHandler {
	LWInstanceFuncs	 *inst;
	LWItemFuncs	 *item;
	int		(*GetOptions)( LWInstance, char * );
	int		(*SetOptions)( LWInstance, char * );
	int		(*Begin     )( LWInstance, char *, double, int, int, int );
	int		(*Evaluate  )( LWInstance, int, double * );
	int		(*End       )( LWInstance );
} LWAnimUVHandler;

#endif

