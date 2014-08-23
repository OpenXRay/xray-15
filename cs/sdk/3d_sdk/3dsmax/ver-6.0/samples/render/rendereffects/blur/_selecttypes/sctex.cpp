/**********************************************************************
 *<
	FILE: sctex.cpp

	DESCRIPTION: A ShadeContext for rendering texture maps

	CREATED BY: Rolf Berteig (took code from mtlrend.cpp

	HISTORY: 2/02/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

// precompiled header
#include "pch.h"

// local includes
#include "sctex.h"


SCTex::SCTex() {
	tiling = 1.0f;
	mtlNum = 0; 
	doMaps = TRUE;
	filterMaps = TRUE;
	shadow = FALSE;
	backFace = FALSE;
	curTime = 0;
	norm = Point3(0,0,1);
	view = Point3(0,0,-1);
	ResetOutput();
	}

Box3 SCTex::ObjectBox() {
	return Box3(Point3(0,0,0),Point3(scale,scale,scale));
	}

Point3 SCTex::PObjRelBox() {
	Point3 q;
	Point3 p = PObj();
	Box3 b = ObjectBox();
	q.x = 2.0f*(p.x-b.pmin.x)/(b.pmax.x-b.pmin.x) - 1.0f;
	q.y = 2.0f*(p.y-b.pmin.y)/(b.pmax.y-b.pmin.y) - 1.0f;
	q.z = 2.0f*(p.z-b.pmin.z)/(b.pmax.z-b.pmin.z) - 1.0f;
	return q;
	}

void SCTex::ScreenUV(Point2& uv, Point2 &duv) {
	uv.x = uvw.x;
	uv.y = uvw.x;
	duv.x = duvw.x;
	duv.y = duvw.y;
	}

/*
Point3 SCTex::V() { 
	Point3 v;
	v.x = -2.0f*(uvw.x-0.5f);
	v.z = 2.0f*(uvw.y-0.5f);
	v.y = -0.5f;
	return Normalize(v);
	}       	
*/

Point3 SCTex::DPObjRelBox() {
	return Point3(0,0,0);
	}

void SCTex::DPdUVW(Point3 dP[3],int chan) { 
	dP[0] = dP[1] = dP[2] = Point3(0,0,0);
	}

Point3 SCTex::P() { 
	return pt; 
	}

Point3 SCTex::DP() { 
	return dpt; 
	}

Point3 SCTex::PObj() { 
	return pt; 
	}

Point3 SCTex::DPObj() { 
	return dpt; 
	}
