/**********************************************************************
 *<
	FILE: optmod.cpp

	DESCRIPTION:  Optimize modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/20/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_OPTIMIZE // JP Morel - June 28th 2002

#include "resourceOverride.h"
#include "iparamm.h"


class OptMod : public Modifier, public MeshOpProgress {	
	public:
		IParamBlock *pblock;
		static IParamMap *pmapParam;
		BOOL forceUpdate;

		OptMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_OPTMOD); }  
		virtual Class_ID ClassID() { return Class_ID(OPTIMIZEOSM_CLASS_ID,0);}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return GetString(IDS_RB_OPTIMIZE); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		int RenderBegin(TimeValue t, ULONG flags);		
		int RenderEnd(TimeValue t);
		IOResult Load(ILoad *iload);

		ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
		ChannelMask ChannelsChanged() {return OBJ_CHANNELS;}
		Class_ID InputType() {return triObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}

		int NumSubs() {return 1;}
		Animatable* SubAnim(int i) {return pblock;}
		TSTR SubAnimName(int i) {return GetString(IDS_RB_PARAMETERS);}

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		// From MeshOpProgress
		void Init(int total);
		BOOL Progress(int p);
	};



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *OptMod::pmapParam = NULL;



class OptClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new OptMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_OPTIMIZE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(OPTIMIZEOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static OptClassDesc optDesc;
extern ClassDesc* GetOptModDesc() { return &optDesc; }


//--- Parameter map/block descriptors -------------------------------

#define A_RENDER			A_PLUGIN1

#define PB_RENDER			0
#define PB_VIEWS			1

#define PB_FACETHRESH1		2
#define PB_EDGETHRESH1		3
#define PB_BIAS1			4
#define PB_PRESERVEMAT1		5
#define PB_PRESERVESMOOTH1	6
#define PB_MAXEDGE1			7

#define PB_FACETHRESH2		8
#define PB_EDGETHRESH2		9
#define PB_BIAS2			10
#define PB_PRESERVEMAT2		11
#define PB_PRESERVESMOOTH2	12
#define PB_MAXEDGE2			13

#define PB_AUTOEDGE			14
#define PB_MANUPDATE		15

// Map indices
#define MAP_FACETHRESH		2
#define MAP_EDGETHRESH		3
#define MAP_BIAS			4
#define MAP_PRESERVEMAT		5
#define MAP_PRESERVESMOOTH	6
#define MAP_MAXEDGE			7

//
//
// Parameters

static int renderIDs[] = {IDC_OPT_RENDERL1,IDC_OPT_RENDERL2};
static int viewsIDs[] = {IDC_OPT_VIEWSL1,IDC_OPT_VIEWSL2};

static ParamUIDesc descParam[] = {
	// Renderer
	ParamUIDesc(PB_RENDER,TYPE_RADIO,renderIDs,2),
	
	// Viewports
	ParamUIDesc(PB_VIEWS,TYPE_RADIO,viewsIDs,2),

	// Face thresh
	ParamUIDesc(
		PB_FACETHRESH1,
		EDITTYPE_FLOAT,
		IDC_OPT_FACETHRESH,IDC_OPT_FACETHRESHSPIN,
		0.0f,90.0f,
		0.1f,
		stdAngleDim),

	// Edge thresh
	ParamUIDesc(
		PB_EDGETHRESH1,
		EDITTYPE_FLOAT,
		IDC_OPT_EDGETHRESH,IDC_OPT_EDGETHRESHSPIN,
		0.0f,90.0f,
		0.1f,
		stdAngleDim),

	// Bias
	ParamUIDesc(
		PB_BIAS1,
		EDITTYPE_FLOAT,
		IDC_OPT_BIAS,IDC_OPT_BIASSPIN,
		0.0f,1.0f,
		0.01f),	

	// Preserve mat boundries
	ParamUIDesc(PB_PRESERVEMAT1,TYPE_SINGLECHEKBOX,IDC_OPT_PRESERVEMAT),	

	// Preserve smooth boundries
	ParamUIDesc(PB_PRESERVESMOOTH1,TYPE_SINGLECHEKBOX,IDC_OPT_PRESERVESMOOTH),

	// Max Edge
	ParamUIDesc(
		PB_MAXEDGE1,
		EDITTYPE_FLOAT,
		IDC_OPT_MAXEDGE,IDC_OPT_MAXEDGESPIN,
		0.0f,BIGFLOAT,
		SPIN_AUTOSCALE),

	// Auto edge
	ParamUIDesc(PB_AUTOEDGE,TYPE_SINGLECHEKBOX,IDC_OPT_AUTOEDGE),

	// Manual update
	ParamUIDesc(PB_MANUPDATE,TYPE_SINGLECHEKBOX,IDC_OPT_MANUPDATE),	
	};
#define PARAMDESC_LENGH 10


static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },

	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, FALSE, 10 },
	{ TYPE_INT, NULL, FALSE, 11 },
	
	{ TYPE_INT, NULL, FALSE, 12 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 13 }, // max edge1

	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, FALSE, 10 },
	{ TYPE_INT, NULL, FALSE, 11 },
	{ TYPE_FLOAT, NULL, TRUE, 14 },	// max edge2

	{ TYPE_INT, NULL, FALSE, 12 },	
	};

static ParamBlockDescID descVer2[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 13 }, // max edge1

	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_FLOAT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, FALSE, 10 },
	{ TYPE_INT, NULL, FALSE, 11 },
	{ TYPE_FLOAT, NULL, TRUE, 14 },	// max edge2

	{ TYPE_INT, NULL, FALSE, 12 },	
	{ TYPE_INT, NULL, FALSE, 15 },
	};

#define PBLOCK_LENGTH	16


// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,13,0),
	ParamVersionDesc(descVer1,15,0)
	};
#define NUM_OLDVERSIONS	2

// Current version
#define CURRENT_VERSION	2
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);



//--- OptDlgProc -------------------------------

class OptDlgProc : public ParamMapUserDlgProc {
	public:
		Interface *ip;
		OptMod *mod;
		OptDlgProc() {ip=NULL;mod=NULL;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void SetupLevels(IParamMap *map);
		void SetupManButton(HWND hWnd,IParamMap *map);
		void DeleteThis() {}
	};
static OptDlgProc theOptProc;

void OptDlgProc::SetupLevels(IParamMap *map)
	{
	int l;
	map->GetParamBlock()->GetValue(PB_VIEWS,0,l,FOREVER);
	if (l==0) {
		map->SetPBlockIndex(MAP_FACETHRESH,PB_FACETHRESH1);
		map->SetPBlockIndex(MAP_EDGETHRESH,PB_EDGETHRESH1);
		map->SetPBlockIndex(MAP_BIAS,PB_BIAS1);
		map->SetPBlockIndex(MAP_PRESERVEMAT,PB_PRESERVEMAT1);
		map->SetPBlockIndex(MAP_PRESERVESMOOTH,PB_PRESERVESMOOTH1);
		map->SetPBlockIndex(MAP_MAXEDGE,PB_MAXEDGE1);
	} else {
		map->SetPBlockIndex(MAP_FACETHRESH,PB_FACETHRESH2);
		map->SetPBlockIndex(MAP_EDGETHRESH,PB_EDGETHRESH2);
		map->SetPBlockIndex(MAP_BIAS,PB_BIAS2);
		map->SetPBlockIndex(MAP_PRESERVEMAT,PB_PRESERVEMAT2);
		map->SetPBlockIndex(MAP_PRESERVESMOOTH,PB_PRESERVESMOOTH2);
		map->SetPBlockIndex(MAP_MAXEDGE,PB_MAXEDGE2);
		}
	}

void OptDlgProc::SetupManButton(HWND hWnd,IParamMap *map)
	{
	int man;
	map->GetParamBlock()->GetValue(PB_MANUPDATE,0,man,FOREVER);
	EnableWindow(GetDlgItem(hWnd,IDC_OPT_UPDATE),man);
	}

BOOL OptDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			SetupLevels(map);
			SetWindowText(GetDlgItem(hWnd,IDC_OPT_VERTCOUNT),NULL);		
			SetWindowText(GetDlgItem(hWnd,IDC_OPT_FACECOUNT),NULL);
			SetupManButton(hWnd,map);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OPT_VIEWSL1: 
				case IDC_OPT_VIEWSL2:
					SetupLevels(map);
					break;

				case IDC_OPT_MANUPDATE:					
					SetupManButton(hWnd,map);
					break;

				case IDC_OPT_UPDATE:
					mod->forceUpdate = TRUE;
					mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					ip->RedrawViews(ip->GetTime());
					break;
				}
			break;
		}
	return FALSE;
	}


//--- Opt methods -------------------------------


OptMod::OptMod()
	{	
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer2, PBLOCK_LENGTH, CURRENT_VERSION));	
	pblock->SetValue(PB_FACETHRESH1,0,DegToRad(4.0f));
	pblock->SetValue(PB_EDGETHRESH1,0,DegToRad(1.0f));
	pblock->SetValue(PB_BIAS1,0,0.1f);
	pblock->SetValue(PB_FACETHRESH2,0,DegToRad(4.0f));
	pblock->SetValue(PB_EDGETHRESH2,0,DegToRad(1.0f));
	pblock->SetValue(PB_BIAS2,0,0.1f);
	pblock->SetValue(PB_MANUPDATE,0,0);
	forceUpdate = FALSE;
	}

IOResult OptMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

void OptMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
	{
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_OPTIMIZEPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	theOptProc.ip  = ip;
	theOptProc.mod = this;
	pmapParam->SetUserDlgProc(&theOptProc);
	}
		
void OptMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{	
	DestroyCPParamMap(pmapParam);
	pmapParam = NULL;
	theOptProc.ip  = NULL;
	theOptProc.mod = NULL;
	}

Interval OptMod::LocalValidity(TimeValue t)
	{
	float f;		
	Interval valid = FOREVER;
	int man;
	pblock->GetValue(PB_MANUPDATE,t,man,valid);
	if (!man) {
		pblock->GetValue(PB_FACETHRESH1,t,f,valid);
		pblock->GetValue(PB_EDGETHRESH1,t,f,valid);	
		pblock->GetValue(PB_BIAS1,t,f,valid);	
		pblock->GetValue(PB_MAXEDGE1,t,f,valid);
			
		pblock->GetValue(PB_FACETHRESH2,t,f,valid);
		pblock->GetValue(PB_EDGETHRESH2,t,f,valid);	
		pblock->GetValue(PB_BIAS2,t,f,valid);	
		pblock->GetValue(PB_MAXEDGE2,t,f,valid);
		}
	return valid;
	}

RefTargetHandle OptMod::Clone(RemapDir& remap) 
	{
	OptMod* newmod = new OptMod();	
	newmod->ReplaceReference(0,pblock->Clone(remap));	
	BaseClone(this, newmod, remap);
	return newmod;
	}

int OptMod::RenderBegin(TimeValue t, ULONG flags)
	{
	int views, render, man;
	pblock->GetValue(PB_VIEWS,0,views,FOREVER);
#ifndef NO_OUTPUTRENDERER
	pblock->GetValue(PB_RENDER,0,render,FOREVER);	
#else
	render = views;
#endif
	pblock->GetValue(PB_MANUPDATE,0,man,FOREVER);
	SetAFlag(A_RENDER);
	if (views!=render || man) {
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
	return 0;
	}

int OptMod::RenderEnd(TimeValue t)
	{
	int views, render;
	pblock->GetValue(PB_VIEWS,0,views,FOREVER);
#ifndef NO_OUTPUTRENDERER
	pblock->GetValue(PB_RENDER,0,render,FOREVER);	
#else
	render = views;
#endif
	ClearAFlag(A_RENDER);
	if (views!=render) {
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
	return 0;
	}

void EscapeChecker::Setup () {
	GetAsyncKeyState (VK_ESCAPE);	// Clear any previous presses.
	mMaxProcessID = GetCurrentProcessId();
}

bool EscapeChecker::Check () {
	if (!GetAsyncKeyState (VK_ESCAPE)) return false;
	DWORD processID=0;
	HWND hForeground = GetForegroundWindow ();
	if (hForeground) GetWindowThreadProcessId (hForeground, &processID);
	return (processID == mMaxProcessID) ? true : false;
}

static EscapeChecker theEscapeChecker;

void OptMod::Init(int total) {
	theEscapeChecker.Setup ();
}

BOOL OptMod::Progress(int p) {
	if (theEscapeChecker.Check()) return FALSE;
	else return TRUE;
}

void OptMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	float faceThresh, edgeThresh, bias, maxEdge;
	int preserveMat, preserveSmooth, which, render=0, autoEdge;
	DWORD flags = 0;
	Interval valid = FOREVER;
	int nv,nf;

	int man;
	pblock->GetValue(PB_MANUPDATE,t,man,valid);
	if (man && !forceUpdate && !TestAFlag(A_RENDER)) return;
	forceUpdate = FALSE;

	if (TestAFlag(A_RENDER)) {
		pblock->GetValue(PB_RENDER,t,which,valid);
	} else {
		pblock->GetValue(PB_VIEWS,t,which,valid);
		}	
	
	pblock->GetValue(PB_AUTOEDGE,t,autoEdge,valid);

	if (which==0) {
		pblock->GetValue(PB_FACETHRESH1,t,faceThresh,valid);
		pblock->GetValue(PB_EDGETHRESH1,t,edgeThresh,valid);	
		pblock->GetValue(PB_BIAS1,t,bias,valid);
		pblock->GetValue(PB_PRESERVEMAT1,t,preserveMat,valid);
		pblock->GetValue(PB_PRESERVESMOOTH1,t,preserveSmooth,valid);
		pblock->GetValue(PB_MAXEDGE1,t,maxEdge,valid);
	} else {
		pblock->GetValue(PB_FACETHRESH2,t,faceThresh,valid);
		pblock->GetValue(PB_EDGETHRESH2,t,edgeThresh,valid);	
		pblock->GetValue(PB_BIAS2,t,bias,valid);
		pblock->GetValue(PB_PRESERVEMAT2,t,preserveMat,valid);
		pblock->GetValue(PB_PRESERVESMOOTH2,t,preserveSmooth,valid);
		pblock->GetValue(PB_MAXEDGE2,t,maxEdge,valid);
		}
	
	assert(os->obj->IsSubClassOf(triObjectClassID));
	TriObject *triOb = (TriObject *)os->obj;
	nv = triOb->GetMesh().getNumVerts();
	nf = triOb->GetMesh().getNumFaces();

	if (preserveMat) flags |= OPTIMIZE_SAVEMATBOUNDRIES;
	if (preserveSmooth) flags |= OPTIMIZE_SAVESMOOTHBOUNDRIES;
	if (autoEdge) flags |= OPTIMIZE_AUTOEDGE;

	if (faceThresh!=0.0f) {
		GetAsyncKeyState(VK_ESCAPE); // clear the state
		HCURSOR hCur;
		if (nf > 2000) hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));

		triOb->GetMesh().Optimize(
			faceThresh,edgeThresh, bias*0.5f, maxEdge, flags,this);

		if (nf > 200) SetCursor(hCur);
		}

	triOb->GetMesh().InvalidateTopologyCache ();
	triOb->PointsWereChanged();
	triOb->UpdateValidity(GEOM_CHAN_NUM,valid);
	triOb->UpdateValidity(TOPO_CHAN_NUM,valid);
	triOb->UpdateValidity (TEXMAP_CHAN_NUM, valid);
	triOb->UpdateValidity (VERT_COLOR_CHAN_NUM, valid);

	if (pmapParam && pmapParam->GetParamBlock()==pblock && !TestAFlag(A_RENDER)) {
		TSTR buf;
		buf.printf("%d / %d",nv,triOb->GetMesh().getNumVerts());
		SetWindowText(GetDlgItem(pmapParam->GetHWnd(),IDC_OPT_VERTCOUNT),buf);
		buf.printf("%d / %d",nf,triOb->GetMesh().getNumFaces());
		SetWindowText(GetDlgItem(pmapParam->GetHWnd(),IDC_OPT_FACECOUNT),buf);
		}
	}

RefResult OptMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{
	switch (message) {
		case REFMSG_CHANGE: {			
			if (pmapParam && pmapParam->GetParamBlock()==pblock) {
				pmapParam->Invalidate();
				}			
			int man = FALSE;
			if (pblock) pblock->GetValue(PB_MANUPDATE,0,man,FOREVER);
			if (man) return REF_STOP;
			break;
			}

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_FACETHRESH1:
				case PB_FACETHRESH2:
				case PB_EDGETHRESH1:
				case PB_EDGETHRESH2:	gpd->dim = stdAngleDim; break;				
				case PB_MAXEDGE1:
				case PB_MAXEDGE2:		gpd->dim = stdWorldDim; break;
				default:				gpd->dim = defaultDim; break;
				}			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_FACETHRESH1:	gpn->name = GetString(IDS_RB_FACETHRESHL1); break;
				case PB_FACETHRESH2:	gpn->name = GetString(IDS_RB_FACETHRESHL2); break;
				case PB_EDGETHRESH1:	gpn->name = GetString(IDS_RB_EDGETHRESHL1); break;
				case PB_EDGETHRESH2:	gpn->name = GetString(IDS_RB_EDGETHRESHL2); break;
				case PB_BIAS1:			gpn->name = GetString(IDS_RB_BIASL1); break;
				case PB_BIAS2:			gpn->name = GetString(IDS_RB_BIASL2); break;
				case PB_MAXEDGE1:		gpn->name = GetString(IDS_RB_MAXEDGE1); break;
				case PB_MAXEDGE2:		gpn->name = GetString(IDS_RB_MAXEDGE2); break;
				default:				gpn->name = TSTR(_T("")); break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}




#endif // NO_MODIFIER_OPTIMIZE 