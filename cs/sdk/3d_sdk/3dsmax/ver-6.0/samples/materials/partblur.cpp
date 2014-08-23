/**********************************************************************
 *<
	FILE: PARTBLUR.CPP

	DESCRIPTION: Particle motion blur

	CREATED BY: Rolf Berteig

	HISTORY: created 2/8/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "buildver.h"

#ifndef NO_PARTICLES // orb 07-11-01
#ifndef NO_MAPTYPE_PARTICLEMBLUR // orb 01-03-2001 Removing map types

extern HINSTANCE hInstance;

#define PARTBLUR_CLASSID	Class_ID(0x8a746be5,0x81163ef6)

//class PartBlurParamDlg;

class PartBlur : public Texmap { 
	public:
		IParamBlock2 *pblock;

		// Caches
		Interval ivalid;
		CRITICAL_SECTION csect;
		Color color1, color2;
		float sharp;

		PartBlur();
		~PartBlur() { DeleteCriticalSection(&csect); }

		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		ULONG Requirements(int subMtlNum) {return MTLREQ_XYZ;}
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) {Update(t,FOREVER); return ivalid;}		

		// Evaluation
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);		
		Point3 EvalNormalPerturb(ShadeContext& sc);

		Class_ID ClassID() {return PARTBLUR_CLASSID;}
		SClass_ID SuperClassID() {return TEXMAP_CLASS_ID;}
		void GetClassName(TSTR& s) { s = GetString(IDS_RB_PARTBLUR); } // mjm - 2.3.99
		void DeleteThis() {delete this;}	

		int NumSubs() {return 1;}  
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) {return TSTR(GetString(IDS_DS_PARAMETERS));}
		
 		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock2*)rtarg;}

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		IOResult Load(ILoad *iload);
		
		// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		bool IsLocalOutputMeaningful( ShadeContext& sc );

	};
class PartBlurClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new PartBlur;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_PARTBLUR_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() {return TEXMAP_CLASS_ID;}
	Class_ID 		ClassID() {return PARTBLUR_CLASSID;}
	const TCHAR* 	Category() {return TEXMAP_CAT_3D;}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("particleBlur"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static PartBlurClassDesc partBlurCD;
ClassDesc* GetPartBlurDesc() {return &partBlurCD;}


enum { pblur_params };  // pblock ID
// pblur_params param IDs
enum 
{ 
	pblur_color1, pblur_color2,
	pblur_sharp
};


static ParamBlockDesc2 pblur_param_blk ( pblur_params, _T("parameters"),  0, &partBlurCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PBLUR_PARAMS, IDS_RB_PBLURPARAMS, 0, 0, NULL, 
	// params
	pblur_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(1.0f,1.0f,1.0f), 
		p_ui,			TYPE_COLORSWATCH, IDC_PBLUR_COLOR1, 
		end,
	pblur_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.f,0.f,0.f), 
		p_ui,			TYPE_COLORSWATCH, IDC_PBLUR_COLOR2, 
		end,
	pblur_sharp,	_T("sharp"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_SHARP,
		p_default,		2.f,
		p_range,		0.0, 9999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PBLUR_SHARP, IDC_PBLUR_SHARPSPIN, 0.01f, 
		end,

	end
);




#define PARAMDESC_LENGH 3

static ParamBlockDescID descVer0[] = {
	{ TYPE_RGBA, NULL, TRUE, pblur_color1 }, // color1
	{ TYPE_RGBA, NULL, TRUE,  pblur_color2 }, // color2
	{ TYPE_FLOAT, NULL, TRUE,  pblur_sharp },	// sharp
	};
#define PBLOCK_LENGTH	3

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),	
	};
#define NUM_OLDVERSIONS	0

#define CURRENT_VERSION	1


//--- PartBlur Methods -----------------------------------------------

PartBlur::PartBlur()
	{
	pblock = NULL;
	partBlurCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	InitializeCriticalSection(&csect);
	}

ParamDlg* PartBlur::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
	IAutoMParamDlg* masterDlg = partBlurCD.CreateParamDlgs(hwMtlEdit, imp, this);
	return masterDlg;
	}

void PartBlur::Update(TimeValue t, Interval& valid)
	{
	EnterCriticalSection(&csect);
	if (!ivalid.InInterval(t)) {
		ivalid = FOREVER;		
		pblock->GetValue(pblur_color1,t,color1,ivalid);
		pblock->GetValue(pblur_color2,t,color2,ivalid);
		pblock->GetValue(pblur_sharp,t,sharp,ivalid);		
		}
	valid &= ivalid;
	LeaveCriticalSection(&csect);
	}

void PartBlur::Init()
	{
	pblock->SetValue(pblur_color1,0,Point3(1,1,1));
	pblock->SetValue(pblur_sharp,0,2.0f);
	ivalid.SetEmpty();
	}

void PartBlur::Reset()
	{
	partBlurCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}

AColor PartBlur::EvalColor(ShadeContext& sc)
	{
	if (gbufID) sc.SetGBufferID(gbufID);
	Object *ob = sc.GetEvalObject();
	if (ob && ob->IsParticleSystem()) {
		ParticleObject *obj = (ParticleObject*)ob;
		TimeValue t = sc.CurTime();
		Point3 pos  = sc.PointTo(sc.P(),REF_WORLD);
		Point3 ppos = obj->ParticlePosition(t,sc.mtlNum);
		Point3 v    = obj->ParticleVelocity(t,sc.mtlNum);
		float size  = obj->ParticleSize(t,sc.mtlNum);
		float size2 = size/2.0f;
		float s = sharp/size2;
		float u = 0.0f;
		if (size2<=sharp) {
			u = 1.0f;
			s = 1.0f;
		} else {
			int ct      = obj->ParticleCenter(t,sc.mtlNum);
			float proj  = DotProd(pos-ppos,v)/Length(v);
			switch (ct) {
				case PARTCENTER_HEAD:
					proj += size2;
					break;
				case PARTCENTER_TAIL:
					proj -= size2;
					break;
				}
			proj = (float)fabs(proj);
			if (proj<sharp) u = 1.0f;
			else {
				u = 1.0f-(proj-sharp)/(size2-sharp);		
				}
			}
		u *= s;
		if (u<0.0) u = 0.0f;
		if (u>1.0f) u = 1.0f;
		return u*color1 + (1-u)*color2;
	} else {
		return AColor(0,0,0);
		}
	}

float PartBlur::EvalMono(ShadeContext& sc)
	{
	return Intens(EvalColor(sc));
	}

Point3 PartBlur::EvalNormalPerturb(ShadeContext& sc)
	{
	return Point3(0,0,0);
	}

RefTargetHandle PartBlur::Clone(RemapDir &remap)
	{
	PartBlur *map = new PartBlur;
	*((MtlBase*)map) = *((MtlBase*)this);  // copy superclass stuff
	map->ReplaceReference(0,remap.CloneRef(pblock));	
	BaseClone(this, map, remap);
	return map;
	}

RefResult PartBlur::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				pblur_param_blk.InvalidateUI(changing_param);
				//NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				}
			break;
		}
	return REF_SUCCEED;
	}


IOResult PartBlur::Load(ILoad *iload)
	{
	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &pblur_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);
	return IO_OK;
	}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// It is meaningful if used with a particle system 
//
bool PartBlur::IsLocalOutputMeaningful( ShadeContext& sc )
{
	Object *ob = sc.GetEvalObject();
	if (ob && ob->IsParticleSystem()) 
		return true;

	return false;
}
#endif // NO_MAPTYPE_PARTICLEMBLUR
#endif // NO_PARTICLES