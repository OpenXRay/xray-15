/**********************************************************************
 *<
	FILE: fog.cpp	

	DESCRIPTION: Simple fog atmospheric effect

	CREATED BY: Rolf Berteig

	HISTORY: 11/21/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "rendpch.h"
#include "imtl.h"
#include "render.h"  
#include <bmmlib.h>
#include "iparamm.h"
#include "texutil.h"
#include "resource.h"
#include "stdmat.h"

Class_ID fogClassID(FOG_CLASS_ID,0);
#define FOG_CLASSNAME GetString(IDS_RB_FOG)

//--- Parameter Maps ----------------------------------------------------

#define PB_COLOR	0
#define PB_USEMAP	1
#define PB_USEOPAC	2
#define PB_FOGBG	3
#define PB_TYPE		4
#define PB_NEAR		5
#define PB_FAR		6
#define PB_TOP		7
#define PB_BOTTOM	8
#define PB_DENSITY	9
#define PB_FALLOFF	10
#define PB_HNOISE	11
#define PB_SCALE	12
#define PB_ANGLE	13
#define PB_PHASE	14
#define PB_EXP		15


class FogDlgProc;

class FogDADMgr: public DADMgr {
	FogDlgProc *dlg;
	public:
		FogDADMgr() { dlg =NULL;}
		void Init(FogDlgProc *d) {dlg = d; }
		// called on the draggee to see what if anything can be dragged from this x,y
		SClass_ID GetDragType(HWND hwnd, POINT p) { return TEXMAP_CLASS_ID; }
		// called on potential dropee to see if can drop type at this x,y
		BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
		int SlotOwner() { return OWNER_SCENE; }  
		ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
		void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
		BOOL AutoTooltip() { return TRUE; }
	};


class FogAtmos : public StdFog {
	public:
		// Parameters
		IParamBlock *pblock;
		Texmap *map, *opac;
		CRITICAL_SECTION csect;
		
		// Caches
		Color fogColor;
		float nearF, farF, top, bottom, density, far_minus_near, fog_range;
		float scale, angle, phase;
		int type, falloff, useMap, useOpac, hnoise, fogBG, exponential;
		Interval valid;		

		static FogDlgProc *dlg;

		FogAtmos();
		~FogAtmos() { 	DeleteCriticalSection(&csect);		}
		void UpdateCaches(TimeValue t);

		// Methods from StdFog:
		void SetColorMap(Texmap *tex) {	ReplaceReference(1,tex); tex->InitSlotType(MAPSLOT_ENVIRON); }
		void SetOpacMap(Texmap *tex) {	ReplaceReference(2,tex); tex->InitSlotType(MAPSLOT_ENVIRON); }
		void SetColor(Color c, TimeValue t){ pblock->SetValue(PB_COLOR,t,c); }		
		void SetUseMap(BOOL onoff){ pblock->SetValue(PB_USEMAP,0,onoff); }		
		void SetUseOpac(BOOL onoff){ pblock->SetValue(PB_USEOPAC,0,onoff); }		
		void SetFogBackground(BOOL onoff) { pblock->SetValue(PB_FOGBG,0,onoff); }		
		void SetType(int type) { pblock->SetValue(PB_TYPE,0,type); }		
		void SetNear(float v, TimeValue t) { pblock->SetValue(PB_NEAR,t,v); }		
		void SetFar(float v, TimeValue t){ pblock->SetValue(PB_FAR,t,v); }		
		void SetTop(float v, TimeValue t) { pblock->SetValue(PB_TOP,t,v); }		
		void SetBottom(float v, TimeValue t) { pblock->SetValue(PB_BOTTOM,t,v); }		
		void SetDensity(float v, TimeValue t) { pblock->SetValue(PB_DENSITY,t,v); }		
		void SetFalloffType(int tp) { pblock->SetValue(PB_FALLOFF,0,tp); }		
		void SetUseNoise(BOOL onoff)  { pblock->SetValue(PB_HNOISE,0,onoff); }		
		void SetNoiseScale(float v, TimeValue t){ pblock->SetValue(PB_SCALE,t,v); }		
		void SetNoiseAngle(float v, TimeValue t) { pblock->SetValue(PB_ANGLE,t,v); }		
		void SetNoisePhase(float v, TimeValue t) { pblock->SetValue(PB_PHASE,t,v); }		

		Color GetColor(TimeValue t) { return pblock->GetColor(PB_COLOR,t); }
		BOOL GetUseMap() { return pblock->GetInt(PB_USEMAP,0); }
		BOOL GetUseOpac() { return pblock->GetInt(PB_USEOPAC,0); }
		Texmap *GetColorMap() { return map; } 
		Texmap *GetOpacMap() { return opac; }
		BOOL GetFogBackground() { return pblock->GetInt(PB_FOGBG,0); }
		int GetType() { return pblock->GetInt(PB_TYPE,0); }
		float GetNear(TimeValue t) { return pblock->GetFloat(PB_NEAR,t); }
		float GetFar(TimeValue t) { return pblock->GetFloat(PB_FAR,t); }
		float GetTop(TimeValue t) { return pblock->GetFloat(PB_BOTTOM,t); }
		float GetBottom(TimeValue t) { return pblock->GetFloat(PB_TOP,t); }
		float GetDensity(TimeValue t) { return pblock->GetFloat(PB_DENSITY,t); }
		int GetFalloffType() { return pblock->GetInt(PB_FALLOFF,0); }
		BOOL GetUseNoise() { return pblock->GetInt(PB_HNOISE,0); }
		float GetNoiseScale( TimeValue t) { return pblock->GetFloat(PB_SCALE,t); }
		float GetNoiseAngle( TimeValue t) { return pblock->GetFloat(PB_ANGLE,t); }
		float GetNoisePhase( TimeValue t) { return pblock->GetFloat(PB_PHASE,t); }


		// Animatable/Reference
		int NumSubs() {return 3;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int NumRefs() {return 3;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		Class_ID ClassID() {return fogClassID;}
		void GetClassName(TSTR& s) {s=FOG_CLASSNAME;}
		void DeleteThis() {delete this;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);

		void RescaleWorldUnits(float f);
		IOResult Load(ILoad *iload);

		// Atmospheric
		TSTR GetName() {return FOG_CLASSNAME;}
		AtmosParamDlg *CreateParamDialog(IRendParams *ip);
		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);
		void Update(TimeValue t, Interval& valid);
		void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG=FALSE);		
	};

class FogParamDlg : public AtmosParamDlg {
	public:
		FogAtmos *fog;
		IRendParams *ip;
		IParamMap *pmap;

		FogParamDlg(FogAtmos *f,IRendParams *i);
		Class_ID ClassID() {return fogClassID;}
		ReferenceTarget* GetThing() {return fog;}
		void SetThing(ReferenceTarget *m);		
		void DeleteThis();
	};


class FogClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new FogAtmos; }
	const TCHAR *	ClassName() { return FOG_CLASSNAME; }
	SClass_ID		SuperClassID() { return ATMOSPHERIC_CLASS_ID; }
	Class_ID 		ClassID() { return fogClassID; }
	const TCHAR* 	Category() { return _T("");  }
	};

static FogClassDesc fogCD;
ClassDesc* GetFogDesc() {return &fogCD;}

static int typeIDs[] = {IDC_FOG_REGULAR,IDC_FOG_LAYERED};
static int falloffIDs[] = {IDC_FOG_TOPFALLOFF,IDC_FOG_BOTTOMFALLOFF,IDC_FOG_NOFALLOFF};

static ParamUIDesc descParam[] = {
	
	// Fog color
	ParamUIDesc(PB_COLOR,TYPE_COLORSWATCH,IDC_FOG_COLOR),
	
	// Use map
	ParamUIDesc(PB_USEMAP,TYPE_SINGLECHEKBOX,IDC_FOG_USEMAP),

	// Use opac
	ParamUIDesc(PB_USEOPAC,TYPE_SINGLECHEKBOX,IDC_FOG_USEOPAC),

	// Fog background
	ParamUIDesc(PB_FOGBG,TYPE_SINGLECHEKBOX,IDC_FOG_FOGBG),

	// Type
	ParamUIDesc(PB_TYPE,TYPE_RADIO,typeIDs,2),

	// Near
	ParamUIDesc(
		PB_NEAR,
		EDITTYPE_FLOAT,
		IDC_FOG_NEAR,IDC_FOG_NEARSPIN,
		0.0f,100.0f,
		0.1f,
		stdPercentDim),

	// Far
	ParamUIDesc(
		PB_FAR,
		EDITTYPE_FLOAT,
		IDC_FOG_FAR,IDC_FOG_FARSPIN,
		0.0f,100.0f,
		0.1f,
		stdPercentDim),

	// Top
	ParamUIDesc(
		PB_TOP,
		EDITTYPE_UNIVERSE,
		IDC_FOG_TOP,IDC_FOG_TOPSPIN,
		-999999999.0f,999999999.0f,
		0.1f),

	// Bottom
	ParamUIDesc(
		PB_BOTTOM,
		EDITTYPE_UNIVERSE,
		IDC_FOG_BOTTOM,IDC_FOG_BOTTOMSPIN,
		-999999999.0f,999999999.0f,
		0.1f),

	// Density
	ParamUIDesc(
		PB_DENSITY,
		EDITTYPE_FLOAT,
		IDC_FOG_DENISITY,IDC_FOG_DENISITYSPIN,
		0.0f,999999999.0f,
		0.1f),

	// Falloff
	ParamUIDesc(PB_FALLOFF,TYPE_RADIO,falloffIDs,3),

	// HNoise
	ParamUIDesc(PB_HNOISE,TYPE_SINGLECHEKBOX,IDC_FOG_HNOISE),

	// Scale
	ParamUIDesc(
		PB_SCALE,
		EDITTYPE_FLOAT,
		IDC_FOG_SCALE,IDC_FOG_SCALESPIN,
		0.0f,999999999.0f,
		0.01f),

	// Angle
	ParamUIDesc(
		PB_ANGLE,
		EDITTYPE_FLOAT,
		IDC_FOG_ANGLE,IDC_FOG_ANGLESPIN,
		0.0f,90.0f,
		0.1f,
		stdAngleDim),

	// Phase
	ParamUIDesc(
		PB_PHASE,
		EDITTYPE_FLOAT,
		IDC_FOG_PHASE,IDC_FOG_PHASESPIN,
		-999999999.0f,999999999.0f,
		0.1f),	

	// Exponential
	ParamUIDesc(PB_EXP,TYPE_SINGLECHEKBOX,IDC_FOG_EXP),
	};
#define PARAMDESC_LENGH 16

static ParamBlockDescID descVer0[] = {
	{ TYPE_POINT3, NULL, TRUE, 0 }, // color
	{ TYPE_INT, NULL, FALSE, 1 },	// use map	
	{ TYPE_INT, NULL, FALSE, 2 },	// fog background
	{ TYPE_INT, NULL, FALSE, 3 },	// type
	{ TYPE_FLOAT, NULL, TRUE, 4 },	// near
	{ TYPE_FLOAT, NULL, TRUE, 5 },	// far
	{ TYPE_FLOAT, NULL, TRUE, 6 }, 	// top
	{ TYPE_FLOAT, NULL, TRUE, 7 },	// bottom
	{ TYPE_FLOAT, NULL, TRUE, 8 },	// density
	{ TYPE_INT, NULL, FALSE, 9 }}; 	// falloff

static ParamBlockDescID descVer1[] = {
	{ TYPE_RGBA, NULL, TRUE, 0 },	// color
	{ TYPE_INT, NULL, FALSE, 1 },	// use map
	{ TYPE_INT, NULL, FALSE, 10 },	// use opac
	{ TYPE_INT, NULL, FALSE, 2 },	// fog background
	{ TYPE_INT, NULL, FALSE, 3 },	// type
	{ TYPE_FLOAT, NULL, TRUE, 4 },	// near
	{ TYPE_FLOAT, NULL, TRUE, 5 },	// far
	{ TYPE_FLOAT, NULL, TRUE, 6 }, 	// top
	{ TYPE_FLOAT, NULL, TRUE, 7 },	// bottom
	{ TYPE_FLOAT, NULL, TRUE, 8 },	// density
	{ TYPE_INT, NULL, FALSE, 9 },	// falloff
	{ TYPE_INT, NULL, FALSE, 11 },	// hnoise
	{ TYPE_FLOAT, NULL, TRUE, 12 },	// scale
	{ TYPE_FLOAT, NULL, TRUE, 13 },	// angle
	{ TYPE_FLOAT, NULL, TRUE, 14 },	// phase	
	}; 	

static ParamBlockDescID descVer2[] = {
	{ TYPE_RGBA, NULL, TRUE, 0 },	// color
	{ TYPE_INT, NULL, FALSE, 1 },	// use map
	{ TYPE_INT, NULL, FALSE, 10 },	// use opac
	{ TYPE_INT, NULL, FALSE, 2 },	// fog background
	{ TYPE_INT, NULL, FALSE, 3 },	// type
	{ TYPE_FLOAT, NULL, TRUE, 4 },	// near
	{ TYPE_FLOAT, NULL, TRUE, 5 },	// far
	{ TYPE_FLOAT, NULL, TRUE, 6 }, 	// top
	{ TYPE_FLOAT, NULL, TRUE, 7 },	// bottom
	{ TYPE_FLOAT, NULL, TRUE, 8 },	// density
	{ TYPE_INT, NULL, FALSE, 9 },	// falloff
	{ TYPE_INT, NULL, FALSE, 11 },	// hnoise
	{ TYPE_FLOAT, NULL, TRUE, 12 },	// scale
	{ TYPE_FLOAT, NULL, TRUE, 13 },	// angle
	{ TYPE_FLOAT, NULL, TRUE, 14 },	// phase	
	{ TYPE_INT, NULL, FALSE, 15},   // exponential
	}; 	

#define PBLOCK_LENGTH	16

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,10,0),	
	ParamVersionDesc(descVer1,15,1),
	};

#define NUM_OLDVERSIONS	2

#define CURRENT_VERSION	2
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);


//--- FogDlgProc ----------------------------------------------------------

class FogDlgProc : public ParamMapUserDlgProc {
	public:
		IParamMap *pmap;
		FogAtmos *fog;
		IRendParams *ip;
		HWND hWnd;
		ICustButton *mapBut[2];
		FogDADMgr dadMgr;

		FogDlgProc(IParamMap *pmap,FogAtmos *f,IRendParams *i);
		~FogDlgProc();

		void AssignMap(int which);
		void SetMap(int which, Texmap *m);
		void Init();
		void SetState();
		void Invalidate() {pmap->Invalidate();}//{ if (hWnd) InvalidateRect(hWnd,NULL,0); }
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};
  
//------------------------------------------------------------------------------
// Drag And Drop manager
//------------------------------------------------------------------------------

BOOL FogDADMgr::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {
	if (hfrom==hto) return FALSE;
	return (type==TEXMAP_CLASS_ID)?1:0;
	}

ReferenceTarget *FogDADMgr::GetInstance(HWND hwnd, POINT p, SClass_ID type) {
	if (hwnd==GetDlgItem(dlg->hWnd,IDC_FOG_MAPNAME))
		return dlg->fog->map;
	if (hwnd==GetDlgItem(dlg->hWnd,IDC_FOG_OPACNAME))
		return dlg->fog->opac;
	return NULL;
	}

void FogDADMgr::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type) {
	Texmap *m = (Texmap *)dropThis;
	if (hwnd==GetDlgItem(dlg->hWnd,IDC_FOG_MAPNAME))
		dlg->SetMap(1, (Texmap *)dropThis);
	if (hwnd==GetDlgItem(dlg->hWnd,IDC_FOG_OPACNAME))
		dlg->SetMap(2, (Texmap *)dropThis);
	}

//------------------------------------------------------------------------------

FogDlgProc::FogDlgProc(IParamMap *pmap,FogAtmos *f,IRendParams *i) 
	{
	this->pmap = pmap;
	fog = f;
	ip  = i;
	fog->dlg = this;
	dadMgr.Init(this);
	}

FogDlgProc::~FogDlgProc()
	{
	fog->dlg = NULL;
	}

void FogDlgProc::SetMap(int which, Texmap *m) {
	fog->ReplaceReference(which,m);
	if (m) {
		assert(m->SuperClassID()==TEXMAP_CLASS_ID);
		Texmap *tm = (Texmap*)m;
		tm->RecursInitSlotType(MAPSLOT_ENVIRON);
		fog->pblock->SetValue(which==1?PB_USEMAP:PB_USEOPAC,0,1);
		}
	Init();
	}

void FogDlgProc::AssignMap(int which)
	{
	BOOL newMat, cancel;
	MtlBase *m = ip->DoMaterialBrowseDlg(
		hWnd,BROWSE_MAPSONLY|BROWSE_INCNONE,newMat,cancel);
	if (!cancel) 
		SetMap(which,(Texmap *)m);
	}

void FogDlgProc::Init()
	{
	if (fog->map) {
		mapBut[0]->SetText(fog->map->GetFullName());
	} else {
		mapBut[0]->SetText(GetString(IDS_DS_NONE));
		}
	if (fog->opac) {
		mapBut[1]->SetText(fog->opac->GetFullName());
	} else {
		mapBut[1]->SetText(GetString(IDS_DS_NONE));
		}
	}

void FogDlgProc::SetState()
	{
	int type, hnoise;
	fog->pblock->GetValue(PB_TYPE,0,type,FOREVER);	
	fog->pblock->GetValue(PB_HNOISE,0,hnoise,FOREVER);		

	EnableWindow(GetDlgItem(hWnd,IDC_FOG_REGULARLABEL),!type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_NEARLABEL),!type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_NEAR),!type);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_FARLABEL),!type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_FAR),!type);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_EXP),!type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_LAYEREDLABEL),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_TOPLABEL),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_TOP),type);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_BOTTOMLABEL),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_BOTTOM),type);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_DENSITYLABEL),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_DENISITY),type);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_HNOISE),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_SCALELABEL),type && hnoise);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_SCALE),type && hnoise);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_FALLOFFLABEL),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_TOPFALLOFF),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_BOTTOMFALLOFF),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_NOFALLOFF),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_ANGLELABEL),type && hnoise);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_ANGLE),type && hnoise);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_PHASELABEL),type && hnoise);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_PHASE),type && hnoise);		

	ISpinnerControl *iSpin;
	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_TOPSPIN));
	if (type) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_BOTTOMSPIN));
	if (type) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_DENISITYSPIN));
	if (type) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_SCALESPIN));
	if (type && hnoise) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_ANGLESPIN));
	if (type && hnoise) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_PHASESPIN));
	if (type && hnoise) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_NEARSPIN));
	if (!type) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);

	iSpin = GetISpinner(GetDlgItem(hWnd,IDC_FOG_FARSPIN));
	if (!type) iSpin->Enable(); else iSpin->Disable();
	ReleaseISpinner(iSpin);
	/*
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_NEARSPIN),!type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_FARSPIN),!type);		
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_TOPSPIN),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_BOTTOMSPIN),type);
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_DENISITYSPIN),type);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_SCALESPIN),type && hnoise);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_ANGLESPIN),type && hnoise);	
	EnableWindow(GetDlgItem(hWnd,IDC_FOG_PHASESPIN),type && hnoise);	
	*/
	}

BOOL FogDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,
		UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			this->hWnd = hWnd;
			mapBut[0] = GetICustButton(GetDlgItem(hWnd, IDC_FOG_MAPNAME));		
			mapBut[1] = GetICustButton(GetDlgItem(hWnd, IDC_FOG_OPACNAME));		
			mapBut[0]->SetDADMgr(&dadMgr);
			mapBut[1]->SetDADMgr(&dadMgr);
			Init();
			SetState();
			break;

		case WM_DESTROY:
			ReleaseICustButton(mapBut[0]);
			ReleaseICustButton(mapBut[1]);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_FOG_HNOISE:
				case IDC_FOG_REGULAR:
				case IDC_FOG_LAYERED:
					SetState();
					break;	
				case IDC_FOG_MAPNAME:
					AssignMap(1);
					break;
				case IDC_FOG_OPACNAME:
					AssignMap(2);
					break;
				}
			break;
		}
	
	return FALSE;
	}


//--- FogParamDlg -------------------------------------------------------


FogParamDlg::FogParamDlg(FogAtmos *f,IRendParams *i) 
	{
	fog = f;
	ip  = i;	
	pmap = CreateRParamMap(
		descParam,PARAMDESC_LENGH,
		fog->pblock,
		i,
		hInstance,
		MAKEINTRESOURCE(IDD_FOG_PARAMS),
		GetString(IDS_RB_FOGPARAMS),
		0);
	
	pmap->SetUserDlgProc(new FogDlgProc(pmap,fog,ip));	
	}

void FogParamDlg::SetThing(ReferenceTarget *m)
	{
	assert(m->ClassID()==fog->ClassID());
	fog = (FogAtmos*)m;
	pmap->SetParamBlock(fog->pblock);
	pmap->SetUserDlgProc(new FogDlgProc(pmap,fog,ip));	
	if (fog->dlg) {
		fog->dlg->fog = fog;		
		fog->dlg->Init();
		fog->dlg->SetState();
		}
	}

void FogParamDlg::DeleteThis()
	{
	DestroyRParamMap(pmap);
	delete this;
	}

//--- FogAtmos ----------------------------------------------------------

FogDlgProc *FogAtmos::dlg = NULL;

FogAtmos::FogAtmos()
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer2, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	map  = NULL;
	opac = NULL;
	valid.SetEmpty();
	InitializeCriticalSection(&csect);

	pblock->SetValue(PB_COLOR,0,Point3(1,1,1));		
	pblock->SetValue(PB_FAR,0,1.0f);
	pblock->SetValue(PB_TOP,0,100.0f);	
	pblock->SetValue(PB_DENSITY,0,50.0f);	
	pblock->SetValue(PB_FALLOFF,0,FALLOFF_NONE);
	pblock->SetValue(PB_SCALE,0,20.0f);
	pblock->SetValue(PB_ANGLE,0,DegToRad(5.0f));	
	pblock->SetValue(PB_FOGBG,0,TRUE);
	}

IOResult FogAtmos::Load(ILoad *iload)
	{
	Atmospheric::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

AtmosParamDlg *FogAtmos::CreateParamDialog(IRendParams *ip)
	{	
	return new FogParamDlg(this,ip);
	}

Animatable* FogAtmos::SubAnim(int i)
	{
	switch (i) {
		case 0: return pblock;
		case 1: return map;
		case 2: return opac;
		default: return NULL;
		}
	}

TSTR FogAtmos::SubAnimName(int i)
	{
	switch (i) {
		case 0: return GetString(IDS_RB_PARAMETERS);
		case 1: return GetString(IDS_RB_FOGTEXMAP);
		case 2: return GetString(IDS_RB_FOGOPACMAP);
		default: return _T("");
		}
	}

RefTargetHandle FogAtmos::GetReference(int i)
	{
	switch (i) {
		case 0: return pblock;
		case 1: return map;
		case 2: return opac;
		default: return NULL;
		}
	}

void FogAtmos::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0: pblock = (IParamBlock*)rtarg; break;
		case 1: map = (Texmap*)rtarg; 
			if (dlg) dlg->Init();					
			break;
		case 2: opac = (Texmap*)rtarg; 
			if (dlg) dlg->Init();					
			break;
		}
	}

void FogAtmos::RescaleWorldUnits(float f) {
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	pblock->RescaleParam(PB_TOP,f);	
	pblock->RescaleParam(PB_BOTTOM,f);	
	//pblock->RescaleParam(PB_SCALE,f);	
	}

RefResult FogAtmos::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
	{
	switch (message) {
		case REFMSG_CHANGE:
			valid.SetEmpty();
			if (dlg) {
				dlg->Invalidate();
				if (hTarget==opac||hTarget==map)
					dlg->Init();
				}
			break;

		case REFMSG_NODE_NAMECHANGE:
			if (dlg&&(hTarget==map||hTarget==opac)) {
				if (map) {
					TSTR name = map->GetFullName();
					dlg->mapBut[0]->SetText(name);
					}
				if (opac) {
					TSTR name = opac->GetFullName();
					dlg->mapBut[1]->SetText(name);
					}
				}
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_COLOR: 	gpd->dim = stdColor255Dim; break;
				case PB_NEAR:
				case PB_FAR:	gpd->dim = stdPercentDim; break;
				case PB_TOP:
				case PB_PHASE:
				case PB_BOTTOM:	gpd->dim = stdWorldDim; break;
				case PB_ANGLE:	gpd->dim = stdAngleDim; break;
				default: 		gpd->dim = defaultDim;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_COLOR:		gpn->name = GetString(IDS_RB_FOGCOLOR); break;
				case PB_NEAR:		gpn->name = GetString(IDS_RB_NEARPERCENT); break;
				case PB_FAR:		gpn->name = GetString(IDS_RB_FARPERCENT); break;
				case PB_TOP:		gpn->name = GetString(IDS_RB_TOP); break;
				case PB_BOTTOM:		gpn->name = GetString(IDS_RB_BOTTOM); break;
				case PB_DENSITY:	gpn->name = GetString(IDS_RB_DENSITY); break;
				case PB_SCALE:		gpn->name = GetString(IDS_RB_FOGSCALE); break;
				case PB_ANGLE:		gpn->name = GetString(IDS_RB_ANGLE2); break;
				case PB_PHASE:		gpn->name = GetString(IDS_RB_PHASE); break;
				case PB_USEMAP:		gpn->name = GetString(IDS_RB_USECOLORMAP); break;
				case PB_USEOPAC:	gpn->name = GetString(IDS_RB_USEOPACMAP); break;
				case PB_FOGBG:		gpn->name = GetString(IDS_RB_FOGBG); break;
				case PB_TYPE:		gpn->name = GetString(IDS_RB_FOGTYPE); break;
				case PB_FALLOFF:	gpn->name = GetString(IDS_RB_FALLOFF); break;
				case PB_HNOISE:		gpn->name = GetString(IDS_RB_HNOISE); break;				
				case PB_EXP:		gpn->name = GetString(IDS_RB_EXP); break;
				default:			gpn->name = _T(""); break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}

void FogAtmos::Update(TimeValue t, Interval& valid)
	{
	if (map) map->Update(t,valid);
	if (opac) opac->Update(t,valid);
	}

int FogAtmos::RenderBegin(TimeValue t, ULONG flags)
	{
	return 0;
	}

int FogAtmos::RenderEnd(TimeValue t)
	{
	return 0;
	}

#define EPSILON	0.001f

void FogAtmos::UpdateCaches(TimeValue t)
	{
	EnterCriticalSection(&csect);
	if (!valid.InInterval(t)) {
		valid = FOREVER;		
		pblock->GetValue(PB_COLOR,t,fogColor,valid);
		pblock->GetValue(PB_USEMAP,t,useMap,valid);
		pblock->GetValue(PB_USEOPAC,t,useOpac,valid);
		pblock->GetValue(PB_NEAR,t,nearF,valid);
		pblock->GetValue(PB_FAR,t,farF,valid);
		pblock->GetValue(PB_TOP,t,top,valid);
		pblock->GetValue(PB_BOTTOM,t,bottom,valid);
		pblock->GetValue(PB_DENSITY,t,density,valid);
		pblock->GetValue(PB_TYPE,t,type,valid);
		pblock->GetValue(PB_FALLOFF,t,falloff,valid);
		pblock->GetValue(PB_HNOISE,t,hnoise,valid);
		pblock->GetValue(PB_SCALE,t,scale,valid);
		pblock->GetValue(PB_ANGLE,t,angle,valid);
		pblock->GetValue(PB_PHASE,t,phase,valid);
		pblock->GetValue(PB_FOGBG,t,fogBG,valid);
		pblock->GetValue(PB_EXP,t,exponential,valid);
		if (exponential) {
			// Take the natural log since we're going to be
			// exponentiating later on.
			farF   = 1.0f-farF;
			nearF  = 1.0f-nearF;
			if (farF  < EPSILON) farF  = EPSILON;
			if (nearF < EPSILON) nearF = EPSILON;
			farF  = -(float)log(farF);
			nearF = -(float)log(nearF);
			}
		far_minus_near = farF - nearF;
		if (top<bottom) {
			float tmp = top;
			top = bottom;
			bottom = tmp;
			}
		fog_range = top-bottom;
		density /= 100.0f;
		if (!map) useMap   = FALSE;
		if (!opac) useOpac = FALSE;
		if (scale != 0.0) scale = 400.0f/scale;
		}
	LeaveCriticalSection(&csect);
	}

inline float CalcPercent(
		float np, float fp, 
		float nF, float fF,
		float nmf,float z)
	{
	float pct;
	if (z<np) pct = nF;
	else
	if (z>fp) pct = fF;
	else
		pct = (z-np)/(fp-np) * nmf + nF;
	return pct;
	}

void FogAtmos::Shade(
		ShadeContext& sc,const Point3& p0,const Point3& p1,
		Color& color, Color& trans, BOOL isBG)
	{	
	float np = sc.CamNearRange(), fp = sc.CamFarRange(), pct; 
	UpdateCaches(sc.CurTime());
	Color opacColor, fogCol=fogColor;	

	if (isBG && !fogBG) return;

	if (useMap && map) {		
		fogCol = map->EvalColor(sc);
		}
	if (useOpac && opac) {		
		opacColor = opac->EvalColor(sc);
		}

	if (type==0) { // Regular		
		if (exponential) {
			Point3 pt0 = sc.PointTo(p0,REF_CAMERA);
			Point3 pt1 = sc.PointTo(p1,REF_CAMERA);
			float z0 = (float)fabs(pt0.z);
			float z1 = (float)fabs(pt1.z);
			float pct0 = CalcPercent(
				np, fp, nearF, farF,
				far_minus_near, z0);
			float pct1 = CalcPercent(
				np, fp, nearF, farF,
				far_minus_near, z1);
			
			if (!useOpac) {
				pct = 1.0f - (float)exp(pct0-pct1);
			} else {
				pct = pct0-pct1;
				}
		
		} else {				
			Point3 p = sc.PointTo(p1,REF_CAMERA);			
			float z = (float)fabs(p.z);
			if (z<np) pct = nearF;
			else
			if (z>fp) pct = farF;
			else
				pct = (z-np)/(fp-np) * far_minus_near + nearF;

			assert(pct<=1.0f);		
			}

		if (useOpac) {
			if (exponential) {
				opacColor.r = 1.0f - (float)exp(opacColor.r*pct);
				opacColor.g = 1.0f - (float)exp(opacColor.g*pct);
				opacColor.b = 1.0f - (float)exp(opacColor.b*pct);

				color += (fogCol-color)*opacColor;
				trans *= Color(1.0f-opacColor.r,1.0f-opacColor.g,1.0f-opacColor.b);
			} else {
				color += (fogCol-color)*opacColor*pct;			
				trans *= Color(1.0f-opacColor.r,1.0f-opacColor.g,1.0f-opacColor.b)
						*(1.0f-pct);
				}
		} else {
			color += (fogCol-color)*pct;
			trans *= 1.0f-pct;
			}
			
	} else { // Layered
		
		// Layered won't work in non-camera views
		if (sc.ProjType()==PROJ_PARALLEL) return;

		Point3 wp0 = sc.PointTo(p0,REF_WORLD);
		Point3 wp1 = sc.PointTo(p1,REF_WORLD);
		if (wp0==wp1) return;
		float dx, dy, dz, za, zb, l, a, b, s, tmp, pct;

		// Compute deltas
		dx = wp1.x-wp0.x;
		dy = wp1.y-wp0.y;
		za = wp0.z;
		zb = wp1.z;
		if (zb<za) {tmp = za; za = zb; zb = tmp;}

		// Are we in the layer
		if (zb<bottom || za>top) return;

		// RB 8/15/97: dz should never be less than 0.
		//if (zb==za) {pct = 1.0f; goto do_lfog;}
		dz = zb-za;
		
		// RB: 9/22/97: It's OK for dz to approach 0. I'm not sure
		// why I had this line in here limiting dz to 1.
		//if (dz<1.0f) dz = 1.0f;
		
		// RB 2/19/99: We can approach 0 but we don't want to divide by 0.
		if (dz<0.0001f) dz = 0.0001f;

		l = (float)sqrt(dx*dx + dy*dy);
		a = (za-bottom)/fog_range;			
		b = (zb-bottom)/fog_range;			
		if (b>1.0f) b = 1.0f;
		if (a<0.0f) a = 0.0f;
		switch (falloff) {
			case FALLOFF_NONE:
				s = (b-a);
				break;
			case FALLOFF_BOTTOM:
				tmp = a; a = 1-b; b = 1-tmp; 
			case FALLOFF_TOP:
				s = b + b*b*(1-b) - a - a*a*(1-a);
				break;
			}		

		if (!useOpac) {
			pct = 1.0f - (float)exp(-s*density*l/dz);
		} else {
			pct = -s*density*l/dz;
			}
	//do_lfog:

		if (hnoise) {			
			Point3 v = Normalize(wp1-wp0);			
			float a2 = (float)asin(v.z);
			if (fabs(a2)<angle) {
				float a1 = (float)atan2(v.y,v.x);
				float n  = noise3(
					Point3(a1*scale,a2*scale-phase,phase))+1.0f;
				if (n>1.0f) n = 1.0f;
				float u = (float)fabs(a2)/angle;
				pct *= u*(1.0f - (n*(1.0f-u)));
				}
			}

		if (useOpac) {
			opacColor.r = 1.0f - (float)exp(opacColor.r*pct);
			opacColor.g = 1.0f - (float)exp(opacColor.g*pct);
			opacColor.b = 1.0f - (float)exp(opacColor.b*pct);

			color += (fogCol-color)*opacColor;
			trans *= Color(1.0f-opacColor.r,1.0f-opacColor.g,1.0f-opacColor.b);

			//color += (fogCol-color)*opacColor*pct;
			//trans *= Color(1.0f-opacColor.r,1.0f-opacColor.g,1.0f-opacColor.b)
			//			*(1.0f-pct);			
		} else {
			color += (fogCol-color)*pct;
			trans *= 1.0f-pct;
			}
		}
	}

		


