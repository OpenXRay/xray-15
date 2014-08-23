/**********************************************************************
 *<
	FILE: FALLOFF.CPP

	DESCRIPTION: FALLOFF 3D Texture map.

	CREATED BY: Dan Silva

	HISTORY: 12/2/98 Updated to Param Block 2 Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "macrorec.h"

extern HINSTANCE hInstance;

static Class_ID falloffClassID(FALLOFF_CLASS_ID,0);

#define PBLOCK_REF 0

#define FALLTYPE_VIEW 0
#define FALLTYPE_OBJ  1
#define FALLTYPE_XLOC 2
#define FALLTYPE_YLOC 3
#define FALLTYPE_ZLOC 4
#define FALLTYPE_XWOR 5
#define FALLTYPE_YWOR 6
#define FALLTYPE_ZWOR 7

//--------------------------------------------------------------
// Falloff: A 3D texture map
//--------------------------------------------------------------

class Falloff: public Texmap { 
	float nearVal;
	float farVal;
	Interval ivalid;
	int rollScroll;
	Point3 nodePos;
	BOOL gotPos;
	BOOL inRender;
	CRITICAL_SECTION csect;
	public:

		BOOL Param1;

		TSTR nodeName;
		INode *vNode;
		int fallDir;
		BOOL frontBack; // vs parallel/perpendicular
		IParamBlock2 *pblock;   // ref #1
		Falloff();
		~Falloff() {DeleteCriticalSection(&csect);}
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void NotifyChanged();
		void EnableStuff();

		void SetNear(float f, TimeValue t); 
		void SetFar(float f, TimeValue t); 
		void SwapValues();
			
		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);
	    float EvalMono(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		int RenderBegin(TimeValue t, ULONG flags); 
		int RenderEnd(TimeValue t) { inRender = FALSE;  return 1; }
		int LoadMapFiles(TimeValue t) { gotPos = FALSE; return 1; }

		// Requirements
		ULONG LocalRequirements(int subMtlNum) { return 0; }

		// Methods to access texture maps of material
		int NumSubTexmaps() { return 0; }
		Texmap* GetSubTexmap(int i) { assert(0); return NULL; }
		void SetSubTexmap(int i, Texmap *m) {}
		TSTR GetSubTexmapSlotName(int i) { return TSTR(""); }

		Class_ID ClassID() {	return falloffClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_FALLOFF); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 1; }
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

class FalloffClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return FALSE; }
	void *			Create(BOOL loading) { 	return new Falloff; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_FALLOFF_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return falloffClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_3D;  }
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("falloff"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static FalloffClassDesc falloffCD;

ClassDesc* GetFalloffDesc() { return &falloffCD;  }


// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { falloff_params, };  // pblock ID
// falloff_params param IDs
enum 
{ 
	falloff_perp, falloff_parallel,
	falloff_type, falloff_dir,
	falloff_node
};
static ParamBlockDesc2 falloff_param_blk ( falloff_params, _T("parameters"),  0, &falloffCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_FALLOFF, IDS_DS_FALLOFFPARAMS, 0, 0, NULL, 
	// params
	falloff_perp,	_T("perpendicularValue"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_PERVAL,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_FALL_NEAR, IDC_FALL_NEAR_SPIN, 0.01f,
		end,
	falloff_parallel,	_T("parallelValue"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_PARVAL,
		p_default,		1.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_FALL_FAR, IDC_FALL_FAR_SPIN, 0.01f,
		end,

	falloff_type, _T("type"), TYPE_INT,				0,				IDS_JW_FALLOFFTYPE,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_FALL_PP, IDC_FALL_FB,
		end,
	falloff_dir, _T("direction"), TYPE_INT,				0,				IDS_PW_DIRECTION,
		p_default,		0,
		p_range,		0,	7,
		p_ui,			TYPE_RADIO, 8, IDC_FALLDIR_VIEW, IDC_FALLDIR_OBJ,
									   IDC_FALLDIR_X, IDC_FALLDIR_Y, IDC_FALLDIR_Z,
									   IDC_FALLDIR_XW, IDC_FALLDIR_YW,IDC_FALLDIR_ZW,
		end,
	falloff_node, 		_T("node"), 		TYPE_INODE, 	0,		IDS_PW_NODE,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_FALLOFF_PICK, 
		end, 

	end
);


//dialog stuff to get the Set Ref button
class FalloffDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Falloff *falloff;		
		FalloffDlgProc(Falloff *m) {falloff = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};



BOOL FalloffDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{

	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_FALL_SWAP:
					{
					falloff = (Falloff*)map->GetParamBlock()->GetOwner(); 
					falloff->SwapValues();
					}
				}
			break;
		}
	return FALSE;
	}


//-----------------------------------------------------------------------------
//  Falloff
//-----------------------------------------------------------------------------

#define FALLOFF_VERSION 1


static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE,falloff_perp }, 	// nearVal
	{ TYPE_FLOAT, NULL, TRUE,falloff_parallel } 	// farVal
	};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,2,1),	// Version 1 params
	};


void Falloff::Init() {
	ivalid.SetEmpty();
	SetNear(0.0f, TimeValue(0));
	SetFar(1.0f, TimeValue(0));
	fallDir = FALLTYPE_VIEW;
	frontBack = 0;
	}

void Falloff::Reset() {
	falloffCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}

void Falloff::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Falloff::Falloff() {
	Param1 = FALSE;
	InitializeCriticalSection(&csect);
	pblock = NULL;
	vNode = NULL;
	inRender = FALSE;
	gotPos = FALSE;
	falloffCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

void Falloff::EnableStuff() {
	if (pblock) {
		IParamMap2 *map = pblock->GetMap();
		pblock->GetValue( falloff_type, 0, frontBack, FOREVER );
		if (map) {
			HWND hw = map->GetHWnd();
			if (frontBack) {
				SetDlgItemText(hw, IDC_FALL_NEARNAME, GetString(IDS_DS_BVAL));
				SetDlgItemText(hw, IDC_FALL_FARNAME, GetString(IDS_DS_FVAL));
				}
			else {
				SetDlgItemText(hw, IDC_FALL_NEARNAME, GetString(IDS_DS_PERVAL));
				SetDlgItemText(hw, IDC_FALL_FARNAME, GetString(IDS_DS_PARVAL));
				}
			}
		}
	}

RefTargetHandle Falloff::Clone(RemapDir &remap) {
	Falloff *mnew = new Falloff();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->nearVal = nearVal;
	mnew->farVal = farVal;
	mnew->fallDir = fallDir;
	mnew->frontBack = frontBack;
	mnew->ivalid.SetEmpty();	
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Falloff::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	IAutoMParamDlg* masterDlg = falloffCD.CreateParamDlgs(hwMtlEdit, imp, this);
//attach a dlg proc to handle the swap button 
	falloff_param_blk.SetUserDlgProc(new FalloffDlgProc(this));
	EnableStuff();
	return masterDlg;

	}

void Falloff::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( falloff_dir, 0, fallDir);
		pblock->SetValue( falloff_type, 0, frontBack);

		Interface *iface = GetCOREInterface();
		vNode = iface->GetINodeByName(nodeName);
		if (vNode != NULL)
			pblock->SetValue( falloff_node, 0, vNode);
		Param1 = FALSE;
		}


	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( falloff_perp, t, nearVal, ivalid );
		pblock->GetValue( falloff_parallel, t, farVal, ivalid );
		pblock->GetValue( falloff_type, t, frontBack, ivalid );
		pblock->GetValue( falloff_dir, t, fallDir, ivalid );
		pblock->GetValue( falloff_node, t, vNode, ivalid );

		EnableStuff();

		}
	valid &= ivalid;
	}


int Falloff::RenderBegin(TimeValue t, ULONG flags)  {
	inRender = TRUE;
	if (fallDir == FALLTYPE_OBJ&&nodeName.Length()) {
		Interface *iface = GetCOREInterface();
		vNode = iface->GetINodeByName(nodeName);
		}
	return 1;
	}

void Falloff::SetNear(float f, TimeValue t) { 
	nearVal = f; 
	pblock->SetValue( falloff_perp, t, f);
	}

void Falloff::SetFar(float f, TimeValue t) { 
	farVal = f; 
	pblock->SetValue( falloff_parallel, t, f);
	}

void Falloff::SwapValues() {
	pblock->SwapControllers(falloff_perp,0,falloff_parallel,0);
	falloff_param_blk.InvalidateUI(falloff_perp);
	falloff_param_blk.InvalidateUI(falloff_parallel);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("perpendicularValue"), mr_reftarg, this, mr_prop, _T("parallelValue"), mr_reftarg, this);
	}

RefTargetHandle Falloff::GetReference(int i) {
	switch(i) {
		case 0:	return pblock ;
		default: assert(0); return NULL; 
		}
	}

void Falloff::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2 *)rtarg; break;
		default: assert(0); break;
		}
	}
	 
Animatable* Falloff::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: assert(0); return NULL;
		}
	}

TSTR Falloff::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: assert(0); return TSTR(_T("null"));
		}
	}

static int ta_nameID[] = {IDS_DS_BVAL, IDS_DS_FVAL };
static int pp_nameID[] = {IDS_DS_PERVAL, IDS_DS_PARVAL };

RefResult Falloff::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				falloff_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}
			else 
				{
				// NotifyChanged();  //DS this is redundant
				}

			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define FALLOFF_TYPE_CHUNK 0x2000
#define NODE_NAME_CHUNK 0x2010
#define FRONTBACK_CHUNK 0x2020
#define PARAM2_CHUNK 0x2030

IOResult Falloff::Save(ISave *isave) { 
	IOResult res;
//	ULONG nb;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	isave->EndChunk();
	if (res!=IO_OK) return res;

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();


	return IO_OK;
	}	
	  
class FalloffPostLoad : public PostLoadCallback {
	public:
		Falloff *n;
		BOOL Param1;
		FalloffPostLoad(Falloff *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( falloff_dir, 0, n->fallDir);
				n->pblock->SetValue( falloff_type, 0, n->frontBack);

				Interface *iface = GetCOREInterface();
				n->vNode = iface->GetINodeByName(n->nodeName);
				if (n->vNode != NULL)
					n->pblock->SetValue( falloff_node, 0, n->vNode);

				}

			delete this; 


			} 
	};

IOResult Falloff::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	int id;
	Param1 = TRUE;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case FALLOFF_TYPE_CHUNK:
				res = iload->Read(&fallDir,sizeof(fallDir), &nb);
				break;
			case FRONTBACK_CHUNK:
				frontBack = 1;
				break;
			case NODE_NAME_CHUNK:
				{
				TCHAR *buf;
				if (IO_OK==iload->ReadWStringChunk(&buf)) 
					nodeName = buf;
				break;
				}
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &falloff_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

//	iload->RegisterPostLoadCallback(new FalloffPostLoad(this,Param1));

	return IO_OK;
	}

static RGBA black(0.0f,0.0f,0.0f,1.0f);
static RGBA white(1.0f,1.0f,1.0f,1.0f);

RGBA Falloff::EvalColor(ShadeContext& sc) {
	RGBA c;
	c.r = EvalMono(sc);
	c.g = c.b = c.r;
	c.a = 1.0f;
	return c;
	}

float Falloff::EvalMono(ShadeContext& sc) {
	float d;
	Point3 n;
	if (!sc.doMaps) return 0.0f;
	if (gbufID) sc.SetGBufferID(gbufID);
	switch (fallDir) {
		case FALLTYPE_VIEW:
			d = -DotProd(sc.Normal(), sc.V());
			break;
		case FALLTYPE_OBJ:
			if (sc.InMtlEditor()) {
				d = sc.Normal().x;
				}
			else {
				if (!gotPos) {
					EnterCriticalSection(&csect);
					if (!gotPos) {
						if (vNode) {
							Matrix3 tm= vNode->GetNodeTM(sc.CurTime());
							nodePos = sc.PointFrom(tm.GetTrans(),REF_WORLD);
							}		
						gotPos = TRUE;
						}
					LeaveCriticalSection(&csect);
					}
				Point3 v = FNormalize(nodePos - sc.P());
				d = DotProd(sc.Normal(), v);
				}
			break;
		case FALLTYPE_XLOC:
		   n = sc.VectorTo(sc.Normal(), REF_OBJECT);
		   d = n.x;
		   break;
		case FALLTYPE_YLOC:
		   n = sc.VectorTo(sc.Normal(), REF_OBJECT);
		   d = n.y;
		   break;
		case FALLTYPE_ZLOC:
		   n = sc.VectorTo(sc.Normal(), REF_OBJECT);
		   d = n.z;
		   break;
		case FALLTYPE_XWOR:
		   n = sc.VectorTo(sc.Normal(), REF_WORLD);
		   d = n.x;
		   break;
		case FALLTYPE_YWOR:
		   n = sc.VectorTo(sc.Normal(), REF_WORLD);
		   d = n.y;
		   break;
		case FALLTYPE_ZWOR:
		   n = sc.VectorTo(sc.Normal(), REF_WORLD);
		   d = n.z;
		   break;
		}
	d = frontBack? 0.5f*(d+1.0f) : (float)fabs(d);
	return d*farVal + (1.0f-d)*nearVal;
	}	

Point3 Falloff::EvalNormalPerturb(ShadeContext& sc) {
	return Point3(0,0,0);
	}
