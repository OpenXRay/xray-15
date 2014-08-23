/**********************************************************************
 *<
	FILE: prim.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __PRIM__H
#define __PRIM__H

#include "Max.h"
#include "resource.h"
#include "resourceOverride.h"

#ifdef DESIGN_VER //for conversion to amodeler solids
#include "..\..\..\include\igeomimp.h"
#include "plugapi.h"
#endif

TCHAR *GetString(int id);

extern ClassDesc* GetBoxobjDesc();
extern ClassDesc* GetSphereDesc();
extern ClassDesc* GetCylinderDesc();
extern ClassDesc* GetSimpleCamDesc();
#ifndef NO_OBJECT_OMNI_LIGHT
extern ClassDesc* GetOmniLightDesc();
#endif
#ifndef NO_OBJECT_DIRECT_LIGHT
extern ClassDesc* GetDirLightDesc();
extern ClassDesc *GetTDirLightDesc();
#endif
#ifndef NO_OBJECT_SPOT_LIGHT
extern ClassDesc* GetFSpotLightDesc();
extern ClassDesc* GetTSpotLightDesc();
#endif
extern ClassDesc* GetLookatCamDesc();
#ifndef NO_OBJECT_SHAPES_SPLINES
extern ClassDesc* GetSplineDesc();
#endif
#ifdef DESIGN_VER
extern ClassDesc* GetOrthoSplineDesc();
#endif
#ifndef NO_OBJECT_SHAPES_SPLINES
extern ClassDesc* GetNGonDesc();
extern ClassDesc* GetDonutDesc();
#endif
extern ClassDesc* GetTargetObjDesc();
#ifndef NO_OBJECT_BONE
extern ClassDesc* GetBonesDesc();
#endif // NO_OBJECT_BONE
extern ClassDesc* GetRingMasterDesc();
extern ClassDesc* GetSlaveControlDesc();
extern ClassDesc* GetQuadPatchDesc();
extern ClassDesc* GetTriPatchDesc();
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
extern ClassDesc* GetTorusDesc();
#endif
extern ClassDesc* GetMorphObjDesc();
extern ClassDesc* GetCubicMorphContDesc();
#ifndef NO_OBJECT_SHAPES_SPLINES
extern ClassDesc* GetRectangleDesc();
#endif
#ifndef NO_OBJECT_BOOL
extern ClassDesc* GetBoolObjDesc();
#endif
#ifndef NO_HELPER_TAPE
extern ClassDesc* GetTapeHelpDesc();
#endif
#ifndef NO_HELPER_PROTRACTOR
extern ClassDesc* GetProtHelpDesc();
#endif
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
extern ClassDesc* GetTubeDesc();
extern ClassDesc* GetConeDesc();
#endif
#if !defined(NO_EXTENDED_PRIMITIVES) && !defined(NO_OBJECT_HEDRA)
extern ClassDesc* GetHedraDesc();
#endif
#ifndef NO_OBJECT_SHAPES_SPLINES
extern ClassDesc* GetCircleDesc();
extern ClassDesc* GetEllipseDesc();
extern ClassDesc* GetArcDesc();
extern ClassDesc* GetStarDesc();
extern ClassDesc* GetHelixDesc();
#endif
#ifndef NO_PARTICLES
extern ClassDesc* GetRainDesc();
extern ClassDesc* GetSnowDesc();
#endif // NO_PARTICLES
#ifndef NO_OBJECT_SHAPES_SPLINES
extern ClassDesc* GetTextDesc();
#endif
#ifndef NO_OBJECT_TEAPOT
extern ClassDesc* GetTeapotDesc();
#endif // NO_OBJECT_TEAPOT
extern ClassDesc* GetBaryMorphContDesc();
#ifdef DESIGN_VER
extern ClassDesc* GetOrthoSplineDesc();
extern ClassDesc* GetParallelCamDesc();
#endif
// xavier robitaille | 03.02.12 | had been left out...
#ifndef NO_OBJECT_STANDARD_PRIMITIVES 
extern ClassDesc* GetGridobjDesc();
#endif // NO_OBJECT_STANDARD_PRIMITIVES
#ifndef NO_OBJECT_BONE
extern ClassDesc* GetNewBonesDesc();
#endif // NO_OBJECT_BONE

extern HINSTANCE hInstance;

#endif