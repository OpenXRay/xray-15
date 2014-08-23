/**********************************************************************
 *<
	FILE: PARTAGE.CPP

	DESCRIPTION: A texture map that changes color based on particle ages.

	CREATED BY: Rolf Berteig
				Update 12/23 to param block2 by Peter Watje

	HISTORY: created 6/21/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "texutil.h"

#include "buildver.h"
#ifndef NO_PARTICLES // orb 07-11-01
#ifndef NO_MAPTYPE_PARTICLEAGE // orb 01-03-2001 Removing map types
			   
extern HINSTANCE hInstance;

#define PARTAGE_CLASSID	Class_ID(0x8d618ea4,0x49bbe8cf)


#define NSUBTEX	3

class PartAgeTex : public Texmap { 
	public:
		static ParamDlg* texoutDlg;
		IParamBlock2 *pblock;		// ref 0		
		TextureOutput *texout;		// ref 1
		Texmap* subTex[NSUBTEX];	// ref 2-4


		// Caches
		Interval ivalid;
		CRITICAL_SECTION csect;
		Color col1, col2, col3;
		int usemap1, usemap2, usemap3;
		float p1, p2, p3;		

		PartAgeTex();
		~PartAgeTex() {DeleteCriticalSection(&csect);}

		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);		
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) {Update(t,FOREVER); return ivalid;}		

		// Evaluation
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);		
		Point3 EvalNormalPerturb(ShadeContext& sc);		

		// Methods to access texture maps of material
		int NumSubTexmaps() {return NSUBTEX;}
		Texmap* GetSubTexmap(int i) {return subTex[i];}
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {return PARTAGE_CLASSID;}
		SClass_ID SuperClassID() {return TEXMAP_CLASS_ID;}
		void GetClassName(TSTR& s) { s = GetString(IDS_RB_PARTICLEAGE); } // mjm - 2.3.99
		void DeleteThis() {delete this;}	

		int NumSubs() {return 5;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;}

 		int NumRefs() {return 5;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

	bool IsLocalOutputMeaningful( ShadeContext& sc );
	};

ParamDlg* PartAgeTex::texoutDlg;


class PartAgeTexClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new PartAgeTex;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_PARTICLEAGE_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() {return TEXMAP_CLASS_ID;}
	Class_ID 		ClassID() {return PARTAGE_CLASSID;}
	const TCHAR* 	Category() {return TEXMAP_CAT_3D;}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("particleAge"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static PartAgeTexClassDesc partAgeCD;
ClassDesc* GetPartAgeDesc() {return &partAgeCD;}

enum { page_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	page_color1, page_color2,page_color3,
	page_map1, page_map2,	page_map3,		
	page_map1_on, page_map2_on, page_map3_on, 
	page_page1, page_page2, page_page3, 
	page_output,	  // access for UVW mapping
};

static ParamBlockDesc2 page_param_blk ( page_params, _T("parameters"),  0, &partAgeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PARTAGE_PARAMS, IDS_RB_PARTAGEPARAMS, 0, 0, NULL, 
	// params
	page_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_COLOR1,	
		p_default,		Color(0.0f,0.0f,0.0f), 
		p_ui,			TYPE_COLORSWATCH, IDC_PARTAGE_COLOR1, 
		end,
	page_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_COLOR2,	
		p_default,		Color(.5f,.5f,.5f), 
		p_ui,			TYPE_COLORSWATCH, IDC_PARTAGE_COLOR2, 
		end,
	page_color3,	 _T("color3"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_COLOR3,	
		p_default,		Color(1.0f,1.0f,1.0f), 
		p_ui,			TYPE_COLORSWATCH, IDC_PARTAGE_COLOR3, 
		end,
	page_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_PARTAGE_MAP1,
		end,
	page_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_PARTAGE_MAP2,
		end,
	page_map3,		_T("map3"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP3,
		p_refno,		4,
		p_subtexno,		2,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_PARTAGE_MAP3,
		end,
	page_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_PARTAGE_USEMAP1,
		end,
	page_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_PARTAGE_USEMAP2,
		end,
	page_map3_on,	_T("map3Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP3ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_PARTAGE_USEMAP3,
		end,

		
	page_page1,	_T("age1"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_AGE1,
		p_default,		0.f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARTAGE_AGE1, IDC_PARTAGE_AGE1SPIN, 0.1f, 
		end,
	page_page2,	_T("age2"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_AGE2,
		p_default,		50.f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARTAGE_AGE2, IDC_PARTAGE_AGE2SPIN, 0.1f, 
		end,
	page_page3,	_T("age3"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_AGE3,
		p_default,		100.f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PARTAGE_AGE3, IDC_PARTAGE_AGE3SPIN, 0.1f, 
		end,

	page_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_OUTPUT,
		p_refno,		1, 
		end,

	end
);


//--- Parameter Map/Parameter block IDs ------------------------------
/*
#define PB_COL1				0
#define PB_COL2				1
#define PB_COL3				2
#define PB_USEMAP1			3
#define PB_USEMAP2			4
#define PB_USEMAP3			5
#define PB_AGE1				6
#define PB_AGE2				7
#define PB_AGE3				8


static ParamUIDesc descParam[] = {
	// Color 1
	ParamUIDesc(PB_COL1,TYPE_COLORSWATCH,IDC_PARTAGE_COLOR1),

	// Color 2
	ParamUIDesc(PB_COL2,TYPE_COLORSWATCH,IDC_PARTAGE_COLOR2),

	// Color 3
	ParamUIDesc(PB_COL3,TYPE_COLORSWATCH,IDC_PARTAGE_COLOR3),

	// Use Map 1
	ParamUIDesc(PB_USEMAP1,TYPE_SINGLECHEKBOX,IDC_PARTAGE_USEMAP1),

	// Use Map 2
	ParamUIDesc(PB_USEMAP2,TYPE_SINGLECHEKBOX,IDC_PARTAGE_USEMAP2),

	// Use Map 3
	ParamUIDesc(PB_USEMAP3,TYPE_SINGLECHEKBOX,IDC_PARTAGE_USEMAP3),

	// Age 1
	ParamUIDesc(
		PB_AGE1,
		EDITTYPE_FLOAT,
		IDC_PARTAGE_AGE1,IDC_PARTAGE_AGE1SPIN,
		0.0f,100.0f,
		0.1f,stdPercentDim),

	// Age 2
	ParamUIDesc(
		PB_AGE2,
		EDITTYPE_FLOAT,
		IDC_PARTAGE_AGE2,IDC_PARTAGE_AGE2SPIN,
		0.0f,100.0f,
		0.1f,stdPercentDim),

	// Age 3
	ParamUIDesc(
		PB_AGE3,
		EDITTYPE_FLOAT,
		IDC_PARTAGE_AGE3,IDC_PARTAGE_AGE3SPIN,
		0.0f,100.0f,
		0.1f,stdPercentDim),
	};
*/
#define PARAMDESC_LENGH 9

static ParamBlockDescID descVer0[] = {
	{ TYPE_RGBA, NULL, TRUE, page_color1 }, // Color 1
	{ TYPE_RGBA, NULL, TRUE, page_color2 }, // Color 2
	{ TYPE_RGBA, NULL, TRUE, page_color3 }, // Color 3	
	{ TYPE_INT,  NULL, FALSE, page_map1_on },	// use map 1
	{ TYPE_INT,  NULL, FALSE, page_map2_on },	// use map 2
	{ TYPE_INT,  NULL, FALSE, page_map3_on },	// use map 3
	{ TYPE_FLOAT,  NULL, TRUE, page_page1 },	// Age 1
	{ TYPE_FLOAT,  NULL, TRUE, page_page2 },	// Age 2
	{ TYPE_FLOAT,  NULL, TRUE, page_page3 },	// Age 3
	};

#define PBLOCK_LENGTH	9

//static ParamVersionDesc versions[] = {
//	ParamVersionDesc(descVer0,12,0),	
//	ParamVersionDesc(descVer1,15,1),
//	};
//#define NUM_OLDVERSIONS	2

#define CURRENT_VERSION	0
//static ParamVersionDesc curVersion(descVer0,PBLOCK_LENGTH,CURRENT_VERSION);
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,9,0)
};
//--- PartAgeTex Methods -----------------------------------------------

PartAgeTex::PartAgeTex()
	{
	pblock   = NULL;	
	texout   = NULL;
	subTex[0] = subTex[1] = subTex[2] = NULL;
	partAgeCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	InitializeCriticalSection(&csect);
	ivalid.SetEmpty();
	}

ParamDlg* PartAgeTex::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
	IAutoMParamDlg* masterDlg = partAgeCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(texoutDlg);
	return masterDlg;
	}

void PartAgeTex::Update(TimeValue t, Interval& valid)
	{
	EnterCriticalSection(&csect);
	if (!ivalid.InInterval(t)) {
		ivalid = FOREVER;				
		texout->Update(t,ivalid);
/*
		pblock->GetValue(PB_COL1,t,col1,ivalid);
		pblock->GetValue(PB_COL2,t,col2,ivalid);
		pblock->GetValue(PB_COL3,t,col3,ivalid);
		pblock->GetValue(PB_USEMAP1,t,usemap1,ivalid);
		pblock->GetValue(PB_USEMAP2,t,usemap2,ivalid);
		pblock->GetValue(PB_USEMAP3,t,usemap3,ivalid);
		pblock->GetValue(PB_AGE1,t,p1,ivalid);
		pblock->GetValue(PB_AGE2,t,p2,ivalid);
		pblock->GetValue(PB_AGE3,t,p3,ivalid);
*/
		pblock->GetValue(page_color1,t,col1,ivalid);
		pblock->GetValue(page_color2,t,col2,ivalid);
		pblock->GetValue(page_color3,t,col3,ivalid);
		pblock->GetValue(page_map1_on,t,usemap1,ivalid);
		pblock->GetValue(page_map2_on,t,usemap2,ivalid);
		pblock->GetValue(page_map3_on,t,usemap3,ivalid);
		pblock->GetValue(page_page1,t,p1,ivalid);
		pblock->GetValue(page_page2,t,p2,ivalid);
		pblock->GetValue(page_page3,t,p3,ivalid);
		p1 = p1 *0.01f;
		p2 = p2 *0.01f;
		p3 = p3 *0.01f;
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}		
		}
	valid &= ivalid;
	LeaveCriticalSection(&csect);
	}

void PartAgeTex::Init()
	{
//	ReplaceReference(0, 
//		CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));	

/*
	pblock->SetValue(PB_COL1,0,Point3( 1 , 1 , 1 ));
	pblock->SetValue(PB_COL2,0,Point3(.5f,.5f,.5f));
	pblock->SetValue(PB_COL3,0,Point3( 0 , 0 , 0 ));
	pblock->SetValue(PB_USEMAP1,0,1);
	pblock->SetValue(PB_USEMAP2,0,1);
	pblock->SetValue(PB_USEMAP3,0,1);
	pblock->SetValue(PB_AGE1,0,0.0f);
	pblock->SetValue(PB_AGE2,0,0.5f);
	pblock->SetValue(PB_AGE3,0,1.0f);
*/
//	if (paramDlg) 
//		paramDlg->pmap->SetParamBlock(pblock);

	ivalid.SetEmpty();	

	if (texout) texout->Reset();
	else ReplaceReference(1, GetNewDefaultTextureOutput());
	}

void PartAgeTex::Reset()
	{
	partAgeCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}


BOOL PartAgeTex::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((texoutDlg!= NULL) && (dlg == texoutDlg))
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}



AColor PartAgeTex::EvalColor(ShadeContext& sc)
	{
	if (gbufID) sc.SetGBufferID(gbufID);
	Color tcol1 = col1;
	Color tcol2 = col2;
	Color tcol3 = col3;

	// Evaluate...
	Object *ob = sc.GetEvalObject();
	float u=0.0f;
	if (ob && ob->IsParticleSystem()) {
		ParticleObject *obj = (ParticleObject*)ob;
		TimeValue t = sc.CurTime();
		TimeValue age  = obj->ParticleAge(t,sc.mtlNum);
		TimeValue life = obj->ParticleLife(t,sc.mtlNum);
		if (age>=0 && life>=0) 
			u = float(age)/float(life);
		}

	if (u<p1) {
		if (usemap1 && subTex[0]) tcol1 = subTex[0]->EvalColor(sc);
		return texout->Filter(RGBA(tcol1));
	} else 
	if (u<p2) {
		u = (u-p1)/(p2-p1);
		if (usemap1 && subTex[0]) tcol1 = subTex[0]->EvalColor(sc);
		if (usemap2 && subTex[1]) tcol2 = subTex[1]->EvalColor(sc);
		return texout->Filter(RGBA(tcol2*u + (1.0f-u)*tcol1));	
	} else 
	if (u<p3) {
		u = (u-p2)/(p3-p2);
		if (usemap2 && subTex[1]) tcol2 = subTex[1]->EvalColor(sc);
		if (usemap3 && subTex[2]) tcol3 = subTex[2]->EvalColor(sc);
		return texout->Filter(RGBA(tcol3*u + (1.0f-u)*tcol2));		
	} else {
		if (usemap3 && subTex[2]) tcol3 = subTex[2]->EvalColor(sc);
		return texout->Filter(RGBA(tcol3));	
		}
	}

float PartAgeTex::EvalMono(ShadeContext& sc)
	{
	return Intens(EvalColor(sc));
	}

Point3 PartAgeTex::EvalNormalPerturb(ShadeContext& sc)
	{	
	Point3 np(0,0,0);	
	return texout->Filter(sc.VectorFrom(np,REF_OBJECT));
	}

void PartAgeTex::SetSubTexmap(int i, Texmap *m)
	{
	ReplaceReference(i+2,m);
//	if (paramDlg) paramDlg->UpdateSubTexNames();
	if (i==0)
		{
		page_param_blk.InvalidateUI(page_map1);
		ivalid.SetEmpty();
		}	
	else if (i==1)
		{
		page_param_blk.InvalidateUI(page_map2);
		ivalid.SetEmpty();
		}	
	else if (i==2)
		{
		page_param_blk.InvalidateUI(page_map3);
		ivalid.SetEmpty();
		}	

	}

TSTR PartAgeTex::GetSubTexmapSlotName(int i)
	{
	switch (i) {
		case 0:  return GetString(IDS_RB_COLOR1);
		case 1:  return GetString(IDS_RB_COLOR2);
		case 2:  return GetString(IDS_RB_COLOR3);
		default: return _T("");
		}
	}

Animatable* PartAgeTex::SubAnim(int i)
	{
	return GetReference(i);
	}

TSTR PartAgeTex::SubAnimName(int i)
	{
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));		
		case 1: return TSTR(GetString(IDS_DS_OUTPUT));		
		default: return GetSubTexmapTVName(i-2);
		}
	assert(0); 
	return _T("");

	}

RefTargetHandle PartAgeTex::GetReference(int i)
	{
	switch (i) {
		case 0:  return pblock;		
		case 1:  return texout;
		default: return subTex[i-2];
		}
	}

void PartAgeTex::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0:  pblock = (IParamBlock2*)rtarg; break;		
		case 1:  texout = (TextureOutput *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
	}

#define MTL_HDR_CHUNK 	0x4000


#define PARAM2_CHUNK 0x1010

IOResult PartAgeTex::Save(ISave *isave) { 
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
class PartAgePostLoadCallback:public  PostLoadCallback
{
public:
	PartAgeTex      *s;
	int Param1;
	PartAgePostLoadCallback(PartAgeTex *r, BOOL b) {s=r;Param1 = b;}
	void proc(ILoad *iload);
};

void PartAgePostLoadCallback::proc(ILoad *iload)
{
	if (Param1)
		{

		Interval ivalid;
		TimeValue t = 0;
		s->pblock->GetValue(page_page1,t,s->p1,ivalid);
		s->pblock->GetValue(page_page2,t,s->p2,ivalid);
		s->pblock->GetValue(page_page3,t,s->p3,ivalid);


		s->p1 *= 100.0f;
		s->p2 *= 100.0f;
		s->p3 *= 100.0f;

		s->pblock->SetValue(page_page1,t,s->p1);
		s->pblock->SetValue(page_page2,t,s->p2);
		s->pblock->SetValue(page_page3,t,s->p3);

		}
	delete this;
}


IOResult PartAgeTex::Load(ILoad *iload)
	{
#if 0
//	iload->RegisterPostLoadCallback(
//		new ParamBlockPLCB(
//			versions, 
//			NUM_OLDVERSIONS, 
//			&curVersion, this, 0/*ref # */));
#endif	

	IOResult res;
	int id;
	BOOL Param1 = TRUE;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &page_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

	PartAgePostLoadCallback* partAgeplcb = new PartAgePostLoadCallback(this,Param1);
	iload->RegisterPostLoadCallback(partAgeplcb);


	return IO_OK;
	}

RefTargetHandle PartAgeTex::Clone(RemapDir &remap)
	{
	PartAgeTex *map = new PartAgeTex;
	*((MtlBase*)map) = *((MtlBase*)this);  // copy superclass stuff
	map->ReplaceReference(0,remap.CloneRef(pblock));		
	map->ReplaceReference(1,remap.CloneRef(texout));
	for (int i=0; i<NSUBTEX; i++) {
		if (subTex[i]) map->ReplaceReference(2+i,remap.CloneRef(subTex[i]));
		}
	BaseClone(this, map, remap);
	return map;
	}

RefResult PartAgeTex::NotifyRefChanged(
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
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//				if (hTarget != texout ) 
					page_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}
			else if (hTarget == texout)
				{
			// notify our dependents that we've changed
				// NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);  
				}

			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_COL1:
				case PB_COL2:
				case PB_COL3: gpd->dim = stdColor255Dim; break;
				case PB_AGE1:
				case PB_AGE2:
				case PB_AGE3: gpd->dim = stdPercentDim; break;
				default:      gpd->dim = defaultDim; break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {				
				case PB_COL1:  gpn->name = GetString(IDS_RB_COLOR1); break;
				case PB_COL2:  gpn->name = GetString(IDS_RB_COLOR2); break;
				case PB_COL3:  gpn->name = GetString(IDS_RB_COLOR3); break;
				case PB_AGE1:  gpn->name = GetString(IDS_RB_AGE1); break;
				case PB_AGE2:  gpn->name = GetString(IDS_RB_AGE2); break;
				case PB_AGE3:  gpn->name = GetString(IDS_RB_AGE3); break;
				default: gpn->name = _T("Parameter");  break;
				}
			return REF_STOP; 
			}
*/
		}
	return REF_SUCCEED;
	}


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// It is meaningful if used with a particle system or at least one map 
// exists and it is on
//
bool PartAgeTex::IsLocalOutputMeaningful( ShadeContext& sc )
{
	Object *ob = sc.GetEvalObject();
	if (ob && ob->IsParticleSystem()) 
		return true;

	for ( int i = 0; i < NumSubTexmaps(); i++ ) 
	{
		if ( SubTexmapOn( i ) && ( GetSubTexmap( i ) != NULL ) )
			return true;
	}
	
	return false;

}

#endif // NO_MAPTYPE_PARTICLEAGE
#endif // NO_PARTICLES
