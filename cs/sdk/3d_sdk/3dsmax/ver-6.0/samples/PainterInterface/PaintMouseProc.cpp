/**********************************************************************
 *<
	FILE: paintMouseProc.cpp

	DESCRIPTION:	Contains the mouse proc for the painter

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "painterInterface.h"
#include "IpainterInterface.h"



int PaintMouseProc::proc(
		HWND hWnd, int msg, int point, int flags, IPoint2 m)

	{
	static IPoint2 prevPoint;
	static int pointCount = 0;

	static BOOL changeSizeMode = FALSE;
	static BOOL changeStrengthMode = FALSE;
	switch (msg) {
		case MOUSE_POINT:
			{
			if (point==0) 
				{	
				pointCount = 0;
				
				changeSizeMode = FALSE;
				changeStrengthMode = FALSE;

				if ((flags & MOUSE_SHIFT) && (flags & MOUSE_ALT))
					changeSizeMode = TRUE;
				if ((flags & MOUSE_SHIFT) && (flags & MOUSE_CTRL))
					changeStrengthMode = TRUE;

				if (changeSizeMode)
					painterInterface->ResizeStr(m, pointCount);
				else if (changeStrengthMode)			
					painterInterface->ResizeRadius(m, pointCount);
				else painterInterface->HitTest(m, pointCount, flags);
				pointCount = 1;
				} 
			else 
				{
				pointCount = 2;
				if ((!changeSizeMode) && (!changeSizeMode))
					painterInterface->HitTest(m, pointCount, flags)	;		
				changeSizeMode = FALSE;
				changeStrengthMode = FALSE;
				painterInterface->inStrChange = FALSE;
				painterInterface->inRadiusChange = FALSE;
				painterInterface->fpressure = 1.0f;
				}
			break;
			}

		case MOUSE_MOVE: 
			{
			if (pointCount == 1)
				{

				if (changeSizeMode)
					painterInterface->ResizeStr(m, pointCount);
				else if (changeStrengthMode)			
					painterInterface->ResizeRadius(m, pointCount);
				else painterInterface->HitTest(m, pointCount, flags);
				}
			break;
			}

		case MOUSE_ABORT:
			{
			pointCount = 4;
			if (changeSizeMode)
				painterInterface->ResizeStr(m, pointCount);
			else if (changeStrengthMode)			
				painterInterface->ResizeRadius(m, pointCount);
			else painterInterface->HitTest(m, pointCount, flags);
			break;
			}
		case MOUSE_FREEMOVE:
			{
			pointCount = 5;
			ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
			HWND viewhWnd = vpt->GetHWnd();
			GetCOREInterface()->ReleaseViewport( vpt );

			if (hWnd == viewhWnd) //make sure we are tracking only on the active viewport
				painterInterface->HitTest(m, pointCount, flags);
			break;
			}
		}
	return TRUE;


	return TRUE;
	}



