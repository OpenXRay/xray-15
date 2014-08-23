#ifndef __WRAP__H
#define __WRAP__H

#include "Max.h"
#include "resource.h"

extern HINSTANCE hInstance;

const float EPS=0.1f;
#define BIGFLOAT	float(999999)
TCHAR *GetString(int id);
extern float Check1to1(float x);

extern ClassDesc* GetSWrapDesc();
extern ClassDesc* GetSWrapModDesc();
//extern ClassDesc* GetMSWrapModDesc();

extern INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#endif  
