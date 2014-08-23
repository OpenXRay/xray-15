/**********************************************************************
 *<
	FILE: TINT.CPP

	DESCRIPTION: TINT Composite.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "buildver.h"
#ifndef NO_MAPTYPE_RGBTINT // orb 01-03-2001 Removing map types

extern HINSTANCE hInstance;

static LRESULT CALLBACK CurveWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

#define NSUBTEX 1    // number of texture map slots
#define NCOLS 3      // number of color swatches

static Class_ID tintClassID(TINT_CLASS_ID,0);


//--------------------------------------------------------------
// Tint: A Composite texture map
//--------------------------------------------------------------
class Tint: public MultiTex { 
	friend class TintPostLoad;
	Color col[NCOLS];
	Texmap* subTex[NSUBTEX];  // More refs
	Interval ivalid;
	BOOL rollScroll;
	public:
		BOOL Param1;
		BOOL mapOn[NSUBTEX];
		IParamBlock2 *pblock;   // ref #0
		Tint();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void SetColor(int i, Color c, TimeValue t);
		void NotifyChanged();

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

		Class_ID ClassID() {	return tintClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_RGBTINT); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 1+NSUBTEX; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 1+NSUBTEX; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

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

int numTints = 0;
class TintClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Tint; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_RGBTINT_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return tintClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COLMOD;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("rgbTint"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static TintClassDesc tintCD;

ClassDesc* GetTintDesc() { return &tintCD;  }

enum { rgbtint_params };  // pblock ID
// rgbtints param IDs
enum 
{ 
	rgbtint_color1, rgbtint_color2, rgbtint_color3,
	rgbtint_map1,
	rgbtint_map1_on, // main grad params 
};

static ParamBlockDesc2 rgbtint_param_blk ( rgbtint_params, _T("parameters"),  0, &tintCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_TINT, IDS_DS_TINTPARAMS, 0, 0, NULL, 
	// params
	rgbtint_color1,	 _T("red"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_RED,	
		p_default,		Color(1.f,0.f,0.f), 
		p_ui,			TYPE_COLORSWATCH, IDC_TINT_R, 
		end,
	rgbtint_color2,	 _T("green"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_GREEN,	
		p_default,		Color(0.f,1.f,0.f), 
		p_ui,			TYPE_COLORSWATCH, IDC_TINT_G, 
		end,
	rgbtint_color3,	 _T("blue"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_BLUE,	
		p_default,		Color(0.f,0.f,1.f), 
		p_ui,			TYPE_COLORSWATCH, IDC_TINT_B, 
		end,
	rgbtint_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		1,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TINT_MAP,
		end,
	rgbtint_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON,
		end,
	end
);




//-----------------------------------------------------------------------------
//  Tint
//-----------------------------------------------------------------------------

#define TINT_VERSION 2

#define NPARAMS 3

static int name_id[NPARAMS] = {IDS_DS_COLOR1, IDS_DS_COLOR2, IDS_DS_COLOR3};

static ParamBlockDescID pbdesc[] = {
	{ TYPE_RGBA, NULL, TRUE,rgbtint_color1 },   // col1
	{ TYPE_RGBA, NULL, TRUE,rgbtint_color2 },   // col2
	{ TYPE_RGBA, NULL, TRUE,rgbtint_color3 }    // col3
	};   

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,3,1),	// Version 1 params
};


void Tint::Init() {
	ivalid.SetEmpty();
	SetColor(0, Color(1.0f,0.0f,0.0f), TimeValue(0));
	SetColor(1, Color(0.0f,1.0f,0.0f), TimeValue(0));
	SetColor(2, Color(0.0f,0.0f,1.0f), TimeValue(0));
	mapOn[0] = 1;
	}

void Tint::Reset() {
	tintCD.Reset(this, TRUE);	// reset all pb2's
	for (int i=0; i<NSUBTEX; i++) 
		DeleteReference(i+1);	// get rid of maps
	Init();
	}

void Tint::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Tint::Tint() {
	for (int i=0; i<NSUBTEX; i++) subTex[i] = NULL;
	pblock = NULL;
	tintCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

static AColor white(1.0f,1.0f,1.0f,1.0f);

AColor Tint::EvalColor(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	AColor c =  mapOn[0]&&subTex[0]? subTex[0]->EvalColor(sc): white;
	AColor c2;
	c2.r = c.r*col[0].r + c.g*col[1].r + c.b*col[2].r;		
	c2.g = c.r*col[0].g + c.g*col[1].g + c.b*col[2].g;		
	c2.b = c.r*col[0].b + c.g*col[1].b + c.b*col[2].b;
	c2.a = c.a;
	return c2;	
	}

float Tint::EvalMono(ShadeContext& sc) {
	return Intens(EvalColor(sc));
	}

Point3 Tint::EvalNormalPerturb(ShadeContext& sc) {
	return subTex[0]&&mapOn[0] ? subTex[0]->EvalNormalPerturb(sc): Point3(0,0,0);
	}

RefTargetHandle Tint::Clone(RemapDir &remap) {
	Tint *mnew = new Tint();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	for (int i=0; i<NCOLS; i++)
		mnew->col[i] = col[i];
	mnew->ivalid.SetEmpty();	
	for (i = 0; i<NSUBTEX; i++) {
	    mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+1,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Tint::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	IAutoMParamDlg* masterDlg = tintCD.CreateParamDlgs(hwMtlEdit, imp, this);
	return masterDlg;

	}

//static int pbId[NPARAMS] = { PB_COL1, PB_COL2, PB_COL3};

void Tint::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( rgbtint_map1_on, 0, mapOn[0]);
		Param1 = FALSE;
		}
	
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( rgbtint_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( rgbtint_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( rgbtint_color3, t, col[2], ivalid );
		col[2].ClampMinMax();
		pblock->GetValue( rgbtint_map1_on, t,mapOn[0], ivalid );

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}


void Tint::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	pblock->SetValue( i==0?rgbtint_color1:(i==1?rgbtint_color2:rgbtint_color3), t, c);
	}

RefTargetHandle Tint::GetReference(int i) {
	switch(i) {
		case 0:	return pblock ;
		default:return subTex[i-1];
		}
	}

void Tint::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-1] = (Texmap *)rtarg; break;
		}
	}

void Tint::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+1,m);
	if (i==0)
		{
		rgbtint_param_blk.InvalidateUI(rgbtint_map1);
		ivalid.SetEmpty();
		}	

	}

TSTR Tint::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_MAP)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Tint::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: return subTex[i-1]; 
		}
	}

TSTR Tint::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-1);
		}
	}

RefResult Tint::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget== pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//			if (hTarget == pblock) 
				rgbtint_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}

			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult Tint::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
	return IO_OK;
	}	
	  
class RGBTint2PostLoad : public PostLoadCallback {
	public:
		Tint *n;
		RGBTint2PostLoad(Tint *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->Param1)
				{
				n->pblock->SetValue( rgbtint_map1_on, 0, n->mapOn[0]);
				}
			delete this; 


			} 
	};




IOResult Tint::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK:
				mapOn[0] = 0; 
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE; 
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &rgbtint_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

//load->RegisterPostLoadCallback(new RGBTint2PostLoad(this));

	return IO_OK;
	}

#endif // NO_MAPTYPE_RGBTINT