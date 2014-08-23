/**********************************************************************
 *<
	FILE: PaintMode.cpp

	DESCRIPTION:	Contains the paint mode for the painter

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "painterInterface.h"
#include "IpainterInterface.h"


void PaintMode::EnterMode()
	{
	if (painterInterface->IsCanvas_5_1())
		{
		painterInterface->StartPaintMode();
		}
	}

void PaintMode::ExitMode()
	{
	
	if (painterInterface->IsCanvas_5_1())
		{
		painterInterface->EndPaintMode();
		}
	else
		painterInterface->SystemEndPaintSession();

	}