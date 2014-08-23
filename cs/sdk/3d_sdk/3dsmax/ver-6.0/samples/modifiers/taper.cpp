/**********************************************************************
 *<
	FILE: taper.cpp

	DESCRIPTION:  Taper OSM

	CREATED BY: Dan Silva & Rolf Berteig

	HISTORY: created 30 Jauary, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_TAPER // JP Morel - June 28th 2002

#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"

// in mods.cpp
extern HINSTANCE hInstance;

#define BIGFLOAT	float(999999)

class TaperMod : public SimpleMod {
	public:
		static IParamMap *pmapParam;

		TaperMod();		
				
		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_TAPERMOD); }  
		virtual Class_ID ClassID() { return Class_ID(TAPEROSM_CLASS_ID,0);}
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next );
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_TAPER); }		
		IOResult Load(ILoad *iload);

		// From simple mod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		BOOL GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
	};


class TaperDeformer: public Deformer {
	public:
		Matrix3 tm,invtm;
		Box3 bbox;
		TimeValue time;
		float k1,k2, from, to;
		int doRegion, naxis;
		BOOL doX, doY, sym;
		TaperDeformer();
		TaperDeformer(
			TimeValue t, ModContext &mc, 
			float  amt, float crv, int naxis,
			int eaxis, int sym,
			float from, float to, int doRegion,
			Matrix3& modmat, Matrix3& modinv);
		void SetAxis(Matrix3 &tmAxis);
		void SetK(float K1, float K2) { k1 = K1; k2 = K2; }
		Point3 Map(int i, Point3 p); 
	};

#define TAPERWSM_CLASSID	Class_ID(TAPEROSM_CLASS_ID,1)

class TaperWSM : public SimpleOSMToWSMObject {
	public:
		TaperWSM() {}
		TaperWSM(TaperMod *m) : SimpleOSMToWSMObject(m) {}
		void DeleteThis() { delete this; }
		SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() {return TAPERWSM_CLASSID;} 
		TCHAR *GetObjectName() {return GetString(IDS_RB_TAPER);}
		RefTargetHandle Clone(RemapDir& remap)
			{TaperWSM *newobj = new TaperWSM((TaperMod*)mod->Clone(remap));
		newobj->SimpleOSMToWSMClone(this,remap);
		BaseClone(this, newobj, remap);
		return newobj;}
	};


//--- ClassDescriptor and class vars ---------------------------------

IParamMap *TaperMod::pmapParam;

class TaperClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TaperMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_TAPER_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return  Class_ID(TAPEROSM_CLASS_ID,0); }
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static TaperClassDesc taperDesc;
extern ClassDesc* GetTaperModDesc() { return &taperDesc; }

class TaperWSMClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
		{if (loading) return new TaperWSM; else return new TaperWSM(new TaperMod);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_TAPER_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return TAPERWSM_CLASSID; }
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_MODBASED);}
	};

static TaperWSMClassDesc taperWSMDesc;
extern ClassDesc* GetTaperWSMDesc() { return &taperWSMDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_AMT			0
#define PB_CRV			1
#define PB_AXIS			2
#define PB_EFFECTAXIS	3
#define PB_SYMMETRY		4
#define PB_DOREGION		5
#define PB_FROM			6
#define PB_TO			7


//
//
// Parameters

static int axisIDs[] = {IDC_X,IDC_Y,IDC_Z};
static int effectAxisIDs[] = {IDC_EFFECT_X,IDC_EFFECT_Y,IDC_EFFECT_BOTH};

static ParamUIDesc descParam[] = {
	// Amount
	ParamUIDesc(
		PB_AMT,
		EDITTYPE_FLOAT,
		IDC_AMT,IDC_AMTSPINNER,
		-10.0f,10.0f,
		0.01f),

	// Curve
	ParamUIDesc(
		PB_CRV,
		EDITTYPE_FLOAT,
		IDC_CRV,IDC_CRVSPINNER,
		-10.0f,10.0f,
		0.01f),
	
	// Primary Axis
	ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),

	// Effect Axis
	ParamUIDesc(PB_EFFECTAXIS,TYPE_RADIO,effectAxisIDs,3),

	// Symmetry
	ParamUIDesc(PB_SYMMETRY,TYPE_SINGLECHEKBOX,IDC_TAPER_SYMMETRY),

	// Affect region
	ParamUIDesc(PB_DOREGION,TYPE_SINGLECHEKBOX,IDC_TAPER_AFFECTREGION),

	// From
	ParamUIDesc(
		PB_FROM,
		EDITTYPE_UNIVERSE,
		IDC_TAPER_FROM,IDC_TAPER_FROMSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// To
	ParamUIDesc(
		PB_TO,
		EDITTYPE_UNIVERSE,
		IDC_TAPER_TO,IDC_TAPER_TOSPIN,
		-BIGFLOAT,BIGFLOAT,		
		SPIN_AUTOSCALE),	
	};
#define PARAMDESC_LENGH 8


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 } };

#define PBLOCK_LENGTH	8

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TaperDlgProc -------------------------------

class TaperDlgProc : public ParamMapUserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void InitEffectAxis(HWND hWnd,TimeValue t,IParamMap *map);
		void DeleteThis() {}
	};
static TaperDlgProc theTaperProc;

void TaperDlgProc::InitEffectAxis(HWND hWnd,TimeValue t,IParamMap *map)
	{
	int axis;
	map->GetParamBlock()->GetValue(PB_AXIS,t,axis,FOREVER);
	switch (axis) {
		case 0:
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_X),_T("Z"));
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_Y),_T("Y"));
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_BOTH),_T("ZY"));
			break;
		case 1:
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_X),_T("X"));
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_Y),_T("Z"));
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_BOTH),_T("XZ"));
			break;
		case 2:
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_X),_T("X"));
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_Y),_T("Y"));
			SetWindowText(GetDlgItem(hWnd,IDC_EFFECT_BOTH),_T("XY"));
			break;
		}
	}

BOOL TaperDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			InitEffectAxis(hWnd,t,map);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_X:
				case IDC_Y:
				case IDC_Z:
					InitEffectAxis(hWnd,t,map);
					break;		
				}
			break;

		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_TAPER_FROMSPIN: {
					float from, to;
					map->GetParamBlock()->GetValue(PB_FROM,t,from,FOREVER);
					map->GetParamBlock()->GetValue(PB_TO,t,to,FOREVER);
					if (from>to) {
						map->GetParamBlock()->SetValue(PB_TO,t,from);
						map->Invalidate();
						}
					break;
					}
				
				case IDC_TAPER_TOSPIN: {
					float from, to;
					map->GetParamBlock()->GetValue(PB_FROM,t,from,FOREVER);
					map->GetParamBlock()->GetValue(PB_TO,t,to,FOREVER);
					if (from>to) {
						map->GetParamBlock()->SetValue(PB_FROM,t,to);
						map->Invalidate();
						}
					break;
					}
				}
			break;
		}
	return FALSE;
	}

//--- Bend methods -------------------------------


TaperMod::TaperMod()
	{
	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));

	pblock->SetValue(PB_AXIS, TimeValue(0), 2/*Z*/);
	pblock->SetValue(PB_EFFECTAXIS, TimeValue(0), 2/*XY*/);
	}

IOResult TaperMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,SIMPMOD_PBLOCKREF));
	return IO_OK;
	}

void TaperMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_TAPERPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&theTaperProc);
	}
		
void TaperMod::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
	{
	SimpleMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}

Interval TaperMod::GetValidity(TimeValue t)
	{
	float f;	
	int i;
	Interval valid = FOREVER;
	pblock->GetValue(PB_AMT,t,f,valid);
	pblock->GetValue(PB_CRV,t,f,valid);
	pblock->GetValue(PB_FROM,t,f,valid);
	pblock->GetValue(PB_TO,t,f,valid);
	pblock->GetValue(PB_SYMMETRY,t,i,valid);
	return valid;
	}

BOOL TaperMod::GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis)
	{
	int limit;
	pblock->GetValue(PB_DOREGION,t,limit,FOREVER);
	pblock->GetValue(PB_FROM,t,zmin,FOREVER);
	pblock->GetValue(PB_TO,t,zmax,FOREVER);
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	return limit?TRUE:FALSE;
	}

RefTargetHandle TaperMod::Clone(RemapDir& remap) 
	{	
	TaperMod* newmod = new TaperMod();
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone(remap));
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}

TaperDeformer::TaperDeformer() 
	{ 
	tm.IdentityMatrix();
	time = 0;	
	}

void TaperDeformer::SetAxis(Matrix3 &tmAxis)
	{
	Matrix3 itm = Inverse(tmAxis);
	tm    = tm*tmAxis;
	invtm =	itm*invtm;
	}

Point3 TaperDeformer::Map(int i, Point3 p)
	{
	float f, z, l;
	l = (bbox.pmax[naxis]-bbox.pmin[naxis]);
	if ( l == float(0) ) return p;
	p = p * tm;
	if (doRegion) {
		if (p.z<from) {
			z = from/l;
		} else 
		if (p.z>to) {
			z = to/l;
		} else {
			z = p.z/l;
			}
	} else {	
		z = p.z/l;
		}	
	if (sym && z<0.0f) z = -z;	
	f =  float(1.0) + z*k1 + k2*z*(float(1.0)-z);	
  	if (doX) p.x *= f;
  	if (doY) p.y *= f;
	p = p * invtm;
	return p;
	}

TaperDeformer::TaperDeformer(
		TimeValue t, ModContext &mc, 
		float  amt, float crv, int naxis,
		int eaxis, int sym,
		float from, float to, int doRegion,
 		Matrix3& modmat, Matrix3& modinv)
	{		
	this->naxis = naxis;
	this->doRegion = doRegion;
	this->from = from;
	this->to   = to;
	this->sym  = sym;
	switch (eaxis) {
		case 0: doX = TRUE;  doY = FALSE; break;
		case 1: doX = FALSE; doY = TRUE;  break;
		case 2: doX = TRUE;  doY = TRUE;  break;
		}
	Interval valid;
	Matrix3 mat;	
	time   = t;	
	tm = modmat;
	invtm = modinv;
	mat.IdentityMatrix();
	SetK(amt,crv);
	switch (naxis) {
		case 0: mat.RotateY( -HALFPI );	 break; //x
		case 1:	mat.RotateX( HALFPI );	break;  //y
		case 2: break;   //z
		}
	SetAxis( mat );
	bbox = *mc.box;
	}


Deformer& TaperMod::GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	static TaperDeformer deformer;
	float amt, crv, from, to;
	int doRegion, axis;
	int eaxis, sym;

	pblock->GetValue(PB_AMT,t,amt,FOREVER);
	pblock->GetValue(PB_CRV,t,crv,FOREVER);
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	pblock->GetValue(PB_EFFECTAXIS,t,eaxis,FOREVER);
	pblock->GetValue(PB_SYMMETRY,t,sym,FOREVER);
	pblock->GetValue(PB_FROM,t,from,FOREVER);
	pblock->GetValue(PB_TO,t,to,FOREVER);
	pblock->GetValue(PB_DOREGION,t,doRegion,FOREVER);
	
	deformer = TaperDeformer(t,mc,amt,crv,axis,eaxis,sym,from,to,doRegion,mat,invmat);
	return deformer;
	}

ParamDimension *TaperMod::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_AMT: 		return stdNormalizedDim; 
		case PB_CRV:		return stdNormalizedDim;		
		case PB_FROM:		return stdWorldDim;
		case PB_TO:			return stdWorldDim;
		case PB_SYMMETRY:	return defaultDim;
		default:			return defaultDim;
		}
	}

TSTR TaperMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_AMT:		return GetString(IDS_RB_AMOUNT);
		case PB_CRV:		return GetString(IDS_RB_CURVITURE);
		case PB_FROM:		return GetString(IDS_RB_FROM);
		case PB_TO:			return GetString(IDS_RB_TO);
		case PB_SYMMETRY:	return GetString(IDS_RB_SYMMETRY);
		default:			return TSTR(_T(""));
		}
	}




#endif // NO_MODIFIER_TAPER 
