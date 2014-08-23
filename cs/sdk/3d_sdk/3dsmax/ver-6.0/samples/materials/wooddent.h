/**********************************************************************
 *<
	FILE:			wooddent.h

	DESCRIPTION:	DLL Main for wood and dent 3D textures

	CREATED BY:		Suryan Stalin

	HISTORY:		Modified from mtlmain.cpp, 4th April 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#ifndef __WOODDENT__H
#define __WOODDENT__H


struct	Col24	{	ULONG	r,g,b; };

extern	void	ScaleFloatController(IParamBlock *pblock, int index, float s);
extern  Color	ColrFromCol24(Col24 a); 
extern  TCHAR	*GetString(int id);

#endif
