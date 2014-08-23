/**********************************************************************
 *<
	FILE: BasicOps.h

	DESCRIPTION:

	CREATED BY: Steve Anderson, based on mods.h

	HISTORY: Created 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __BASIC_OPS__H
#define __BASIC_OPS__H

#include "Max.h"
#include "resource.h"

// Modifiers hereabouts:
extern ClassDesc* GetFaceExtrudeModDesc();
extern ClassDesc* GetVertexChamferModDesc();
extern ClassDesc* GetEdgeChamferModDesc();
extern ClassDesc* GetVertexWeldModDesc();
extern ClassDesc* GetSymmetryModDesc();
extern ClassDesc* GetEditPolyModDesc();

TCHAR *GetString(int id);
bool MNMeshCollapseEdges (MNMesh & mm, DWORD edgeFlag);

// in BasicOps.cpp
extern HINSTANCE hInstance;

// For 'Supports Object of Type' rollups
extern INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#define BIGFLOAT	float(9999999)

#define NEWSWMCAT	_T("Modifiers")

#endif

