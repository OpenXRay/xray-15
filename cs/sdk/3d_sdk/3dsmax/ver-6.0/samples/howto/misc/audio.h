
/**************************************************************************
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 *  Copyright (c) 1992, 1993  Microsoft Corporation.  All Rights Reserved.
 * 
 **************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif


BOOL FAR aviaudioPlay(
	HWND hwnd, PAVISTREAM pavi, 
	LONG lCur, LONG lStart, LONG lEnd, 
	BOOL fWait, BOOL repeat, float lrstart, float lrend);

void FAR aviaudioMessage(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam);
	// WIN64 Cleanup: Shuler

void FAR aviaudioStop(void);
LONG FAR aviaudioTime(void);

#ifdef __cplusplus
}
#endif
