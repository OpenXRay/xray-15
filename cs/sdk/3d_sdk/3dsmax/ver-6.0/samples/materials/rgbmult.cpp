/**********************************************************************
 *<
	FILE: RGBMULT.CPP

	DESCRIPTION: RGBMULT Composite.

	CREATED BY: Dan Silva

	HISTORY: 12/3/98 Updated to Param Block2 Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "iparamm2.h"
#include "buildver.h"
#ifndef NO_MAPTYPE_RGBMULT // orb 01-03-2001 Removing map types

extern HINSTANCE hInstance;

#define NSUBTEX 2    // number of texture map slots
#define NCOLS 2

static Class_ID rgbmultClassID(RGBMULT_CLASS_ID,0);

static int subTexId[NSUBTEX] = { IDC_MULT_TEX1, IDC_MULT_TEX2 };

#define ALPHA_FROM_1 0
#define ALPHA_FROM_2 1
#define ALPHA_FROM_MULT 2

//--------------------------------------------------------------
// RGBMult: A Composite texture map
//--------------------------------------------------------------
class RGBMult: public Texmap { 
	public:
	BOOL Param1;
	IParamBlock2 *pblock;   // ref #0
	Texmap* subTex[NSUBTEX];  // refs 1,2;
	BOOL mapOn[NSUBTEX];
	Color col[NCOLS];
	Interval ivalid;
	int alphaFrom;
	BOOL rollScroll;
	BOOL loadingOld;
		RGBMult();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void NotifyChanged();

		void SetColor(int i, Color c, TimeValue t);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {	return rgbmultClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_RGBMULT); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return NSUBTEX+1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return NSUBTEX+1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int RemapRefOnLoad(int iref); 

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

	};

class RGBMultClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new RGBMult; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_RGBMULT_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return rgbmultClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COMP;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("rgbMult"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static RGBMultClassDesc maskCD;

ClassDesc* GetRGBMultDesc() { return &maskCD;  }

enum { rgbmult_params };  // pblock ID
// rgbmult_params param IDs
enum 
{ 
	rgbmult_color1, rgbmult_color2,
	rgbmult_map1, rgbmult_map2,		
	rgbmult_map1_on, rgbmult_map2_on, // main grad params 
	rgbmult_type,
};


static ParamBlockDesc2 rgbmult_param_blk ( rgbmult_params, _T("parameters"),  0, &maskCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_RGBMULT, IDS_DS_RGBMULTPARAMS, 0, 0, NULL, 
	// params
	rgbmult_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_MULT_COL1, 
		end,
	rgbmult_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_MULT_COL2, 
		end,
	rgbmult_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		1,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MULT_TEX1,
		end,
	rgbmult_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		2,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MULT_TEX2,
		end,
	rgbmult_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	rgbmult_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	rgbmult_type, _T("alphaFrom"), TYPE_INT,				0,				IDS_PW_ALPHAFROM,
		p_default,		2,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 3, IDC_MULT_ALPHA1, IDC_MULT_ALPHA2, IDC_MULT_ALPHA3,
		end,

	end
);


//-----------------------------------------------------------------------------
//  RGBMult
//-----------------------------------------------------------------------------
#define NPARAMS 2

static int name_id[NPARAMS] = {IDS_DS_COLOR1, IDS_DS_COLOR2};

#define RGBMULT_VERSION 2
static ParamBlockDescID pbdesc[NPARAMS] = {
	{ TYPE_RGBA, NULL, TRUE,rgbmult_color1 },  // col1
	{ TYPE_RGBA, NULL, TRUE,rgbmult_color2 }   // col2
	};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,2,1),	// Version 1 params
};


void RGBMult::Init() {
	ivalid.SetEmpty();
	alphaFrom = ALPHA_FROM_MULT;
	SetColor(0, Color(1.0f,1.0f,1.0f), TimeValue(0));
	SetColor(1, Color(1.0f,1.0f,1.0f), TimeValue(0));
	mapOn[0] = mapOn[1] = 1;
	}

void RGBMult::Reset() {
	maskCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(1);	// get rid of maps
	DeleteReference(2);	// get rid of maps
	Init();
	}

void RGBMult::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

RGBMult::RGBMult() {
	for (int i=0; i<NSUBTEX; i++) subTex[i] = NULL;
	pblock = NULL;
	maskCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}


static AColor white(1.0f,1.0f,1.0f,1.0f);

AColor RGBMult::EvalColor(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	AColor c0 = (subTex[0]&&mapOn[0])? subTex[0]->EvalColor(sc): col[0];
	AColor c1 = (subTex[1]&&mapOn[1])? subTex[1]->EvalColor(sc): col[1];
	AColor c;
	c.r = c0.r*c1.r;
	c.g = c0.g*c1.g;
	c.b = c0.b*c1.b;
	switch(alphaFrom) {
		case ALPHA_FROM_1: c.a = c0.a; break;
		case ALPHA_FROM_2: c.a = c1.a; break;
		case ALPHA_FROM_MULT: c.a = c0.a*c1.a; break;
		}
	return c;
	}

float RGBMult::EvalMono(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	float m = (subTex[1]&&mapOn[1])? subTex[1]->EvalMono(sc): Intens(col[0]);
	float c0 = (subTex[0]&&mapOn[0])? subTex[0]->EvalMono(sc): Intens(col[1]);
	return m*c0;
	}

Point3 RGBMult::EvalNormalPerturb(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	Texmap *sub0 = mapOn[0]?subTex[0]:NULL;
	Texmap *sub1 = mapOn[1]?subTex[1]:NULL;
	Point3 p0,p1;
	float m0, m1;
	if (sub0&&sub1) {
		m0 = sub0->EvalMono(sc);
		p0 = sub0->EvalNormalPerturb(sc);
		m1 = sub1->EvalMono(sc);
		p1 = sub1->EvalNormalPerturb(sc);
		return p0*m1+p1*m0;
		}
	else {
		if (sub0)
			return sub0->EvalNormalPerturb(sc);
		else if (sub1)
			return sub1->EvalNormalPerturb(sc);
	
		else
			return Point3(0.0f,0.0f,0.0f);
	}
//	Point3 p0  = subTex[0]&&mapOn[0]? subTex[0]->EvalNormalPerturb(sc): Point3(0.0f,0.0f,0.0f);
//	Point3 p1  = subTex[1]&&mapOn[1]? subTex[1]->EvalNormalPerturb(sc): Point3(0.0f,0.0f,0.0f);
//	return p0+p1;
	}

RefTargetHandle RGBMult::Clone(RemapDir &remap) {
	RGBMult *mnew = new RGBMult();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+1,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	mnew->alphaFrom = alphaFrom;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* RGBMult::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	IAutoMParamDlg* masterDlg = maskCD.CreateParamDlgs(hwMtlEdit, imp, this);
	return masterDlg;

	}

void RGBMult::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( rgbmult_map1_on, 0, mapOn[0]);
		pblock->SetValue( rgbmult_map2_on, 0, mapOn[1]);
		pblock->SetValue( rgbmult_type, 0, alphaFrom);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( rgbmult_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( rgbmult_color2, t, col[1], ivalid );

		pblock->GetValue( rgbmult_map1_on, t, mapOn[0], ivalid );
		pblock->GetValue( rgbmult_map2_on, t, mapOn[1], ivalid );
		pblock->GetValue( rgbmult_type, t, alphaFrom, ivalid );


		col[1].ClampMinMax();
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

void RGBMult::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	int id = i==0?rgbmult_color1:rgbmult_color2;
	pblock->SetValue( id, t, c);
	}

RefTargetHandle RGBMult::GetReference(int i) {
	if (i==0) return pblock;
	else return subTex[i-1];
	}

int RGBMult::RemapRefOnLoad(int iref) { 
	if (loadingOld)  
		return iref+1;
	else return iref;
	}

void RGBMult::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-1] = (Texmap *)rtarg; break;
		} 
	}

void RGBMult::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+1,m);
	if (i==0)
		{
		rgbmult_param_blk.InvalidateUI(rgbmult_map1);
		ivalid.SetEmpty();
		}	
	else if (i==1)
		{
		rgbmult_param_blk.InvalidateUI(rgbmult_map2);
		ivalid.SetEmpty();
		}	

	}

TSTR RGBMult::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* RGBMult::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: return subTex[i-1]; 
		}
	}

TSTR RGBMult::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-1);
		}
	}

RefResult RGBMult::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock) 
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				rgbmult_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}

			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define INVERT_RGBMULT_CHUNK 0x2000
#define ALPHA_FROM_CHUNK 0x2010
#define RGBMULT_VERSION_CHUNK  0x2020
#define PARAM2_CHUNK  0x2030

IOResult RGBMult::Save(ISave *isave) { 
	IOResult res;
	ULONG nb;

	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	int vers = RGBMULT_VERSION;
	isave->BeginChunk(RGBMULT_VERSION_CHUNK);
	isave->Write(&vers,sizeof(vers),&nb);			
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
	return IO_OK;
	}	
	  
class RGBMultPostLoad : public PostLoadCallback {
	public:
		RGBMult *tm;
		RGBMultPostLoad(RGBMult *b) {tm=b;}
		void proc(ILoad *iload) {  tm->loadingOld = FALSE; delete this; } 
	};

class RGBMult2PostLoad : public PostLoadCallback {
	public:
		RGBMult *n;
		RGBMult2PostLoad(RGBMult *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->Param1)
				{
				n->pblock->SetValue( rgbmult_map1_on, 0, n->mapOn[0]);
				n->pblock->SetValue( rgbmult_map2_on, 0, n->mapOn[1]);
				n->pblock->SetValue( rgbmult_type, 0, n->alphaFrom);
				}
			delete this; 


			} 
	};



IOResult RGBMult::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	int id;
	loadingOld = TRUE;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case PARAM2_CHUNK:
				Param1= FALSE;
				break;
			case RGBMULT_VERSION_CHUNK:
				loadingOld = FALSE;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			case ALPHA_FROM_CHUNK:
				res = iload->Read(&alphaFrom,sizeof(alphaFrom), &nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	if (loadingOld) 
		iload->RegisterPostLoadCallback(new RGBMultPostLoad(this));

	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &rgbmult_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

//	iload->RegisterPostLoadCallback(new RGBMult2PostLoad(this));



	return IO_OK;
	}


#endif // NO_MAPTYPE_RGBMULT