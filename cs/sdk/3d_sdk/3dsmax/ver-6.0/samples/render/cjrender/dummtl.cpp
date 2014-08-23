//***************************************************************************
// CJRender - [dummtl.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of dummy material shader.
// This is the material that will be used if an INode does not have an
// assigned material.
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"

DumMtl::DumMtl(Color c)
{ 
	diff = c; spec = Color(DUMSPEC,DUMSPEC,DUMSPEC); 
	phongexp = (float)pow(2.0, DUMSHINE*10.0);
}

void DumMtl::Update(TimeValue t, Interval& valid)
{
}

void DumMtl::Reset()
{
}

Interval DumMtl::Validity(TimeValue t)
{
	return FOREVER;
}

ParamDlg* DumMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
	return NULL;
}

Color DumMtl::GetAmbient(int mtlNum, BOOL backFace)
{
	return diff;
}

Color DumMtl::GetDiffuse(int mtlNum, BOOL backFace)
{
	return diff;
}

Color DumMtl::GetSpecular(int mtlNum, BOOL backFace)
{
	return spec;
}

float DumMtl::GetShininess(int mtlNum, BOOL backFace)
{
	return 0.0f;
}

float DumMtl::GetShinStr(int mtlNum, BOOL backFace)
{
	return 0.0f;
}

float DumMtl::GetXParency(int mtlNum, BOOL backFace)
{
	return 0.0f;
}

void DumMtl::SetAmbient(Color c, TimeValue t)
{
}		

void DumMtl::SetDiffuse(Color c, TimeValue t)
{
}

void DumMtl::SetSpecular(Color c, TimeValue t)
{
}

void DumMtl::SetShininess(float v, TimeValue t)
{
}

Class_ID DumMtl::ClassID()
{
	return DUMMTL_CLASS_ID;
}

void DumMtl::DeleteThis()
{
	delete this;
}

RefResult DumMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message)
{
	return REF_SUCCEED;
}

//***************************************************************************
// Shade method for the dummy material
// If a node does not have a material assigned we create
// a dummy material that inherits the wireframe color of
// the node
//***************************************************************************

void DumMtl::Shade(ShadeContext& sc)
{
	Color lightCol;
	Color diffwk(0.0f,0.0f,0.0f);
	Color specwk(0.0f,0.0f,0.0f);
	Color ambwk(0.0f,0.0f,0.0f);
	Point3 N = sc.Normal();
	Point3	R = sc.ReflectVector();
	LightDesc *l;
	for (int i = 0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (!l->Illuminate(sc, N, lightCol, L, NL, diffCoef))
			continue;

		if (l->ambientOnly) {
			ambwk += lightCol;
			continue;
			}
		// diffuse
		if (l->affectDiffuse)
			diffwk += diffCoef*lightCol;
		// specular
		if (l->affectSpecular) {
			float c = DotProd(L,R);
			if (c>0.0f) {
				c = (float)pow((double)c, (double)phongexp); 
				specwk += c*lightCol*NL;   // multiply by NL to SOFTEN 
			}
		}
	}
	sc.out.t = Color(0.0f,0.0f,0.0f);
	sc.out.c = (.3f*sc.ambientLight + diffwk)*diff + specwk*spec + ambwk;		
}
