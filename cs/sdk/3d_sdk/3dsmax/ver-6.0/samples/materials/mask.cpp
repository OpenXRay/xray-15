/**********************************************************************
 *<
	FILE: MASK.CPP

	DESCRIPTION: MASK Composite.

	CREATED BY: Dan Silva

	HISTORY:12/2/98 Updated to Param Block 2 Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"

extern HINSTANCE hInstance;
#include "iparamm2.h"



#define NSUBTEX 2    // number of texture map slots

static Class_ID maskClassID(MASK_CLASS_ID,0);

static int subTexId[NSUBTEX] = { IDC_MASK_MAP, IDC_MASK_MASK };

//--------------------------------------------------------------
// Mask: A Composite texture map
//--------------------------------------------------------------
class Mask: public Texmap { 
	Texmap* subTex[NSUBTEX];  // refs
	Interval ivalid;
	BOOL rollScroll;
	public:
		BOOL Param1;
		Mask();
		BOOL mapOn[NSUBTEX];
		BOOL invertMask;

		IParamBlock2 *pblock;   // ref #2
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

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

		Class_ID ClassID() {	return maskClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_MASK); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return NSUBTEX+1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return NSUBTEX+1; }
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

		bool IsLocalOutputMeaningful( ShadeContext& sc );
	};

class MaskClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Mask; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_MASK_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return maskClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COMP;  }
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("maskTex"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};

static MaskClassDesc maskCD;

ClassDesc* GetMaskDesc() { return &maskCD;  }

enum { mask_params, };  // pblock ID
// mask_params param IDs
enum 
{ 
	mask_map1, mask_map2,		
	mask_map1_on, mask_map2_on, // main grad params 
	mask_invert
};

// per instance gradient block
static ParamBlockDesc2 mask_param_blk ( mask_params, _T("parameters"),  0, &maskCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 2, 
	//rollout
	IDD_MASK, IDS_MASKPARMS, 0, 0, NULL, 
	// params
	mask_map1,		_T("map"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_DS_MAP,
		p_refno,		0,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MASK_MAP,
		end,
	mask_map2,		_T("mask"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_DS_MASK,
		p_refno,		1,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MASK_MASK,
		end,
	mask_map1_on,	_T("mapEnabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	mask_map2_on,	_T("maskEnabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	mask_invert,	_T("maskInverted"), TYPE_BOOL,			0,				IDS_PW_INVERT,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_INVERT_MASK,
		end,
	end
);


//-----------------------------------------------------------------------------
//  Mask
//-----------------------------------------------------------------------------


void Mask::Init() {
	ivalid.SetEmpty();
	invertMask = 0;
	}

void Mask::Reset() {
	for (int i=0; i<NSUBTEX; i++) {
		DeleteReference(i);	// get rid of maps
		mapOn[i] = 1;
		}
	maskCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}

void Mask::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Mask::Mask() {
	Param1 = FALSE;
	mapOn[0] = mapOn[1] = 1;
	for (int i=0; i<NSUBTEX; i++) subTex[i] = NULL;
	pblock = NULL;
	maskCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}


static AColor white(1.0f,1.0f,1.0f,1.0f);

AColor Mask::EvalColor(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	float m = 1.0f;
	if (subTex[1]&&mapOn[1]) {
		m = subTex[1]->EvalMono(sc);
		if (invertMask) m = 1.0f-m;
		}
	AColor c0 = subTex[0]&&mapOn[0]? subTex[0]->EvalColor(sc): white;
	if(m==1.0f)   
		return c0;
	else 
		return m*c0;
	}

float Mask::EvalMono(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	float m = 1.0f;
	if (subTex[1]&&mapOn[1]) {
		m = subTex[1]->EvalMono(sc);
		if (invertMask) m = 1.0f-m;
		}
	float c0 = subTex[0]&&mapOn[0]? subTex[0]->EvalMono(sc): 1.0f;
	return m*c0;
	}

Point3 Mask::EvalNormalPerturb(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	float m = 1.0f;
	if (subTex[1]&&mapOn[1]) {
		m = subTex[1]->EvalMono(sc);
		if (invertMask) m = 1.0f-m;
		}
	Point3 p0  = subTex[0]&&mapOn[0]? subTex[0]->EvalNormalPerturb(sc): Point3(0.0f,0.0f,0.0f);
	return m*p0;
	}

RefTargetHandle Mask::Clone(RemapDir &remap) {
	Mask *mnew = new Mask();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(2,remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		mnew->invertMask = invertMask;
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Mask::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	IAutoMParamDlg* masterDlg = maskCD.CreateParamDlgs(hwMtlEdit, imp, this);
//attach a dlg proc to handle the swap button 
	// add the secondary dialogs to the master
	return masterDlg;

	}

void Mask::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( mask_map1_on, 0, mapOn[0]);
		pblock->SetValue( mask_map2_on, 0, mapOn[1]);
		pblock->SetValue( mask_invert, 0, invertMask);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {

		ivalid.SetInfinite();

		pblock->GetValue( mask_map1_on, t, mapOn[0], ivalid);
		pblock->GetValue( mask_map2_on, t, mapOn[1], ivalid);
		pblock->GetValue( mask_invert, t, invertMask, ivalid);

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

RefTargetHandle Mask::GetReference(int i) {
	if (i <2 )
		return subTex[i];
	else return pblock;
	}

void Mask::SetReference(int i, RefTargetHandle rtarg) {
	if (i < 2)
		subTex[i] = (Texmap *)rtarg; 
	else pblock = (IParamBlock2 *)rtarg; 
	}

void Mask::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i,m);
	if (i==0)
		{
		mask_param_blk.InvalidateUI(mask_map1);
		ivalid.SetEmpty();
		}	
	else if (i==1)
		{
		mask_param_blk.InvalidateUI(mask_map2);
		ivalid.SetEmpty();
		}	

	}

TSTR Mask::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_MAP)); 
		case 1:  return TSTR(GetString(IDS_DS_MASK)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Mask::SubAnim(int i) {
	if (i < 2)
		return subTex[i]; 
	else return pblock;
	}

TSTR Mask::SubAnimName(int i) {
	if (i< 2)
		return GetSubTexmapTVName(i);
	else return TSTR(_T(""));
	}

RefResult Mask::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			//if (paramDlg&&!paramDlg->isActive) 
				ParamID changing_param = pblock->LastNotifyParamID();
				mask_param_blk.InvalidateUI(changing_param);
				}

//			if (paramDlg) 
//					paramDlg->Invalidate();
			break;

		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define INVERT_MASK_CHUNK 0x2000
#define PARAM2_CHUNK 0x2010

IOResult Mask::Save(ISave *isave) { 
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


//watje
class MaskPostLoadCallback:public  PostLoadCallback
{
public:
	Mask      *s;
	int loadedChecks;
	MaskPostLoadCallback(Mask *r, BOOL b) {s=r;loadedChecks = b;}
	void proc(ILoad *iload);
};

void MaskPostLoadCallback::proc(ILoad *iload)
{
	if (loadedChecks)
		{
		s->pblock->SetValue( mask_map1_on, 0, s->mapOn[0]);
		s->pblock->SetValue( mask_map2_on, 0, s->mapOn[1]);
		s->pblock->SetValue( mask_invert, 0, s->invertMask);
		}
	delete this;
}
	  

IOResult Mask::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	int id;
//	BOOL loadedChecks = FALSE;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			case INVERT_MASK_CHUNK:
				invertMask = 1;
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

//	MaskPostLoadCallback* maskplcb = new MaskPostLoadCallback(this,loadedChecks);
//	iload->RegisterPostLoadCallback(maskplcb);

	return IO_OK;
	}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This map is not meaningful unless all of its submaps are on. 
//
bool Mask::IsLocalOutputMeaningful( ShadeContext& sc )
{
	for ( int i = 0; i < NumSubTexmaps(); i++ ) 
	{
		if ( SubTexmapOn( i ) && ( GetSubTexmap( i ) != NULL ) )
			return true;
	}
	
	return false;
}
