/**********************************************************************
 *<
	FILE: sctex.cpp

	DESCRIPTION: A ShadeContext for rendering texture maps

	CREATED BY: Rolf Berteig (took code from mtlrend.cpp

	HISTORY: 2/02/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "sctex.h"
#include "gamma.h"

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

Point3 SCTex::DPObjRelBox() {
	return Point3(0,0,0);
	}

// Bump vectors for UVW: in Camera space
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

UBYTE *RenderTexMap(Texmap *tex,int w, int h)
	{
	float du = 1.0f/float(w);
	float dv = 1.0f/float(h);
	AColor col;
	SCTex sc;
//watje 3-4-99	int scanw = ByteWidth(w*3);
	int scanw = ByteWidth(w);
//	UBYTE *image = new UBYTE[ByteWidth(w*3)*h];
	UBYTE *image = new UBYTE[ByteWidth(w)*h];
	UBYTE *p1;
		
	sc.curTime = GetCOREInterface()->GetTime();
	
	sc.scale = 1.0f;
	sc.duvw = Point3(du,dv,0.0f);
	sc.dpt  = sc.duvw;
	sc.uvw.y = 1.0f-0.5f*dv;
	sc.uvw.z = 0.0f;
	for (int j=0; j<h; j++) {
		sc.scrPos.y = j;
		sc.uvw.x = 0.5f*du;				
		p1 = image + (h-j-1)*scanw;
		for (int i=0; i<w; i++) {
			sc.scrPos.x = i;
			sc.pt = sc.uvw;
			col = tex->EvalColor(sc);
			*p1++ = gammaCorrect((UBYTE)(col.b*255.0f));
			*p1++ = gammaCorrect((UBYTE)(col.g*255.0f));
			*p1++ = gammaCorrect((UBYTE)(col.r*255.0f));	
			sc.uvw.x +=du;
			}		
		sc.uvw.y -= dv;
		}
	return image;
	}
