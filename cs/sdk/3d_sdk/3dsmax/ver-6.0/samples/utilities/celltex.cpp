/**********************************************************************
 *<
	FILE: CELLTEX.CPP

	DESCRIPTION: A Cellular texture

	CREATED BY: Rolf Berteig

	HISTORY: created 3/22/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "util.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "texutil.h"

extern HINSTANCE hInstance;

#define SHOW_3DMAPS_WITH_2D

#define CELLTEX_NAME		GetString(IDS_RB_CELLULAR)
#define CELLTEX_CLASSID	Class_ID(0xc90017a5,0x111940bb)

//class CellTexParamDlg;

#define NSUBTEX	3

class CellTex : public Texmap { 
	public:

		static ParamDlg* xyzGenDlg;	
		static ParamDlg* texoutDlg;

		IParamBlock2 *pblock;		// ref 0
		XYZGen *xyzGen;				// ref 1
		TextureOutput *texout;		// ref 2
		Texmap* subTex[NSUBTEX];	// ref 3-5

//		CellTexParamDlg *paramDlg;

		// Caches
		Interval ivalid;
		CRITICAL_SECTION csect;
		Color cellCol, divCol1, divCol2;
		float size, spread, low, high, mid, var, blend, varOff;
		float highMinuslow, midMinuslow, highMinusmid, iterations;
		float rough, smooth;
		int type, fract, useCellMap, useDiv1Map, useDiv2Map, adapt;

#ifdef SHOW_3DMAPS_WITH_2D
		TexHandle *texHandle;
		Interval texHandleValid;
#endif

		CellTex();
		~CellTex() {
			DeleteCriticalSection(&csect);
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}

		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		//ULONG Requirements(int subMtlNum);
		ULONG LocalRequirements(int subMtlNum);
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq);  
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) {Update(t,FOREVER); return ivalid;}		
		XYZGen* GetTheXYZGen() { return xyzGen; }

		// Evaluation
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);		
		Point3 EvalNormalPerturb(ShadeContext& sc);
		float CellFunc(Point3 pt,float dpsq,Point3 &np,BOOL noAdapt);

		// Methods to access texture maps of material
		int NumSubTexmaps() {return NSUBTEX;}
		Texmap* GetSubTexmap(int i) {return subTex[i];}
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {return CELLTEX_CLASSID;}
		SClass_ID SuperClassID() {return TEXMAP_CLASS_ID;}
		void GetClassName(TSTR& s) { s = GetString(IDS_RB_CELLULAR); } // mjm - 2.3.99
		void DeleteThis() {delete this;}	

		int NumSubs() {return 6;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;}

 		int NumRefs() {return 6;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		// Same as Marble
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

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

	};
/*
class CellTexParamDlg : public ParamDlg, public ParamMapUserDlgProc {
	public:
		CellTex *map;
		IMtlParams *ip;
		IParamMap *pmap;
		ParamDlg *xyzGenDlg;
		ParamDlg *texoutDlg;
		TexDADMgr dadMgr;
		HWND hwmedit;

		CellTexParamDlg(CellTex *m,IMtlParams *i,HWND hMedit);
		Class_ID ClassID() {return CELLTEX_CLASSID;}
		ReferenceTarget* GetThing() {return map;}
		void SetThing(ReferenceTarget *m);
		void DeleteThis();
		void SetTime(TimeValue t);
		void ReloadDialog();
		void ActivateDlg(BOOL onOff) {}
		void UpdateSubTexNames();
		int FindSubTexFromHWND(HWND hw);
		void SetStates(HWND hWnd, BOOL isFractal);

		// From ParamMapUserDlgProc
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
	};
*/
class CellTexClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new CellTex;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_CELLULAR_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() {return TEXMAP_CLASS_ID;}
	Class_ID 		ClassID() {return CELLTEX_CLASSID;}
	const TCHAR* 	Category() {return TEXMAP_CAT_3D;}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("cellularTex"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static CellTexClassDesc cellTexCD;
ClassDesc* GetCellTexDesc() {return &cellTexCD;}
ParamDlg* CellTex::xyzGenDlg;	
ParamDlg* CellTex::texoutDlg;

#define MAX_ITERATIONS	20.0f


//--- Parameter Map/Parameter block IDs ------------------------------
/*
#define PB_CELLCOL		0
#define PB_DIVCOL1		1
#define PB_DIVCOL2		2
#define PB_VAR			3
#define PB_SIZE			4
#define PB_SPREAD		5
#define PB_LOW			6
#define PB_MID			7
#define PB_HIGH			8
#define PB_TYPE			9
#define PB_FRACT		10
#define PB_ITER			11
#define PB_USECELLMAP	12
#define PB_USEDIV1MAP	13
#define PB_USEDIV2MAP	14
#define PB_ROUGH		15
#define PB_SMOOTH		16
#define PB_ADAPT		17


static int typeIDs[] = {IDC_CELLTEX_CIRCULAR,IDC_CELLTEX_IRREGULAR};

static ParamUIDesc descParam[] = {
	// Cell color
	ParamUIDesc(PB_CELLCOL,TYPE_COLORSWATCH,IDC_CELLTEX_CELLCOLOR),

	// Div color 1
	ParamUIDesc(PB_DIVCOL1,TYPE_COLORSWATCH,IDC_CELLTEX_DIVCOL1),
	
	// Div color 2
	ParamUIDesc(PB_DIVCOL2,TYPE_COLORSWATCH,IDC_CELLTEX_DIVCOL2),

	// Variation
	ParamUIDesc(
		PB_VAR,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_VAR,IDC_CELLTEX_VARSPIN,
		0.0f,100.0f,
		0.1f),	

	// Size	
	ParamUIDesc(
		PB_SIZE,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_SIZE,IDC_CELLTEX_SIZESPIN,
		0.001f,999999999.0f,
		0.1f),	

	// Spread
	ParamUIDesc(
		PB_SPREAD,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_SPREAD,IDC_CELLTEX_SPREADSPIN,
		0.001f,999999999.0f,
		0.01f),	

	// Low thresh
	ParamUIDesc(
		PB_LOW,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_LOW,IDC_CELLTEX_LOWSPIN,
		0.0f,1.0f,
		0.01f),	

	// Mid thresh
	ParamUIDesc(
		PB_MID,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_MID,IDC_CELLTEX_MIDSPIN,
		0.0f,1.0f,
		0.01f),	

	// High thresh
	ParamUIDesc(
		PB_HIGH,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_HIGH,IDC_CELLTEX_HIGHSPIN,
		0.0f,1.0f,
		0.01f),	

	// Type
	ParamUIDesc(PB_TYPE,TYPE_RADIO,typeIDs,2),

	// Fractal
	ParamUIDesc(PB_FRACT,TYPE_SINGLECHEKBOX,IDC_CELLTEX_FRACTAL),

	// Iterations
	ParamUIDesc(
		PB_ITER,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_ITER,IDC_CELLTEX_ITERSPIN,
		1.0f,25.0f,
		0.01f),	

	// Use cell color map
	ParamUIDesc(PB_USECELLMAP,TYPE_SINGLECHEKBOX,IDC_CELLTEX_CELLCOLOR_USEMAP),

	// Use div1 color map
	ParamUIDesc(PB_USEDIV1MAP,TYPE_SINGLECHEKBOX,IDC_CELLTEX_DIVCOL1_USEMAP),

	// Use div2 color map
	ParamUIDesc(PB_USEDIV2MAP,TYPE_SINGLECHEKBOX,IDC_CELLTEX_DIVCOL2_USEMAP),

	// Rough
	ParamUIDesc(
		PB_ROUGH,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_ROUGH,IDC_CELLTEX_ROUGHSPIN,
		0.0f,1.0f,
		0.01f),	

	// Smooth
	ParamUIDesc(
		PB_SMOOTH,
		EDITTYPE_FLOAT,
		IDC_CELLTEX_BUMPSMOOTH,IDC_CELLTEX_BUMPSMOOTHSPIN,
		0.0f,1.0f,
		0.01f),	

	// adaptive
	ParamUIDesc(PB_ADAPT,TYPE_SINGLECHEKBOX,IDC_CELLTEX_ADAPTIVE),
	};
*/




enum { cellular_params };  // pblock ID
// grad_params param IDs


enum 
{ 
	cellular_celcolor, cellular_divcol1, cellular_divcol2,
	cellular_celmap, cellular_divmap1, cellular_divmap2,
	cellular_map1_on, cellular_map2_on, cellular_map3_on, 
	cellular_variation,cellular_size,cellular_spread,
	cellular_lowthresh,cellular_midthresh,cellular_highthresh,
	cellular_type, cellular_fractal,cellular_iteration,
	cellular_rough, cellular_smooth,cellular_adaptive,// main grad params 

	cellular_coords, cellular_output	  // access for UVW mapping
};

static ParamBlockDesc2 cellular_param_blk ( cellular_params, _T("parameters"),  0, &cellTexCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_CELLTEX_PARAMS, IDS_RB_CELLPARAMS, 0, 0, NULL, 
	// params
	cellular_celcolor,	 _T("cellColor"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_CELLCOLOR,	
		p_default,		Color(1.0,1.0,1.0), 
		p_ui,			TYPE_COLORSWATCH, IDC_CELLTEX_CELLCOLOR, 
		end,
	cellular_divcol1,	 _T("divColor1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_DIVCOLOR1,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_CELLTEX_DIVCOL1, 
		end,
	cellular_divcol2,	 _T("divColor2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_DIVCOLOR2,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_CELLTEX_DIVCOL2, 
		end,
	cellular_celmap,		_T("cellMap"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_RB_CELLMAP,
		p_refno,		3,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CELLTEX_CELLCOLOR_MAP,
		end,
	cellular_divmap1,		_T("divMap1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_RB_DIVMAP1,
		p_refno,		4,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CELLTEX_DIVCOL1_MAP,
		end,
	cellular_divmap2,		_T("divMap2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_RB_DIVMAP2,
		p_refno,		5,
		p_subtexno,		2,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CELLTEX_DIVCOL2_MAP,
		end,
	cellular_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP1_ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_CELLTEX_CELLCOLOR_USEMAP,
		end,
	cellular_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP2_ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_CELLTEX_DIVCOL1_USEMAP,
		end,
	cellular_map3_on,	_T("map3Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP3_ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_CELLTEX_DIVCOL2_USEMAP,
		end,

	cellular_variation,	_T("variation"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_VARIATION,
		p_default,		0.f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_VAR, IDC_CELLTEX_VARSPIN, 0.1f, 
		end,
	cellular_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_SIZE,
		p_default,		5.f,
		p_range,		0.001f,999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_SIZE, IDC_CELLTEX_SIZESPIN, 0.1f, 
		end,
	cellular_spread,	_T("spread"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_SPREAD,
		p_default,		0.5f,
		p_range,		0.001f,999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_SPREAD,IDC_CELLTEX_SPREADSPIN, 0.01f, 
		end,

	cellular_lowthresh,	_T("lowThresh"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_LOW,
		p_default,		0.0f,
		p_range,		0.0f,1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_LOW,IDC_CELLTEX_LOWSPIN, 0.01f, 
		end,
	cellular_midthresh,	_T("midThresh"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_MID,
		p_default,		0.5f,
		p_range,		0.0f,1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_MID,IDC_CELLTEX_MIDSPIN, 0.01f, 
		end,
	cellular_highthresh,	_T("highThresh"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_HIGH,
		p_default,		1.0f,
		p_range,		0.0f,1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_HIGH,IDC_CELLTEX_HIGHSPIN, 0.01f, 
		end,

	cellular_type, _T("type"), TYPE_INT,				0,				IDS_RB_TYPE,
		p_default,		0,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 2, IDC_CELLTEX_CIRCULAR, IDC_CELLTEX_IRREGULAR,
		end,

	cellular_fractal, _T("fractal"), TYPE_BOOL,				0,		IDS_RB_FRACTAL,
		p_default,		0,
		p_ui,			TYPE_SINGLECHEKBOX,  IDC_CELLTEX_FRACTAL,
		p_enable_ctrls,	3, cellular_iteration,cellular_rough,cellular_adaptive,
		end,

	cellular_iteration,		_T("iteration"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_ITERATIONS,
		p_default,		3.f,
		p_range,		1.0, MAX_ITERATIONS,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_ITER,IDC_CELLTEX_ITERSPIN, 0.01f, 
		p_enabled,		FALSE,
		end,

	cellular_rough,		_T("roughness"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_ROUGHNESS,
		p_default,		0.f,
		p_range,		0.0, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_ROUGH,IDC_CELLTEX_ROUGHSPIN, 0.01f, 
		p_enabled,		FALSE,
		end,

	cellular_smooth,	_T("smooth"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_BUMPSMOOTHING,
		p_default,		0.1f,
		p_range,		0.0, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_BUMPSMOOTH,IDC_CELLTEX_BUMPSMOOTHSPIN, 0.01f, 
		end,

	cellular_adaptive, _T("adaptive"), TYPE_BOOL,				0,		IDS_PW_ADAPTIVE,
		p_default,		1,
		p_ui,			TYPE_SINGLECHEKBOX,  IDC_CELLTEX_ADAPTIVE,
		p_enabled,		FALSE,
		end,
	cellular_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDS,
		p_refno,		1, 
		end,
	cellular_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_OUTPUT,
		p_refno,		2, 
		end,


	end
);


#define PARAMDESC_LENGH 18

static ParamBlockDescID descVer0[] = {
	{ TYPE_POINT3, NULL, TRUE, cellular_celcolor }, // Cell color
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol1 }, // Div col 1
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol2 }, // Div col 2
	{ TYPE_FLOAT,  NULL, TRUE, cellular_variation },	// variation
	{ TYPE_FLOAT,  NULL, TRUE, cellular_size },	// size
	{ TYPE_FLOAT,  NULL, TRUE, cellular_spread },	// spread
	{ TYPE_FLOAT,  NULL, TRUE, cellular_lowthresh },	// low thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_midthresh },	// mid thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_highthresh },	// high thresh
	{ TYPE_INT,  NULL, FALSE, cellular_type },	// type
	{ TYPE_INT,  NULL, FALSE, cellular_fractal },	// fractal
	{ TYPE_FLOAT,  NULL, TRUE, cellular_iteration },// iterations	
	};

static ParamBlockDescID descVer1[] = {
	{ TYPE_POINT3, NULL, TRUE, cellular_celcolor }, // Cell color
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol1 }, // Div col 1
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol2 }, // Div col 2
	{ TYPE_FLOAT,  NULL, TRUE, cellular_variation },	// variation
	{ TYPE_FLOAT,  NULL, TRUE, cellular_size },	// size
	{ TYPE_FLOAT,  NULL, TRUE, cellular_spread },	// spread
	{ TYPE_FLOAT,  NULL, TRUE, cellular_lowthresh },	// low thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_midthresh },	// mid thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_highthresh },	// high thresh
	{ TYPE_INT,  NULL, FALSE, cellular_type },	// type
	{ TYPE_INT,  NULL, FALSE, cellular_fractal },	// fractal
	{ TYPE_FLOAT,  NULL, TRUE, cellular_iteration },// iterations
	{ TYPE_INT,  NULL, FALSE, cellular_map1_on },	// use cell col map
	{ TYPE_INT,  NULL, FALSE, cellular_map2_on },	// use div1 col map
	{ TYPE_INT,  NULL, FALSE, cellular_map3_on },	// use div2 col map
	};

static ParamBlockDescID descVer2[] = {
	{ TYPE_RGBA, NULL, TRUE, cellular_celcolor }, // Cell color
	{ TYPE_RGBA, NULL, TRUE, cellular_divcol1 }, // Div col 1
	{ TYPE_RGBA, NULL, TRUE, cellular_divcol2 }, // Div col 2
	{ TYPE_FLOAT,  NULL, TRUE, cellular_variation },	// variation
	{ TYPE_FLOAT,  NULL, TRUE, cellular_size },	// size
	{ TYPE_FLOAT,  NULL, TRUE, cellular_spread },	// spread
	{ TYPE_FLOAT,  NULL, TRUE, cellular_lowthresh },	// low thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_midthresh },	// mid thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_highthresh },	// high thresh
	{ TYPE_INT,  NULL, FALSE, cellular_type },	// type
	{ TYPE_INT,  NULL, FALSE, cellular_fractal },	// fractal
	{ TYPE_FLOAT,  NULL, TRUE, cellular_iteration },// iterations
	{ TYPE_INT,  NULL, FALSE, cellular_map1_on },	// use cell col map
	{ TYPE_INT,  NULL, FALSE, cellular_map2_on },	// use div1 col map
	{ TYPE_INT,  NULL, FALSE, cellular_map3_on },	// use div2 col map
	{ TYPE_FLOAT,  NULL, TRUE, cellular_rough },// rough
	{ TYPE_FLOAT,  NULL, TRUE, cellular_smooth },// smooth
	{ TYPE_INT,  NULL, FALSE, cellular_adaptive },	// adaptive
	};

#define PBLOCK_LENGTH	18

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,12,0),	
	ParamVersionDesc(descVer1,15,1),
	ParamVersionDesc(descVer2,18,2)
	};
#define NUM_OLDVERSIONS	3

//#define CURRENT_VERSION	2
//static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);


/*
//--- CellTexParamDlg -------------------------------------------------------

CellTexParamDlg::CellTexParamDlg(CellTex *m,IMtlParams *i,HWND hMedit) 
	{
	dadMgr.Init(this);
	map = m;
	ip  = i;	
	hwmedit = hMedit;
	xyzGenDlg = map->xyzGen->CreateParamDlg(hwmedit, i);
	pmap = CreateMParamMap(
		descParam,PARAMDESC_LENGH,
		map->pblock,
		i,
		hInstance,
		MAKEINTRESOURCE(IDD_CELLTEX_PARAMS),
		GetString(IDS_RB_CELLPARAMS),
		0);	
	pmap->SetUserDlgProc(this);
	texoutDlg = map->texout->CreateParamDlg(hwmedit, i);
	}

void CellTexParamDlg::SetThing(ReferenceTarget *m)
	{
	assert(m->ClassID()==map->ClassID());
	map->paramDlg = NULL;
	map = (CellTex*)m;
	pmap->SetParamBlock(map->pblock);
	if (xyzGenDlg)
		xyzGenDlg->SetThing(map->xyzGen);
	if (texoutDlg)
		texoutDlg->SetThing(map->texout);
	map->paramDlg = this;
	ReloadDialog();
	}

void CellTexParamDlg::ReloadDialog() 
	{
	pmap->Invalidate();
	UpdateSubTexNames();	
	SetStates(pmap->GetHWnd(),map->fract);
	}

void CellTexParamDlg::SetTime(TimeValue t)
	{
	xyzGenDlg->SetTime(t);
	texoutDlg->SetTime(t);
	}

void CellTexParamDlg::DeleteThis()
	{
	static BOOL block = FALSE;
	if (block) return;
	map->paramDlg = NULL;
	block = TRUE;
	pmap->SetUserDlgProc(NULL);
	block = FALSE;
	DestroyMParamMap(pmap);
	xyzGenDlg->DeleteThis();
	texoutDlg->DeleteThis();
	delete this;
	}

int CellTexParamDlg::FindSubTexFromHWND(HWND hw)
	{
	if (hw==GetDlgItem(pmap->GetHWnd(),IDC_CELLTEX_CELLCOLOR_MAP)) return 0;
	if (hw==GetDlgItem(pmap->GetHWnd(),IDC_CELLTEX_DIVCOL1_MAP)) return 1;
	if (hw==GetDlgItem(pmap->GetHWnd(),IDC_CELLTEX_DIVCOL2_MAP)) return 2;
	return -1;
	}

void CellTexParamDlg::UpdateSubTexNames()
	{
	HWND hWnd = pmap->GetHWnd();
	ICustButton *but;

	but = GetICustButton(GetDlgItem(hWnd,IDC_CELLTEX_CELLCOLOR_MAP));
	if (map->subTex[0]) but->SetText(map->subTex[0]->GetFullName());
	else but->SetText(GetString(IDS_RB_NONE));
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd,IDC_CELLTEX_DIVCOL1_MAP));
	if (map->subTex[1]) but->SetText(map->subTex[1]->GetFullName());
	else but->SetText(GetString(IDS_RB_NONE));
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd,IDC_CELLTEX_DIVCOL2_MAP));
	if (map->subTex[2]) but->SetText(map->subTex[2]->GetFullName());
	else but->SetText(GetString(IDS_RB_NONE));
	ReleaseICustButton(but);

	}

void CellTexParamDlg::SetStates(HWND hWnd, BOOL isFractal)
	{
	ISpinnerControl *iIter  = GetISpinner(GetDlgItem(hWnd,IDC_CELLTEX_ITERSPIN));
	ISpinnerControl *iRough = GetISpinner(GetDlgItem(hWnd,IDC_CELLTEX_ROUGHSPIN));
	if (isFractal) {
		iIter->Enable();
		iRough->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_CELLTEX_ITERLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_CELLTEX_ROUGHLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_CELLTEX_ADAPTIVE),TRUE);
	} else {
		iIter->Disable();
		iRough->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_CELLTEX_ITERLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_CELLTEX_ROUGHLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_CELLTEX_ADAPTIVE),FALSE);
		}
	ReleaseISpinner(iIter);
	ReleaseISpinner(iRough);
	}

BOOL CellTexParamDlg::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {			
			ICustButton *but;
			but = GetICustButton(GetDlgItem(hWnd,IDC_CELLTEX_CELLCOLOR_MAP));
			but->SetDADMgr(&dadMgr);
			ReleaseICustButton(but);

			but = GetICustButton(GetDlgItem(hWnd,IDC_CELLTEX_DIVCOL1_MAP));
			but->SetDADMgr(&dadMgr);
			ReleaseICustButton(but);

			but = GetICustButton(GetDlgItem(hWnd,IDC_CELLTEX_DIVCOL2_MAP));
			but->SetDADMgr(&dadMgr);
			ReleaseICustButton(but);

			UpdateSubTexNames();
			SetStates(hWnd,IsDlgButtonChecked(hWnd,IDC_CELLTEX_FRACTAL));
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CELLTEX_FRACTAL:
					SetStates(hWnd,IsDlgButtonChecked(hWnd,IDC_CELLTEX_FRACTAL));
					return FALSE;

				case IDC_CELLTEX_CELLCOLOR_MAP:
					PostMessage(hwmedit,WM_TEXMAP_BUTTON,0 ,(LPARAM)this->map);
					break;
				case IDC_CELLTEX_DIVCOL1_MAP:
					PostMessage(hwmedit,WM_TEXMAP_BUTTON,1 ,(LPARAM)this->map);
					break;
				case IDC_CELLTEX_DIVCOL2_MAP:
					PostMessage(hwmedit,WM_TEXMAP_BUTTON,2 ,(LPARAM)this->map);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

*/
//--- CellTex Methods -----------------------------------------------

ParamDlg* CellTex::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
//	paramDlg = new CellTexParamDlg(this,imp,hwMtlEdit);	
//	return paramDlg;
	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = cellTexCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	masterDlg->AddDlg(texoutDlg);
//	celTex_param_blk.SetUserDlgProc(new NoiseDlgProc(this));
	return masterDlg;

	}

void CellTex::Update(TimeValue t, Interval& valid)
	{
	EnterCriticalSection(&csect);
	if (!ivalid.InInterval(t)) {
		ivalid = FOREVER;		
		xyzGen->Update(t,ivalid);
		texout->Update(t,ivalid);
/*	
		pblock->GetValue(PB_CELLCOL,t,cellCol,ivalid);
		pblock->GetValue(PB_DIVCOL1,t,divCol1,ivalid);
		pblock->GetValue(PB_DIVCOL2,t,divCol2,ivalid);
		pblock->GetValue(PB_VAR,t,var,ivalid);
		pblock->GetValue(PB_SIZE,t,size,ivalid);
		pblock->GetValue(PB_SPREAD,t,spread,ivalid);
		pblock->GetValue(PB_LOW,t,low,ivalid);
		pblock->GetValue(PB_MID,t,mid,ivalid);
		pblock->GetValue(PB_HIGH,t,high,ivalid);		
		pblock->GetValue(PB_TYPE,t,type,ivalid);
		pblock->GetValue(PB_FRACT,t,fract,ivalid);
		pblock->GetValue(PB_ITER,t,iterations,ivalid);		
		pblock->GetValue(PB_USECELLMAP,t,useCellMap,ivalid);
		pblock->GetValue(PB_USEDIV1MAP,t,useDiv1Map,ivalid);
		pblock->GetValue(PB_USEDIV2MAP,t,useDiv2Map,ivalid);		
		pblock->GetValue(PB_ROUGH,t,rough,ivalid);
		pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);
		pblock->GetValue(PB_ADAPT,t,adapt,ivalid);
*/		

		pblock->GetValue(cellular_celcolor,t,cellCol,ivalid);
		pblock->GetValue(cellular_divcol1,t,divCol1,ivalid);
		pblock->GetValue(cellular_divcol2,t,divCol2,ivalid);
		pblock->GetValue(cellular_variation,t,var,ivalid);
		pblock->GetValue(cellular_size,t,size,ivalid);
		pblock->GetValue(cellular_spread,t,spread,ivalid);
		pblock->GetValue(cellular_lowthresh,t,low,ivalid);
		pblock->GetValue(cellular_midthresh,t,mid,ivalid);
		pblock->GetValue(cellular_highthresh,t,high,ivalid);		
		pblock->GetValue(cellular_type,t,type,ivalid);
		pblock->GetValue(cellular_fractal,t,fract,ivalid);
		pblock->GetValue(cellular_iteration,t,iterations,ivalid);		
		pblock->GetValue(cellular_map1_on,t,useCellMap,ivalid);
		pblock->GetValue(cellular_map2_on,t,useDiv1Map,ivalid);
		pblock->GetValue(cellular_map3_on,t,useDiv2Map,ivalid);		
		pblock->GetValue(cellular_rough,t,rough,ivalid);
		pblock->GetValue(cellular_smooth,t,smooth,ivalid);
		pblock->GetValue(cellular_adaptive,t,adapt,ivalid);
		
		
		smooth *= 0.7f;
		rough = 2.0f-rough;

		highMinuslow = high-low;		
		midMinuslow = mid - low;
		highMinusmid = high - mid;		
		if (type) {
			spread = spread/2.0f;
			}		
		var /= 50.0f;
		varOff = 1.0f-var * 0.5f;	

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}		
		}
	valid &= ivalid;
	LeaveCriticalSection(&csect);
	}

void CellTex::Init()
	{
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference(1, GetNewDefaultXYZGen());

	if (texout) texout->Reset();
	else ReplaceReference(2, GetNewDefaultTextureOutput());
/*
	pblock->SetValue(PB_CELLCOL,0,Point3(1,1,1));
	pblock->SetValue(PB_DIVCOL1,0,Point3(.5f,.5f,.5f));
	pblock->SetValue(PB_DIVCOL2,0,Point3(0,0,0));
	pblock->SetValue(PB_SIZE,0,5.0f);
	pblock->SetValue(PB_SPREAD,0,0.5f);
	pblock->SetValue(PB_LOW,0,0.0f);
	pblock->SetValue(PB_MID,0,0.5f);
	pblock->SetValue(PB_HIGH,0,1.0f);
	pblock->SetValue(PB_FRACT,0,0);
	pblock->SetValue(PB_ITER,0,3.0f);
	pblock->SetValue(PB_USECELLMAP,0,1);
	pblock->SetValue(PB_USEDIV1MAP,0,1);
	pblock->SetValue(PB_USEDIV2MAP,0,1);
	pblock->SetValue(PB_SMOOTH,0,0.1f);	
	pblock->SetValue(PB_ADAPT,0,1);	
	if (paramDlg)  
		paramDlg->pmap->SetParamBlock(pblock);
*/
	fract = 0;
	ivalid.SetEmpty();
	}

void CellTex::Reset(){
	cellTexCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(3);
	DeleteReference(4);
	DeleteReference(5);
	Init();
	}

CellTex::CellTex()
	{
//	paramDlg = NULL;
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	pblock   = NULL;
	xyzGen   = NULL;
	texout   = NULL;
	subTex[0] = subTex[1] = subTex[2] = NULL;
	cellTexCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	InitializeCriticalSection(&csect);
	ivalid.SetEmpty();
	}


BOOL CellTex::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((xyzGenDlg!= NULL) && (dlg == xyzGenDlg))
		xyzGenDlg->SetThing(xyzGen);
	else if ((texoutDlg!= NULL) && (dlg == texoutDlg))
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR CellTex::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif

#define ITER	3.0f
static Point3 ptOffset(1000.0f,1000.0f,1000.0f);

AColor CellTex::EvalColor(ShadeContext& sc)
	{	
	// Get object point
	Point3 p,dp;
	if (gbufID) sc.SetGBufferID(gbufID);
	xyzGen->GetXYZ(sc,p,dp);
	p += ptOffset;
	p = p/size;
	
	// Eval maps
	Color cellC, div1C, div2C;
	if (useCellMap && subTex[0]) cellC = subTex[0]->EvalColor(sc);
	else cellC = cellCol;
	if (useDiv1Map && subTex[1]) div1C = subTex[1]->EvalColor(sc);
	else div1C = divCol1;
	if (useDiv2Map && subTex[2]) div2C = subTex[2]->EvalColor(sc);
	else div2C = divCol2;

	// Evaluate cell function
	float dist[2];
	int ids[2];
	float u;
	if (type) {
		if (fract) FractalCellFunction(p,iterations,rough,2,dist,ids);
		else CellFunction(p,2,dist,ids);
		u = 1.0f - (dist[1]-dist[0])/spread;		
	} else {
		if (fract) FractalCellFunction(p,iterations,rough,1,dist,ids);
		else CellFunction(p,1,dist,ids);
		u = dist[0]/spread;
		}

	// Vari cell color
	if (var>0.0f) {
		float vr = RandFromCellID(ids[0])*var + varOff;
		cellC.r = cellC.r*vr;
		cellC.g = cellC.g*vr;
		cellC.b = cellC.b*vr;
		cellC.ClampMinMax();		
		}

	if (u<low) return texout->Filter(RGBA(cellC));
	if (u>high) return texout->Filter(RGBA(div2C));
	if (u<mid) {
		u = (u-low)/(midMinuslow);
		return texout->Filter(RGBA(div1C*u + (1.0f-u)*cellC));
	} else {
		u = (u-mid)/(highMinusmid);
		return texout->Filter(RGBA(div2C*u + (1.0f-u)*div1C));
		}
	}

float CellTex::EvalMono(ShadeContext& sc)
	{
	return Intens(EvalColor(sc));
	}

#define SMOOTH	0.2f

float CellTex::CellFunc(Point3 pt,float dpsq,Point3 &np,BOOL noAdapt)
	{
	float dist[3];
	Point3 grad[3];
	float u, iter;
	
	if (fract) {
		if (adapt) {
			iter = iterations/dpsq;
			if (iter>MAX_ITERATIONS) iter = MAX_ITERATIONS; // RB 2/19/99: We run into some sort of floating point limitation (I think) when this gets over about 20 or so.
			if (iter<1.0f)  iter = 1.0f;
		} else {
			iter = iterations;
			}
		}

	if (type) {
		if (fract) FractalCellFunction(
			 pt/size,iter,rough,3,dist,NULL,grad,smooth);
		else CellFunction(pt/size,3,dist,NULL,grad,smooth);
		u  = (dist[1]-dist[0])/spread;
		np = (grad[1]-grad[0])/spread;
	} else {
		if (fract) FractalCellFunction(
			 pt/size,iter,rough,2,dist,NULL,grad,smooth);
		else CellFunction(pt/size,2,dist,NULL,grad,smooth);
		u  = dist[0]/spread;
		np = grad[0]/spread;
		}
	
#if 1
	if (u<low+SMOOTH) {
		if (u<low) {
			np = Point3(0,0,0);
		} else {
			float s = (u-low)/SMOOTH;
			np = np*s;
			}
		return 0.0f;
		}
	if (u>high) {
		if (u>high+SMOOTH) {
			np = Point3(0,0,0);
		} else {
			float s = 1.0f-(u-high)/SMOOTH;
			np = np*s;
			}
		return 1.0f;
		}
#else
	if (u<0.0f) {
		np = Point3(0,0,0);
		return 0.0f;
		}
	if (u>1.0f) {
		np = Point3(0,0,0);
		return 1.0f;
		}
#endif
	return u;
	}

Point3 CellTex::EvalNormalPerturb(ShadeContext& sc)
	{
	Point3 p,dp;
	xyzGen->GetXYZ(sc,p,dp);	
	p += ptOffset;
	Point3 np(0.0f,0.0f,0.0f);
	float dpsq = DotProd(dp,dp);		
	float d = CellFunc(p,dpsq,np,sc.InMtlEditor());

	Texmap* sub0 = (useCellMap && subTex[0])?subTex[0]:NULL; 
	Texmap* sub1 = (useDiv1Map && subTex[1])?subTex[1]:NULL; 
	Texmap* sub2 = (useDiv2Map && subTex[2])?subTex[2]:NULL; 
	if (d<low) {
		if (sub0) 
			np  = sub0->EvalNormalPerturb(sc);
		}
	else 
	if (d>high) {
		if (sub2) 
			np  = sub2->EvalNormalPerturb(sc);
		}
	else {
		Point3 M[3];
		xyzGen->GetBumpDP(sc,M);
		np = Point3( DotProd(np,M[0]),DotProd(np,M[1]),DotProd(np,M[2]));
		if (d<mid) {
			if (sub0||sub1) {
				float a,b;
				Point3 da,db;
				// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
				d = (d-low)/(midMinuslow);

				// div1C*u + (1.0f-u)*cellC) ;
				if (sub0) {
					a = sub0->EvalMono(sc);
					da = sub0->EvalNormalPerturb(sc);
					}
				else {
					 a = 1.0f;
					 da = Point3(0.0f,0.0f,0.0f);
					 }
				if (sub1) {
					b = sub1->EvalMono(sc);
					db = sub1->EvalNormalPerturb(sc);
					}
				else {
					 b = 1.0f;
					 db = Point3(0.0f,0.0f,0.0f);
					 }
				np = (b-a)*np + d*(db-da) + da;
				}
			} 
		else {
			if (sub1 || sub2) {
				float a,b;
				Point3 da,db;
				// div2C*u + (1.0f-u)*div1C);
				d = (d-mid)/(highMinusmid);
				if (sub1) {
					a = sub1->EvalMono(sc);
					da = sub1->EvalNormalPerturb(sc);
					}
				else {
					 a = 1.0f;
					 da = Point3(0.0f,0.0f,0.0f);
					 }
				if (sub2) {
					b = sub2->EvalMono(sc);
					db = sub2->EvalNormalPerturb(sc);
					}
				else {
					 b = 1.0f;
					 db = Point3(0.0f,0.0f,0.0f);
					 }
				np = (b-a)*np + d*(db-da)+ da;
				}
			}
		}

//	float d = CellFunc(p,dpsq,np,sc.InMtlEditor());
//	Point3 tmp;
//	float div = type ? -0.1875f : 0.0375f;
//	Point3 DP[3];
//	xyzGen->GetBumpDP(sc,DP);
//	np.x = (CellFunc(p+DP[0],dpsq,tmp,sc.InMtlEditor()) - d)/div;
//	np.y = (CellFunc(p+DP[1],dpsq,tmp,sc.InMtlEditor()) - d)/div;
//	np.z = (CellFunc(p+DP[2],dpsq,tmp,sc.InMtlEditor()) - d)/div;

	if (type) np = np * -0.5f;	
	

	return texout->Filter(sc.VectorFromNoScale(np,REF_OBJECT));
	}

void CellTex::SetSubTexmap(int i, Texmap *m)
	{
	ReplaceReference(i+3,m);
	if (i==0)
		{
		cellular_param_blk.InvalidateUI(cellular_celmap);
		ivalid.SetEmpty();
		}
	else if (i==1)
		{
		cellular_param_blk.InvalidateUI(cellular_divmap1);
		ivalid.SetEmpty();
		}
	else if (i==2)
		{
		cellular_param_blk.InvalidateUI(cellular_divmap2);
		ivalid.SetEmpty();
		}

//	if (paramDlg) paramDlg->UpdateSubTexNames();
	}


TSTR CellTex::GetSubTexmapSlotName(int i)
	{
	switch (i) {
		case 0:  return GetString(IDS_RB_CELLCOLOR);
		case 1:  return GetString(IDS_RB_DIVCOLOR1);
		case 2:  return GetString(IDS_RB_DIVCOLOR2);
		default: return _T("");
		}
	}

Animatable* CellTex::SubAnim(int i)
	{
	return GetReference(i);
	}

TSTR CellTex::SubAnimName(int i)
	{
	switch (i) {
		default:
		case 0: return GetString(IDS_RB_PARAMETERS);
		case 1: return GetString(IDS_RB_COORDINATES);
		case 2: return GetString(IDS_RB_OUTPUT);
		case 3: return GetString(IDS_RB_CELLMAP);
		case 4: return GetString(IDS_RB_DIVMAP1);
		case 5: return GetString(IDS_RB_DIVMAP2);
		}
	}

RefTargetHandle CellTex::GetReference(int i)
	{
	switch (i) {
		case 0:  return pblock;
		case 1:  return xyzGen;
		case 2:  return texout;
		default: return subTex[i-3];
		}
	}

void CellTex::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0:  pblock = (IParamBlock2*)rtarg; break;
		case 1:  xyzGen = (XYZGen *)rtarg; break;
		case 2:  texout = (TextureOutput *)rtarg; break;
		default: subTex[i-3] = (Texmap *)rtarg; break;
		}
	}

#define MTL_HDR_CHUNK 0x4000
IOResult CellTex::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	isave->EndChunk();
	if (res!=IO_OK) return res;
	return IO_OK;
	}

IOResult CellTex::Load(ILoad *iload)
	{
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &cellular_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

/*
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(
			versions, 
			NUM_OLDVERSIONS, 
			&curVersion, this, 0));
*/
	return IO_OK;
	}

RefTargetHandle CellTex::Clone(RemapDir &remap)
	{
	CellTex *map = new CellTex;
	*((MtlBase*)map) = *((MtlBase*)this);  // copy superclass stuff
	map->ReplaceReference(0,remap.CloneRef(pblock));	
	map->ReplaceReference(1,remap.CloneRef(xyzGen));
	map->ReplaceReference(2,remap.CloneRef(texout));
	for (int i=0; i<NSUBTEX; i++) {
		if (subTex[i]) map->ReplaceReference(3+i,remap.CloneRef(subTex[i]));
		}
	BaseClone(this, map, remap);
	return map;
	}

/*
ULONG CellTex::Requirements(int subMtlNum) 
	{
	return MTLREQ_XYZ;
	}
	*/

ULONG CellTex::LocalRequirements(int subMtlNum) 
	{
	return xyzGen->Requirements(subMtlNum);
	}

void CellTex::LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
	xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
	}

RefResult CellTex::NotifyRefChanged(
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
				cellular_param_blk.InvalidateUI(changing_param);
#ifdef SHOW_3DMAPS_WITH_2D
				if (changing_param != -1)
					DiscardTexHandle();
#endif
				}
#ifdef SHOW_3DMAPS_WITH_2D
			else if (hTarget == xyzGen) 
				{
				DiscardTexHandle();
				}
#endif
		
			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_CELLCOL:
				case PB_DIVCOL1:
				case PB_DIVCOL2:	gpd->dim = stdColor255Dim; break;				
				case PB_SIZE:		gpd->dim =  stdWorldDim; break;
				default:			gpd->dim = defaultDim; break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {				
				case PB_CELLCOL:  gpn->name = GetString(IDS_RB_CELLCOLOR); break;
				case PB_DIVCOL1:  gpn->name = GetString(IDS_RB_DIVCOLOR1); break;
				case PB_DIVCOL2:  gpn->name = GetString(IDS_RB_DIVCOLOR2); break;
				case PB_VAR:      gpn->name = GetString(IDS_RB_VARIATION); break;
				case PB_SIZE:     gpn->name = GetString(IDS_RB_SIZE); break;
				case PB_SPREAD:   gpn->name = GetString(IDS_RB_SPREAD); break;
				case PB_LOW:      gpn->name = GetString(IDS_RB_LOW); break;
				case PB_MID:      gpn->name = GetString(IDS_RB_MID); break;
				case PB_HIGH:     gpn->name = GetString(IDS_RB_HIGH); break;
				case PB_TYPE:     gpn->name = GetString(IDS_RB_TYPE); break;
				case PB_FRACT:    gpn->name = GetString(IDS_RB_FRACTAL); break;
				case PB_ITER:     gpn->name = GetString(IDS_RB_ITERATIONS); break;
				case PB_ROUGH:    gpn->name = GetString(IDS_RB_ROUGHNESS); break;
				case PB_SMOOTH:   gpn->name = GetString(IDS_RB_BUMPSMOOTHING); break;
				default:          gpn->name = GetString(IDS_RB_PARAMETER);  break;
				}
			return REF_STOP; 
			}
*/
		}
	return REF_SUCCEED;
	}


