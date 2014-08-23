/**********************************************************************
 *<
	FILE: MARBLE.CPP

	DESCRIPTION: MARBLE 3D Texture map.

	CREATED BY: Dan Silva
				Updated to Param Block 2 12/1/98 Peter Watje
	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "macrorec.h"

extern HINSTANCE hInstance;

#define SHOW_3DMAPS_WITH_2D

#define NSUBTEX 2
#define NCOLS 2

static Class_ID marbleClassID(MARBLE_CLASS_ID,0);
//--------------------------------------------------------------
// Marble: A 3D texture map
//--------------------------------------------------------------

class Marble: public Tex3D { 
	friend class MarblePostLoad;
	static ParamDlg* xyzGenDlg;	
	Color col[NCOLS];
	XYZGen *xyzGen;		   // ref #0
	Texmap* subTex[NSUBTEX];  // More refs
	Interval ivalid;
	int rollScroll;
	int vers;
#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif
	float MarbleFunc(Point3 p);
	public:
		BOOL Param1;
		BOOL mapOn[NSUBTEX];
		float width,size;
		IParamBlock2 *pblock;   // ref #1
		Marble();
		~Marble() {
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}	
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		XYZGen *GetTheXYZGen() { return xyzGen; }

		void SetColor(int i, Color c, TimeValue t);
		void SetSize(float f, TimeValue t);
		void SetWidth(float f, TimeValue t);
		void NotifyChanged();
		void SwapInputs(); 

		void ReadSXPData(TCHAR *name, void *sxpdata);

	
		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

#ifdef SHOW_3DMAPS_WITH_2D
		void DiscardTexHandle() {
			if (texHandle) {
				texHandle->DeleteThis();
				texHandle = NULL;
				}
			}
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff) {
			if (!onoff) DiscardTexHandle();
			}
		DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
#endif SHOW_3DMAPS_WITH_2D

		// Requirements
		ULONG LocalRequirements(int subMtlNum) { return xyzGen->Requirements(subMtlNum); }
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
			}

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {	return marbleClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_MARBLE); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 2+NSUBTEX; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 2+NSUBTEX; }
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
		BOOL SetDlgThing(ParamDlg* dlg);

		// This is a good example where there's no way to find out
		// from the sc if the map is meaningful. 
		// For radiosity purposes it's not meaningful when the 
		// coord sys of XYZGen is UVW_COORDS or UVW2_COORDS
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

	};
ParamDlg* Marble::xyzGenDlg;	

class MarbleClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Marble; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_MARBLE_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return marbleClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_3D;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("marble"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static MarbleClassDesc marbleCD;

ClassDesc* GetMarbleDesc() { return &marbleCD;  }

//dialog stuff to get the Set Ref button
class MarbleDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
	    Marble *marble;		
		MarbleDlgProc(Marble *m) {marble = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			marble = (Marble*)m;
			}

	};


BOOL MarbleDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_MARBLE_SWAP:
					{
					marble->SwapInputs();
					}
				break;
				}
			break;
		}
	return FALSE;
	}



//-----------------------------------------------------------------------------
//  Marble
//-----------------------------------------------------------------------------


#define MARBLE_VERSION 1

enum { marble_params };  // pblock ID


// marble_params param IDs
enum 
{ 
	marble_map1, marble_map2,		
	marble_color1, marble_color2,
	marble_map1_on, marble_map2_on, 
	marble_size,marble_width, 
	marble_coords	  // access for XYZ mapping
};

static ParamBlockDesc2 marble_param_blk ( marble_params, _T("parameters"),  0, &marbleCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_MARBLE, IDS_DS_MARBPARMS, 0, 0, NULL, 
	// params
	marble_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MARB_TEX1,
		end,
	marble_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MARB_TEX2,
		end,
	marble_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0.2f,0.2f,0.1f), 
		p_ui,			TYPE_COLORSWATCH, IDC_MARB_COL1, 
		end,
	marble_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.82f,0.82f,0.6f), 
		p_ui,			TYPE_COLORSWATCH, IDC_MARB_COL2, 
		end,
	marble_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	marble_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	marble_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_MARBSIZE,
		p_default,		70.f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MARBSIZE_EDIT,IDC_MARBSIZE_SPIN,  1.0f, 
		end,
	marble_width,		_T("width"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_DS_MARBWIDTH,
		p_default,		0.025f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_WIDTH_EDIT,IDC_WIDTH_SPIN, 0.02f, 
		end,
	marble_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_COORDINATES,
		p_refno,		0, 
		end,

	end
);

static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE,marble_size }, 	// size
	{ TYPE_FLOAT, NULL, TRUE,marble_width }, 	// width
	{ TYPE_RGBA, NULL, TRUE,marble_color1 },  // col1
	{ TYPE_RGBA, NULL, TRUE,marble_color2 }   // col2
	};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,4,1),	// Version 1 params
	};

void Marble::Init() {
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference( 0, GetNewDefaultXYZGen());	
	ivalid.SetEmpty();
	macroRecorder->Disable();  // don't want to see this parameter reset in macrorecorder
		SetColor(0, Color(0.2f,0.2f,0.1f), TimeValue(0));
		SetColor(1, Color(0.82f,0.82f,0.6f), TimeValue(0));
		SetWidth(.025f, TimeValue(0));
		SetSize(70.0f, TimeValue(0));
	macroRecorder->Enable();  
	}

void Marble::Reset() {
	marbleCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

void Marble::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Marble::Marble() {
	subTex[0] = subTex[1] = NULL;
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	pblock = NULL;
	xyzGen = NULL;
	marbleCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	vers = 0;
	mapOn[0] = mapOn[1] = 1;
	}

RefTargetHandle Marble::Clone(RemapDir &remap) {
	Marble *mnew = new Marble();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(xyzGen));
	mnew->ReplaceReference(1,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->width = width;
	mnew->size = size;
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		mnew->mapOn[i] = mapOn[i];
		if (subTex[i])
			mnew->ReplaceReference(i+2,remap.CloneRef(subTex[i]));
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Marble::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif

ParamDlg* Marble::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = marbleCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	marble_param_blk.SetUserDlgProc(new MarbleDlgProc(this));
	return masterDlg;

	}


#define MARBLE_VERS 0xE001

struct Col24 {ULONG r,g,b; };

#pragma pack(1)
struct MarbleState {
	ulong version;
	float size;
	float width;
	Col24 col1,col2;
	};
#pragma pack()

static Color ColrFromCol24(Col24 a) {
	Color c;
	c.r = (float)a.r/255.0f;
	c.g = (float)a.g/255.0f;
	c.b = (float)a.b/255.0f;
	return c;
	}

void Marble::ReadSXPData(TCHAR *name, void *sxpdata) {
	MarbleState *state = (MarbleState*)sxpdata;
	if (state!=NULL && (state->version==MARBLE_VERS)) {
		SetColor(0, ColrFromCol24(state->col1),0);
		SetColor(1, ColrFromCol24(state->col2),0);
		SetWidth(state->width,0);
		SetSize(state->size,0);
		}
	}


void Marble::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( marble_map1_on, 0, mapOn[0]);
		pblock->SetValue( marble_map2_on, 0, mapOn[1]);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		xyzGen->Update(t,ivalid);
		pblock->GetValue( marble_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( marble_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( marble_width, t, width, ivalid );
		pblock->GetValue( marble_size, t, size, ivalid );
		pblock->GetValue( marble_map1_on, t, mapOn[0], ivalid);
		pblock->GetValue( marble_map2_on, t, mapOn[1], ivalid);

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

BOOL Marble::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((xyzGenDlg!= NULL) && (dlg == xyzGenDlg))
		xyzGenDlg->SetThing(xyzGen);
	else 
		return FALSE;
	return TRUE;
}



void Marble:: SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	pblock->SetValue( i==0?marble_color1:marble_color2, t, c);
	}

void Marble::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(marble_color1,0,marble_color2,0);
	marble_param_blk.InvalidateUI(marble_color1);
	marble_param_blk.InvalidateUI(marble_color2);
	marble_param_blk.InvalidateUI(marble_map1);
	marble_param_blk.InvalidateUI(marble_map2);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
	}

void Marble::SetWidth(float f, TimeValue t) { 
	width = f; 
	pblock->SetValue( marble_width, t, f);
	}

void Marble::SetSize(float f, TimeValue t) { 
	size = f; 
	pblock->SetValue( marble_size, t, f);
	}

RefTargetHandle Marble::GetReference(int i) {
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
		}
	}

void Marble::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
	}

void Marble::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2,m);

	if (i==0)
		{
		marble_param_blk.InvalidateUI(marble_map1);
		ivalid.SetEmpty();
		}	
	else if (i==1)
		{
		marble_param_blk.InvalidateUI(marble_map2);
		ivalid.SetEmpty();
		}	

	}

TSTR Marble::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Marble::SubAnim(int i) {
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
		}
	}

TSTR Marble::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_COORDINATES));		
		case 1: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-2);
		}
	}

static int nameID[] = {IDS_DS_MARBSIZE, IDS_DS_MARBWIDTH, IDS_DS_COLOR1, IDS_DS_COLOR2 };

RefResult Marble::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//				if (hTarget != xyzGen  && hTarget != pblock) 
					marble_param_blk.InvalidateUI(changing_param);
				// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
#ifdef SHOW_3DMAPS_WITH_2D
					if (changing_param != -1)
						DiscardTexHandle();
#endif
				}
			else if (hTarget == xyzGen) 
				{
#ifdef SHOW_3DMAPS_WITH_2D
				DiscardTexHandle();
#endif
				// NotifyChanged();  //DS this is redundant
				}
			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MARBLEVERS1_CHUNK 0x4001
#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult Marble::Save(ISave *isave) { 
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
	  
//extern void ScaleFloatController(IParamBlock *pblock, int index, float s);

class MarblePostLoad : public PostLoadCallback {
	public:
		Marble *chk;
		MarblePostLoad(Marble *b) {chk=b;}
		void proc(ILoad *iload) {
			if (chk->vers<1) {
				if (chk->pblock) 
//					ScaleFloatController(chk->pblock, PB_SIZE, 100.0f);
					chk->pblock->RescaleParam(marble_size, 0, 100.0f);
//				iload->SetObsolete();
				}
			delete this;
			}
	};
/*
class Marble2PostLoad : public PostLoadCallback {
	public:
		Marble *n;
		Marble2PostLoad(Marble *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->Param1)
				{
				n->pblock->SetValue( marble_map1_on, 0, n->mapOn[0]);
				n->pblock->SetValue( marble_map2_on, 0, n->mapOn[1]);
				}
			delete this; 


			} 
	};

*/

IOResult Marble::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	int id;
	vers = 0;
	Param1 = TRUE;
//	iload->RegisterPostLoadCallback(new MarblePostLoad(this));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MARBLEVERS1_CHUNK:
				vers = 1;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
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
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &marble_param_blk, this, 1);
	iload->RegisterPostLoadCallback(plcb);

//	iload->RegisterPostLoadCallback(new Marble2PostLoad(this));

	return IO_OK;
	}


#define WD .02f
#define FACT 500.0f
#define SZ 1.0f //50.0

float Marble::MarbleFunc(Point3 p) {
	int id;
	float d,i,r[3];
	r[0] = p.x/100.0f;	r[1] = p.y/200.0f;	r[2] = p.z/200.0f;
	d = (p.x+10000.0f)*width + 7*NOISE(r);
	id = ((int)d)%17;
	if (id<4) {
		r[0] = p.x/70.0f;	r[1] = p.y/50.0f;	r[2] = p.z/50.0f;
		i = 0.7f+0.2f*NOISE(r);
		}
	else {
		r[0] = p.x/100.0f;	r[1] = p.y/100.0f;	r[2] = p.x/100.0f;
		if (id<9 || id>=12) {
			d = (float)fabs(d- ((int)d/17.0f)*17.0f-10.5f)*0.1538462f;
			i = 0.4f + 0.3f*d + 0.2f*NOISE(r);
			}
		else 
			i = 0.2f*(1.0f + NOISE(r));
		}
	return(i);
	}

static AColor black(0.0f,0.0f,0.0f,0.0f);

RGBA Marble::EvalColor(ShadeContext& sc) {
	Point3 p,dp;
	if (!sc.doMaps) return black;
	if (gbufID) sc.SetGBufferID(gbufID);
	
	xyzGen->GetXYZ(sc,p,dp);
	if (size==0.0f) size=.0001f;
	p *= FACT/size;

	float d = MarbleFunc(p);

	if (d<=.0005) return  mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
	else if (d>=.9995) return  mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
	RGBA c0 = mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
	return (1.0f-d)*c0 + d*c1;
	}

Point3 Marble::EvalNormalPerturb(ShadeContext& sc) {
	float del,d;
	Point3 p,dp;
	if (!sc.doMaps) return Point3(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
	xyzGen->GetXYZ(sc,p,dp);
	if (size==0.0f) size=.0001f;
	p *= FACT/size;

	d = MarbleFunc(p);
	del = 20.0f;
	Point3 np;

	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
    np.x = (MarbleFunc(p+del*M[0]) - d)/del;
	np.y = (MarbleFunc(p+del*M[1]) - d)/del;
	np.z = (MarbleFunc(p+del*M[2]) - d)/del;

	np *= 100.0f;
	np = sc.VectorFromNoScale(np,REF_OBJECT);
	Texmap *sub0 = mapOn[0]?subTex[0]:NULL;
	Texmap *sub1 = mapOn[1]?subTex[1]:NULL;
	if (sub0||sub1) {
		// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
		float a,b;
		Point3 da,db;
		if (sub0) { 	a = sub0->EvalMono(sc); 	da = sub0->EvalNormalPerturb(sc);		}
		else {	 a = Intens(col[0]);	 da = Point3(0.0f,0.0f,0.0f);		 }
		if (sub1) { 	b = sub1->EvalMono(sc); 	db = sub1->EvalNormalPerturb(sc);	}
		else {	 b = Intens(col[1]);	 db= Point3(0.0f,0.0f,0.0f);		 }
		np = (b-a)*np + d*(db-da) + da;
		}
	else 
		np *= Intens(col[1])-Intens(col[0]);
	return np;
	}
