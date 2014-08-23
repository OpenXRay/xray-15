/**********************************************************************
 *<
	FILE: skew.cpp

	DESCRIPTION:  Skew Modifier

	CREATED BY: Rolf Berteig

	HISTORY: created 18 October 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"

// in mods.cpp
extern HINSTANCE hInstance;

#define BIGFLOAT	float(999999)

class SkewMod : public SimpleMod {	
	public:
		static IParamMap *pmapParam;

		SkewMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_SKEWMOD); }  
		virtual Class_ID ClassID() { return Class_ID(SKEWOSM_CLASS_ID,0);}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_SKEW); }
		IOResult Load(ILoad *iload);

		// From simple mod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		BOOL GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
	};

class SkewDeformer: public Deformer {
	public:
		Matrix3 tm,invtm;
		Box3 bbox;		
		float from, to, amountOverLength;
		int doRegion;
		SkewDeformer();
		SkewDeformer(
			ModContext &mc,
			float amount, float dir, int naxis, 
			float from, float to, int doRegion, 
			Matrix3& modmat, Matrix3& modinv);		
		Point3 Map(int i, Point3 p); 
	};

#define SKEWWSM_CLASSID	Class_ID(SKEWOSM_CLASS_ID,1)

class SkewWSM : public SimpleOSMToWSMObject {
	public:
		SkewWSM() {}
		SkewWSM(SkewMod *m) : SimpleOSMToWSMObject(m) {}
		void DeleteThis() { delete this; }
		SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() {return SKEWWSM_CLASSID;} 
		TCHAR *GetObjectName() {return GetString(IDS_RB_SKEW);}
		RefTargetHandle Clone(RemapDir& remap)
		{SkewWSM *newobj = new SkewWSM((SkewMod*)mod->Clone(remap));
		newobj->SimpleOSMToWSMClone(this,remap);
		BaseClone(this, newobj, remap);
		return newobj;}
	};



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *SkewMod::pmapParam = NULL;



class SkewClassDesc:public ClassDesc {
	public:
#ifdef MODIFIER_SKEW_PRIVATE
	int 			IsPublic() { return 0; }
#else
	int 			IsPublic() { return 1; }
#endif
	void *			Create(BOOL loading = FALSE) { return new SkewMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_SKEW_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SKEWOSM_CLASS_ID,0); }
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static SkewClassDesc skewDesc;
extern ClassDesc* GetSkewModDesc() { return &skewDesc; }

class SkewWSMClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
		{if (loading) return new SkewWSM; else return new SkewWSM(new SkewMod);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_SKEW_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return SKEWWSM_CLASSID; }
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_MODBASED);}
	};

static SkewWSMClassDesc skewWSMDesc;
extern ClassDesc* GetSkewWSMDesc() { return &skewWSMDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_AMOUNT	0
#define PB_DIR		1
#define PB_AXIS		2
#define PB_DOREGION	3
#define PB_FROM		4
#define PB_TO		5


//
//
// Parameters

static int axisIDs[] = {IDC_X,IDC_Y,IDC_Z};

static ParamUIDesc descParam[] = {
	// Amount
	ParamUIDesc(
		PB_AMOUNT,
		EDITTYPE_UNIVERSE,
		IDC_SKEW_AMOUNT,IDC_SKEW_AMOUNTSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Direction
	ParamUIDesc(
		PB_DIR,
		EDITTYPE_FLOAT,
		IDC_DIR,IDC_DIRSPINNER,
		-BIGFLOAT,BIGFLOAT,
		0.5f),

	// Axis
	ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),

	// Affect region
	ParamUIDesc(PB_DOREGION,TYPE_SINGLECHEKBOX,IDC_SKEW_AFFECTREGION),

	// From
	ParamUIDesc(
		PB_FROM,
		EDITTYPE_UNIVERSE,
		IDC_SKEW_FROM,IDC_SKEW_FROMSPIN,
		-BIGFLOAT,0.0f,
		SPIN_AUTOSCALE),

	// To
	ParamUIDesc(
		PB_TO,
		EDITTYPE_UNIVERSE,
		IDC_SKEW_TO,IDC_SKEW_TOSPIN,
		0.0f,BIGFLOAT,		
		SPIN_AUTOSCALE),	
	};
#define PARAMDESC_LENGH 6


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },	
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 } };
#define PBLOCK_LENGTH	6

#if 0
// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);
#endif

#define CURRENT_VERSION	0

//--- SkewDlgProc -------------------------------


class SkewDlgProc : public ParamMapUserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {}
	};
static SkewDlgProc theSkewProc;

BOOL SkewDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_SKEW_FROMSPIN: {
					float from, to;
					map->GetParamBlock()->GetValue(PB_FROM,t,from,FOREVER);
					map->GetParamBlock()->GetValue(PB_TO,t,to,FOREVER);
					if (from>to) {
						map->GetParamBlock()->SetValue(PB_TO,t,from);
						map->Invalidate();
						}
					break;
					}
				
				case IDC_SKEW_TOSPIN: {
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

//--- Skew methods -------------------------------


SkewMod::SkewMod() : SimpleMod()
	{	
	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
		CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_AXIS, TimeValue(0), 2/*Z*/);
	}

IOResult SkewMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	//iload->RegisterPostLoadCallback(
	//	new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,SIMPMOD_PBLOCKREF));
	return IO_OK;
	}

void SkewMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_SKEWPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&theSkewProc);
	}
		
void SkewMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	SimpleMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}

Interval SkewMod::GetValidity(TimeValue t)
	{
	float f;	
	Interval valid = FOREVER;
	pblock->GetValue(PB_AMOUNT,t,f,valid);
	pblock->GetValue(PB_DIR,t,f,valid);	
	pblock->GetValue(PB_FROM,t,f,valid);
	pblock->GetValue(PB_TO,t,f,valid);
	return valid;
	}

BOOL SkewMod::GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis)
	{
	int limit;
	pblock->GetValue(PB_DOREGION,t,limit,FOREVER);
	pblock->GetValue(PB_FROM,t,zmin,FOREVER);
	pblock->GetValue(PB_TO,t,zmax,FOREVER);
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	return limit?TRUE:FALSE;
	}

RefTargetHandle SkewMod::Clone(RemapDir& remap) 
	{	
	SkewMod* newmod = new SkewMod();	
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone(remap));
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}

SkewDeformer::SkewDeformer() 
	{ 
	tm.IdentityMatrix();
	invtm.IdentityMatrix();
	}

Point3 SkewDeformer::Map(int i, Point3 p)
	{
	float z;	
	p = p * tm;
	z = p.z;
	if (doRegion) {
		if (p.z<from) {
			z = from;
		} else 
		if (p.z>to) {
			z = to;
			}
		}			
	p.x += z * amountOverLength;
	return p * invtm;	
	}

SkewDeformer::SkewDeformer(
		ModContext &mc,
		float amount, float dir, int naxis, 
		float from, float to, int doRegion,
		Matrix3& modmat, Matrix3& modinv) 
	{	
	this->doRegion = doRegion;
	this->from = from;
	this->to   = to;
	Matrix3 mat;
	
	mat.IdentityMatrix();	
	switch (naxis) {
		case 0: mat.RotateY( -HALFPI );	 break; //X
		case 1: mat.RotateX( HALFPI );  break; //Y
		case 2: break;  //Z
		}
	mat.RotateZ(DegToRad(dir));		
	tm    = modmat * mat;
	invtm = Inverse(mat) * modinv;
	
	assert (mc.box);
	bbox = *mc.box;
	
	float len = 0.0f;
	if (!doRegion) {
		switch (naxis) {
			case 0:  len = bbox.pmax.x - bbox.pmin.x; break;
			case 1:	 len = bbox.pmax.y - bbox.pmin.y; break;
			case 2:  len = bbox.pmax.z - bbox.pmin.z; break;
			}
	} else {
		len = to-from;
		}		
	if (len==0.0f) len = 0.000001f;
	amountOverLength = amount/len;
	} 


Deformer& SkewMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	float amount, dir, from, to;
	int axis;	
	int doRegion;
	pblock->GetValue(PB_AMOUNT,t,amount,FOREVER);
	pblock->GetValue(PB_DIR,t,dir,FOREVER);
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	pblock->GetValue(PB_FROM,t,from,FOREVER);
	pblock->GetValue(PB_TO,t,to,FOREVER);
	pblock->GetValue(PB_DOREGION,t,doRegion,FOREVER);
	static SkewDeformer deformer;
	deformer = SkewDeformer(mc,amount,dir,axis,from,to,doRegion,mat,invmat);
	return deformer;
	}

ParamDimension *SkewMod::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_AMOUNT: return stdWorldDim;
		case PB_DIR:	return defaultDim;
		case PB_FROM:	return stdWorldDim;
		case PB_TO:		return stdWorldDim;
		default:		return defaultDim;
		}
	}

TSTR SkewMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_AMOUNT:	return GetString(IDS_RB_AMOUNT);
		case PB_DIR:	return GetString(IDS_RB_DIRECTION);
		case PB_FROM:	return GetString(IDS_RB_FROM);
		case PB_TO:		return GetString(IDS_RB_TO);
		default:		return TSTR(_T(""));
		}
	}







