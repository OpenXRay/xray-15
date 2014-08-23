/*===========================================================================*\
 | 
 |  FILE:	PerFaceData.h
 |			Project to demonstrate custom  per face data
 |			Simply allows user to define a custom value and bind it to a face
 |			All data is maintained by the modifier.  The modifier will survive
 |			a stack
 |
 |			The file implements the Face storage class
 | 
 |  AUTH:   Neil Hazzard
 |			Developer Consulting Group
 |			Copyright(c) Discreet 2000
 |
 |  HIST:	Started 26-9-00
 | 
\*===========================================================================*/

#ifndef __PERFACEDATA__H
#define __PERFACEDATA__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "modstack.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

extern ClassDesc2 *GetRandomFaceDataDesc(), *GetDataToColorDesc();
extern ClassDesc *GetEditFaceDataDesc();
extern ClassDesc2 *GetFaceDataExportDesc();

#endif
