/*===========================================================================*\
 |    File: Water.cpp
 |
 | Purpose: A 3D Map for creating watery and wavy effects.
 |          This is a port of the 3D Studio/DOS SXP by Dan Silva.
 |
 | History: Mark Meier, Began 02/06/97.
 |          MM, Last Change 02/07/97.
			Updated to Param Block2 by Peter Watje 12/1/1998
\*===========================================================================*/
/*===========================================================================*\
 | Include Files
\*===========================================================================*/
#include "procmaps.h"
#include "iparamm2.h"
#include "macrorec.h"
#include "buildver.h"
#include "resource.h"
#include "resourceOverride.h"
#ifndef NO_MAPTYPE_WATER // orb 01-07-2001

#define SHOW_3DMAPS_WITH_2D

/*===========================================================================*\
 | Miscellaneous Defines
\*===========================================================================*/
#define DEFAULT_NUM_WAVESETS	3
#define DEFAULT_WAVE_RADIUS		1000.f
#define DEFAULT_WAVE_LEN_MIN	50.f
#define DEFAULT_WAVE_LEN_MAX	50.f
#define DEFAULT_COLOR1			Color(1.0f, 1.0f, 1.0f)
#define DEFAULT_COLOR2			Color(0.0f, 0.0f, 0.0f)
//#define DEFAULT_NUM_WAVESETS	10
//#define DEFAULT_WAVE_RADIUS		800.f
//#define DEFAULT_WAVE_LEN_MIN	5.f
//#define DEFAULT_WAVE_LEN_MAX	50.f
//#define DEFAULT_COLOR1			Color(0.78f, 0.78f, 0.94f)
//#define DEFAULT_COLOR2			Color(0.1f, 0.1f, 0.78f)

// The unique ClassID
static Class_ID waterClassID(WATER_CLASS_ID, 0);
// This is the number of colors used
#define NUM_COLORS 2

// This is the number of sub-texmaps used
#define NUM_SUB_TEXMAPS 2

// This is the version number of the IPAS SXP files that can be read
#define WATER_SXP_VERSION 0x1D0A45EF

struct Col24 {
	ULONG r, g, b; 
};

static Color ColorFromCol24(Col24 a) {
	Color c;
	c.r = (float)a.r/255.0f;
	c.g = (float)a.g/255.0f;
	c.b = (float)a.b/255.0f;
	return c;
}

static Col24 Col24FromColor(Color a) {
	Col24 c;
	c.r = (ULONG)(a.r*255.0f);
	c.g = (ULONG)(a.g*255.0f);
	c.b = (ULONG)(a.b*255.0f);
	return c;
}

#pragma pack(1)
struct WaterState {
	ulong version;
	ulong count;
	ulong type;
	float size;
	float minperiod;
	float maxperiod;
	float amp;
	float speed;
	Col24 col1,col2;
	long startframe,endframe;
};
#pragma pack()

// These are various resource IDs
static int colID[2] = { IDC_COL1, IDC_COL2 };
//static int subTexId[NUM_SUB_TEXMAPS] = { IDC_TEX1, IDC_TEX2 };
//static int mapOnId[NUM_SUB_TEXMAPS] = { IDC_MAPON1, IDC_MAPON2 };

// Forward references
//class Water;
//class WaterDlgProc;

/*===========================================================================*\
 | Water/Wave Functions and Definitions
\*===========================================================================*/

// returns random number in [-1.0,1.0] 
float frand() {
	float r =  ((float)(rand()&0x7FFF))/16384.0f - 1.0f;
	return(r);
	}

static float LerpFraction(int fld, int f1, int f2) {
	int nflds, fld1,fld2;
	fld1 = 2*f1;
	fld2 = 2*f2;
	if (fld>=fld2) return(1.0f);
	else if (fld<=fld1) return(0.0f);
	else {
		nflds = fld2-fld1;
		if (nflds<=0) return(1.0f);
		else return((float)(fld-fld1)/((float)nflds));
	}
}

struct WaveDesc {
	float cent[3];
	float period;
	float rate;
	};

/*===========================================================================*\
 | Water 3D Texture Map Plug-In Class
\*===========================================================================*/
class Water : public Tex3D { 
	// This allows the class that manages the UI to access the private 
	// data members of this class.
//	friend class WaterDlg;

	// These are the current colors from the color swatch controls.
	Color col[NUM_COLORS];

	// These are the parameters managed by the parameter map
	int count;			// Num Wave Sets
	int type;			// 2D or 3D
	int randSeed;       // Random number seed
	float size;			// Wave radius
	float minperiod;	// Wave len min
	float maxperiod;	// Wave len max
	float amp;			// Amlitude
	float phase;		// PHase

	Tab<WaveDesc> waves;

	Point3 col1, col2;

#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif

	// This points to the XYZGen instance used to handle the 
	// 'Coordinates' rollup in the materials editor.
	// This is reference #0 of this class.
	XYZGen *xyzGen;
	// These are the sub-texmaps.  If these are set by the user
	// then the color of our texture is affected by the sub-texmaps
	// and not the color swatches.
	// These are reference #2 and #3 of this class.
	Texmap *subTex[NUM_SUB_TEXMAPS];
	// This holds the validity interval of the texmap.
	Interval texValidity;
	// This is the version of the texture loaded from disk.
	int fileVersion;
	// This points to the ParamDlg instance used to manage the UI
//	WaterDlg *paramDlg;

	public:
		static ParamDlg* xyzGenDlg;	
	// Indicates if a sub-texmap is to be used or not
		BOOL mapOn[NUM_SUB_TEXMAPS];
	// This is the parameter block which manages the data for the
	// spinner and color swatch controls.
	// This is reference #1 of this class.
		IParamBlock2 *pblock;

		// --- Methods inherited from Animatable ---
		Class_ID ClassID() { return waterClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_WATER); }  
		void DeleteThis() { delete this; }	

		// We have 4 sub-animatables.  These are the xyzGen, 
		// the pblock, and the two sub-texmaps
		int NumSubs() { return 2+NUM_SUB_TEXMAPS; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// --- Methods inherited from ReferenceMaker ---
		// We have 4 references.  These are the xyzGen, 
		// the pblock, and the two sub-texmaps
 		int NumRefs() { return 2+NUM_SUB_TEXMAPS; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged(Interval changeInt, 
			RefTargetHandle hTarget, PartID& partID, RefMessage message);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// --- Methods inherited from ReferenceTarget ---
		RefTargetHandle Clone(RemapDir &remap = NoRemap());

		// --- Methods inherited from MtlBase ---
		ULONG LocalRequirements(int subMtlNum) { 
			return xyzGen->Requirements(subMtlNum); 
		}
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
		}
		void Update(TimeValue t, Interval& ivalid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t);
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		int NumSubTexmaps() { return NUM_SUB_TEXMAPS; }
		Texmap* GetSubTexmap(int i);
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		// --- Methods inherited from Texmap ---
		XYZGen *GetTheXYZGen() { return xyzGen; }
		RGBA EvalColor(ShadeContext& sc);
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// --- Methods inherited from Tex3D ---
		void ReadSXPData(TCHAR *name, void *sxpdata);

		// --- Methods of Water ---
		Water();
		~Water() {
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}
		void SwapInputs(); 
		void NotifyChanged();
		void SetNum(int i, TimeValue t, BOOL init = FALSE);
		void SetSize(float f, TimeValue t, BOOL init = FALSE);
		void SetLenMin(float f, TimeValue t, BOOL init = FALSE);
		void SetLenMax(float f, TimeValue t, BOOL init = FALSE);
		void SetAmp(float f, TimeValue t, BOOL init = FALSE);
		void SetPhase(float f, TimeValue t, BOOL init = FALSE);
		void SetColor(int i, Color c, TimeValue t);
		void SetRandSeed(int i, BOOL init = FALSE);
		void ClampFloat(float &f, float min, float max);
		void ClampInt(int &i, int min, int max);
		float swave(float *v, float rate);
		float ScalarWave(float *p);
		void VectorWave(Point3 &p, Point3 &n);
		void vwave(float *v, float rate, float *w);
		void ReInit();
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

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

// This is the Class Descriptor for the Water 3D Texture plug-in
class WaterClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return 1; }
		void *			Create(BOOL loading) { 	return new Water; }
		const TCHAR *	ClassName() { return GetString(IDS_DS_WATER_CDESC); } // mjm - 2.3.99
		SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
		Class_ID 		ClassID() { return waterClassID; }
		const TCHAR* 	Category() { return TEXMAP_CAT_3D; }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
		const TCHAR*	InternalName() { return _T("water"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};
static WaterClassDesc waterCD;
ClassDesc *GetWaterDesc() { return &waterCD; }

ParamDlg* Water::xyzGenDlg;	


/*===========================================================================*\
 | Class to Manage the User Interface in the Materials Editor
\*===========================================================================*/
/*
class WaterDlg: public ParamDlg {
	public:
		// This is our UI rollup page window handle in the materials editor
		HWND hParamDlg;
		// Window handle of the materials editor dialog itself
		HWND hMedit;
		// Interface for calling methods provided by MAX
		IMtlParams *ip;
		// The current Water being edited.
		Water *theTex;
		// Parameter Map for handling UI controls
		IParamMap *pmap;
		// Custom buttons for texture maps
		ICustButton *iCustButton[NUM_SUB_TEXMAPS];
		// Custom conrols for the colors
		IColorSwatch *cs[NUM_COLORS];
		// This is used inside the SetTime method to only update the UI
		// controls when the time slider has changed
		TimeValue cTime; 
		// Point to the XYZGenDlg we use
		ParamDlg *xyzGenDlg;
		BOOL valid;
		BOOL isActive;
		TexDADMgr dadMgr;

		// --- Methods inherited from ParamDlg ---
		Class_ID ClassID();
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing();
		void SetTime(TimeValue t);
		int FindSubTexFromHWND(HWND hw);
		void ReloadDialog();
		void ActivateDlg(BOOL onOff);
		void DeleteThis() { delete this; }

		// --- WaterDlg Methods ---
		WaterDlg(HWND hwMtlEdit, IMtlParams *imp, Water *m); 
		~WaterDlg();
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		void UpdateSubTexNames();
		void LoadDialog();
		void UpdateMtlDisplay() { ip->MtlChanged(); }
		void Invalidate();
};
*/

/*===========================================================================*\
 | Noise and Lerp Functions
\*===========================================================================*/
static void lerp_color(Col24 *c, Col24 *a, Col24 *b, float f) {
	int alph = (int)(4096*f);
	int ialph = 4096-alph;
	c->r = (ialph*a->r + alph*b->r)>>12;
	c->g = (ialph*a->g + alph*b->g)>>12;
	c->b = (ialph*a->b + alph*b->b)>>12;
}

/*===========================================================================*\
 | Parameter Map Related Data and Methods
\*===========================================================================*/
// Parameter block indices
/*
#define PB_NUM		0
#define PB_SIZE		1
#define PB_LEN_MIN	2 
#define PB_LEN_MAX	3
#define PB_AMP		4
#define PB_PHASE	5
#define PB_TYPE		6
#define PB_COL1		7
#define PB_COL2		8
#define PB_SEED		9
*/

// Spinner limits
#define MIN_NUM 1
#define MAX_NUM 50

#define MIN_SIZE 0.0001f
#define MAX_SIZE 999999999.0f

#define MIN_LEN_MIN 0.0f
#define MAX_LEN_MIN 999999999.0f

#define MIN_LEN_MAX 0.0f
#define MAX_LEN_MAX 999999999.0f

#define MIN_AMP 0.0f
#define MAX_AMP 10000.0f

#define MIN_SPEED 0.0f
#define MAX_SPEED 10000.0f

// Paramter block version number
#define WATER_PB_VERSION 2

// Resource IDs of the radio buttons
static int typeIDs[] = { IDC_RADIO_3D, IDC_RADIO_2D };

// Array of parameter descriptors
/*
static ParamUIDesc paramDesc[] = {
	ParamUIDesc(
		PB_NUM, 
		EDITTYPE_INT, 
		IDC_NUM_EDIT,IDC_NUM_SPIN, 
		MIN_NUM, MAX_NUM, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_SIZE, 
		EDITTYPE_FLOAT, 
		IDC_SIZE_EDIT,IDC_SIZE_SPIN, 
		MIN_SIZE, MAX_SIZE, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_LEN_MIN, 
		EDITTYPE_FLOAT, 
		IDC_LEN_MIN_EDIT,IDC_LEN_MIN_SPIN, 
		MIN_LEN_MIN, MAX_LEN_MIN, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_LEN_MAX, 
		EDITTYPE_FLOAT, 
		IDC_LEN_MAX_EDIT,IDC_LEN_MAX_SPIN, 
		MIN_LEN_MAX, MAX_LEN_MAX, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_AMP, 
		EDITTYPE_FLOAT, 
		IDC_AMP_EDIT,IDC_AMP_SPIN, 
		MIN_AMP, MAX_AMP, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_PHASE, 
		EDITTYPE_FLOAT, 
		IDC_SPEED_EDIT,IDC_SPEED_SPIN, 
		MIN_SPEED, MAX_SPEED, 
		SPIN_AUTOSCALE),

	ParamUIDesc(PB_TYPE, TYPE_RADIO, typeIDs, 2),

	ParamUIDesc(PB_COL1, TYPE_COLORSWATCH, IDC_COL1),
	ParamUIDesc(PB_COL2, TYPE_COLORSWATCH, IDC_COL2),
	ParamUIDesc(
		PB_SEED, 
		EDITTYPE_INT, 
		IDC_RSEED_EDIT,IDC_RSEED_SPIN, 
		0, 65535, 
		SPIN_AUTOSCALE), 
};
*/

enum { water_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	water_num,water_size,water_len_min,water_len_max,
	water_amp,water_phase,water_type,
	water_seed,
	water_color1, water_color2,
	water_map1, water_map2,
	water_mapon1,water_mapon2,
	water_coords,	  // access for UVW mapping
};

static ParamBlockDesc2 water_param_blk ( water_params, _T("parameters"),  0, &waterCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_WATER, IDS_DS_WATER_PARAMS, 0, 0, NULL, 
	// params

	water_num,	_T("numWaveSets"),   TYPE_INT,			P_ANIMATABLE,	IDS_DS_NUMWAVSETS,
		p_default,		DEFAULT_NUM_WAVESETS,
		p_range,		MIN_NUM, MAX_NUM,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_NUM_EDIT,IDC_NUM_SPIN, 1.0f, 
		end,


	water_size,	_T("waveRadius"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_WAVERAD,
		p_default,		DEFAULT_WAVE_RADIUS,
		p_range,		MIN_SIZE, MAX_SIZE, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZE_EDIT,IDC_SIZE_SPIN,  0.1f, 
		end,

	water_len_min,	_T("waveLenMin"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_WAVELENMIN,
		p_default,		DEFAULT_WAVE_LEN_MIN,
		p_range,		MIN_LEN_MIN, MAX_LEN_MIN, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LEN_MIN_EDIT,IDC_LEN_MIN_SPIN, 0.1f, 
		end,
	water_len_max,	_T("waveLenMax"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_WAVELENMAX,
		p_default,		DEFAULT_WAVE_LEN_MAX,
		p_range,		MIN_LEN_MAX, MAX_LEN_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LEN_MAX_EDIT,IDC_LEN_MAX_SPIN, 0.1f, 
		end,


	water_amp,	_T("amplitude"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_AMPL,
		p_default,		1.f,
		p_range,		MIN_AMP, MAX_AMP, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_AMP_EDIT,IDC_AMP_SPIN, 0.1f, 
		end,
	water_phase,	_T("phase"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_PHASE,
		p_default,		0.f,
		p_range,		MIN_SPEED, MAX_SPEED, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SPEED_EDIT,IDC_SPEED_SPIN, 0.1f, 
		end,
	water_type, _T("distribution"), TYPE_INT,				0,				IDS_PW_DISTRIBUTION,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_RADIO_3D, IDC_RADIO_2D,
		end,

	water_seed,	_T("randomSeed"),   TYPE_INT,			0,	IDS_PW_RSEED,
		p_default,		30159,
		p_range,		0, 65535, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_RSEED_EDIT,IDC_RSEED_SPIN, 1.0f, 
		end,

	water_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL1,	
		p_default,		DEFAULT_COLOR1, 
		p_ui,			TYPE_COLORSWATCH, IDC_COL1, 
		end,
	water_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL2,	
		p_default,		DEFAULT_COLOR2, 
		p_ui,			TYPE_COLORSWATCH, IDC_COL2, 
		end,

	water_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX1,
		end,
	water_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX2,
		end,
	water_mapon1,	_T("map1On"), TYPE_BOOL,			0,				IDS_PW_MAPON1,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	water_mapon2,	_T("map2On"), TYPE_BOOL,			0,				IDS_PW_MAPON2,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	water_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDINATES,
		p_refno,		0, 
		end,

	end
);
// The number of descriptors in the paramDesc array
#define PARAMDESC_LENGTH 10

// Parameter block parameters	
static ParamBlockDescID pbdesc[] = {
	{ TYPE_INT,   NULL, TRUE,  water_num }, // count
	{ TYPE_FLOAT, NULL, TRUE,  water_size}, // size
	{ TYPE_FLOAT, NULL, TRUE,  water_len_min }, // len_min
	{ TYPE_FLOAT, NULL, TRUE,  water_len_max }, // len_max
	{ TYPE_FLOAT, NULL, TRUE,  water_amp }, // amp
	{ TYPE_FLOAT, NULL, TRUE,  water_phase }, // phase
	{ TYPE_INT,   NULL, FALSE, water_type }, // type
	{ TYPE_RGBA,  NULL, TRUE,  water_color1 }, // color 1
	{ TYPE_RGBA,  NULL, TRUE,  water_color2 },  // color 2
	{ TYPE_INT,  NULL, FALSE,  water_seed }  // randSeed
};
// The number of parameters in the parameter block
#define PB_LENGTH 10

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,10,1)	// Version 1 params
	};

// The names of the parameters in the parameter block
static int nameIDs[] = {
	IDS_DS_NUMWAVSETS, IDS_DS_WAVERAD, IDS_DS_WAVELENMIN,
	IDS_DS_WAVELENMAX, IDS_DS_AMPL, IDS_DS_PHASE, IDS_DS_TYPE,
	IDS_DS_COL1, IDS_DS_COL2, IDS_DS_SEED };

// This is the class that allows the sub-map buttons to be processed.
/*
class WaterDlgProc : public ParamMapUserDlgProc {
	public:
		WaterDlg *theDlg;
		WaterDlgProc(WaterDlg *s) { theDlg = s; }
		BOOL DlgProc(TimeValue t, IParamMap *map,
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { delete this; }
};

// This is the dialog proc to process the texmap buttons
BOOL WaterDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	theDlg->isActive = TRUE;
	BOOL res = theDlg->PanelProc(hWnd, msg, wParam, lParam);
	theDlg->isActive = FALSE;
	return res;
}

BOOL WaterDlg::PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG: {
			for (int i = 0; i < NUM_COLORS; i++) 
   				cs[i] = GetIColorSwatch(GetDlgItem(hParamDlg, colID[i]),
   					theTex->col[i], theTex->GetSubTexmapSlotName(i).data());
			for (i = 0; i < NUM_SUB_TEXMAPS; i++) {
				iCustButton[i] = GetICustButton(GetDlgItem(hWnd, subTexId[i]));
				iCustButton[i]->SetDADMgr(&dadMgr);
				SetCheckBox(hWnd, mapOnId[i], theTex->mapOn[i]);
			}
			return TRUE;
		}
		break;

		case WM_COMMAND:  
		    switch (id) {
				case IDC_TEX1: 
					PostMessage(hMedit, WM_TEXMAP_BUTTON, 0, (LPARAM)theTex);
					break;

				case IDC_TEX2: 
					PostMessage(hMedit, WM_TEXMAP_BUTTON, 1, (LPARAM)theTex);
					break;

				case IDC_SWAP: {
					theTex->SwapInputs(); 
					IParamBlock *pb = (IParamBlock *)pmap->GetParamBlock();
					pb->SetValue(PB_COL1, cTime, theTex->col[0]);
					pb->SetValue(PB_COL2, cTime, theTex->col[1]);
					pmap->Invalidate();
					UpdateSubTexNames();
					UpdateMtlDisplay();
					theTex->NotifyChanged();
					}
					break;

				case IDC_MAPON1:
					theTex->mapOn[0] = GetCheckBox(hWnd, id);
					theTex->NotifyChanged();
					UpdateMtlDisplay();
					break;

				case IDC_MAPON2:
					theTex->mapOn[1] = GetCheckBox(hWnd, id);
					theTex->NotifyChanged();
					UpdateMtlDisplay();
					break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
		case WM_CUSTEDIT_ENTER: {
			theTex->ReInit();
			theTex->NotifyChanged();
		    UpdateMtlDisplay();
			break;
		}

		case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_MOUSEMOVE:
			ip->RollupMouseMessage(hWnd, msg, wParam, lParam);
			return FALSE;

		case WM_PAINT: 	
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
			}
			break;

		case WM_CLOSE: 
			break;       

		case WM_DESTROY: 
			break;
   	}
	return FALSE;
}
*/
/*===========================================================================*\
 | WaterDlg Methods
\*===========================================================================*/
// --- WaterDlg Methods ---
// Constructor.
// This is called from within the Water::CreateParamDlg method.  That
// method is passed the handle to the materials editor dialog, and an
// interface for calling methods of MAX.  These are passed in here and stored.
/*
WaterDlg::WaterDlg(HWND hwMtlEdit, IMtlParams *imp, Water *m) { 
	dadMgr.Init(this);
	hMedit = hwMtlEdit;
	ip = imp;
	theTex = m; 
    valid = FALSE;
    isActive = FALSE;
	cTime = ip->GetTime();

	// This call allocates a new instance of the XYZGen class
	xyzGenDlg = theTex->xyzGen->CreateParamDlg(hMedit, imp);

	// Creates a parameter map to handle the display of texture map 
	// parameters in the material editor
	pmap = CreateMParamMap(paramDesc, PARAMDESC_LENGTH,
		theTex->pblock, ip, hInstance, MAKEINTRESOURCE(IDD_WATER),
		GetString(IDS_DS_WATER_PARAMS), 0);

	// Save the window handle of the rollup page
	hParamDlg = pmap->GetHWnd();

	// Establish the dialog proc to handle the custom button controls
	pmap->SetUserDlgProc(new WaterDlgProc(this));
}

// Destructor.
// This is called after the user changes to another sample slot in
// the materials editor that does not contain a Water texture.
// Note that it is not called if they do go to another Water -- in
// that case, the parameters in the rollup page are updated, but
// the entire page is not deleted.  This is accomplished by simply
// changing the parameter block pointer (done inside WaterDlg::SetThing()).
WaterDlg::~WaterDlg() {
	theTex->paramDlg = NULL;
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		ReleaseICustButton(iCustButton[i]);
		iCustButton[i] = NULL; 
	}
	// Delete the XYZGen class we created
	xyzGenDlg->DeleteThis();
	// Delete the parameter map
	DestroyMParamMap(pmap);
	pmap = NULL;
}

// This is called by the DADMgr
int WaterDlg::FindSubTexFromHWND(HWND hw) {
	for (int i=0; i<NUM_SUB_TEXMAPS; i++) {
		if (hw == iCustButton[i]->GetHwnd()) return i;
		}	
	return -1;
	}


// This is called when the dialog is loaded to set the names of the
// textures displayed
void WaterDlg::UpdateSubTexNames() {
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		TSTR nm;
		Texmap *m = theTex->subTex[i];
		if (m) 	
			nm = m->GetFullName();
		else
			nm = GetString(IDS_DS_NONE);
		iCustButton[i]->SetText(nm.data());
	}
}

// Update the dialog display with the values of the texture we are
// currently editing.
void WaterDlg::LoadDialog() {
	if (theTex) {
		Interval ivalid;
		theTex->Update(cTime, ivalid);

		ISpinnerControl *spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_NUM_SPIN));
		spin->SetValue(theTex->count, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SIZE_SPIN));
		spin->SetValue(theTex->size, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_LEN_MIN_SPIN));
		spin->SetValue(theTex->minperiod, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_LEN_MAX_SPIN));
		spin->SetValue(theTex->maxperiod, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_AMP_SPIN));
		spin->SetValue(theTex->amp, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SPEED_SPIN));
		spin->SetValue(theTex->phase, FALSE);
		ReleaseISpinner(spin);

		cs[0]->SetColor(theTex->col[0]);
		cs[1]->SetColor(theTex->col[1]);

		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) 
			SetCheckBox(hParamDlg, mapOnId[i], theTex->mapOn[i]);

		UpdateSubTexNames();
	}
}

// This method invalidates the rollup page so it will get redrawn
void WaterDlg::Invalidate() { 
	InvalidateRect(hParamDlg, NULL, FALSE); 
	valid = FALSE; 
}

// --- Methods inherited from ParamDlg ---
// Returns the Class_ID of the plug-in this dialog manages
Class_ID WaterDlg::ClassID() {
	return waterClassID; 
}

// This sets the current texture being edited to the texture passed
void WaterDlg::SetThing(ReferenceTarget *m) {
	assert(m->ClassID() == waterClassID);
	assert(m->SuperClassID() == TEXMAP_CLASS_ID);
	if (theTex) 
		theTex->paramDlg = NULL;

	// Set the pointer to the texmap being edited to the one passed.
	theTex = (Water *)m;

	// Point the parameter map parameter block pointer at the
	// one that is now being edited.
	pmap->SetParamBlock(theTex->pblock);
	if (theTex)
		theTex->paramDlg = this;

	// Let the XYZGen set the new one being edited
	xyzGenDlg->SetThing(theTex->xyzGen);

	// Update the dialog display with the values of the new texmap.
	LoadDialog();
}

// This returns the current texture being edited
ReferenceTarget *WaterDlg::GetThing() {
	return (ReferenceTarget *)theTex; 
}

// This method is called when the current time has changed.  
// This gives the developer an opportunity to update any user 
// interface data that may need adjusting due to the change in time.
void WaterDlg::SetTime(TimeValue t) {
	Interval ivalid;
	if (t != cTime) {
		xyzGenDlg->SetTime(t);
		cTime = t;
		theTex->Update(cTime, ivalid);
		LoadDialog();
		InvalidateRect(hParamDlg, NULL, 0);
	}
}

// This method should place values into all the parameter dialog's controls, 
// edit fields etc.  
void WaterDlg::ReloadDialog() {
	Interval ivalid;
	theTex->Update(cTime, ivalid);
	LoadDialog();
}

// This method is called when the dialog box becomes active or inactive. 
void WaterDlg::ActivateDlg(BOOL onOff) {
	for (int i = 0; i < NUM_COLORS; i++) {
		cs[i]->Activate(onOff);
	}
}
*/

//dialog stuff to get the Set Ref button
class WaterDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Water *water;		
		WaterDlgProc(Water *m) {water = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};



BOOL WaterDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_SWAP:
					{
					water = (Water*)map->GetParamBlock()->GetOwner(); 

					water->SwapInputs();
					}
				break;
				}
			break;
		}
	return FALSE;
	}


/*===========================================================================*\
 | Water Methods
\*===========================================================================*/
// --- Methods inherited from Animatable ---
// This method returns a pointer to the 'i-th' sub-anim.  
Animatable* Water::SubAnim(int i) {
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
	}
}



float Water::swave(float *v, float rate) {
	float d = (float)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	return(0.5f*(1.0f+(float)sin((d-phase*rate)*TWOPI)));
	}

float Water::ScalarWave(float *p) {
	int i;
	float n;
	float pd;
	float v[3];
	n = 0.0f;
	for (i=0; i < count; i++ )	{
		WaveDesc &wd = waves[i];
		pd = wd.period;
		v[0] = (p[0]-wd.cent[0])/pd;								
		v[1] = (p[1]-wd.cent[1])/pd;								
		v[2] = (p[2]-wd.cent[2])/pd;								
		n += swave(v, wd.rate)*pd/maxperiod;
	}
	return(n*amp/((float)count));
	}
						 
void Water::vwave(float *v, float rate, float *w) {
	float d = (float)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	float f = (float)cos((d-phase*rate)*TWOPI);
	w[0] = f*v[0]/d;
	w[1] = f*v[1]/d;
	w[2] = f*v[2]/d;
	}

void Water::VectorWave(Point3 &p, Point3 &n) {

	int i;
	float f,pd;
	Point3 d,v;
	n.x = n.y = n.z = 0.0f;
	for (i = 0; i < count; i++)	{
		WaveDesc &wd = waves[i];
		pd = wd.period;
		v.x = (p.x-wd.cent[0])/pd;								
		v.y = (p.y-wd.cent[1])/pd;								
		v.z = (p.z-wd.cent[2])/pd;								
		vwave(v,wd.rate,d);
		f = pd/maxperiod;
		n += f*d;
		}
	f = amp/((float)count);
	n *= f;
	}

void Water::ReInit() {
	float c[3], d;

	if (count!=waves.Count()) {
		waves.SetCount(count);
		waves.Resize(count);
		}

	// Reseed random number generator
	srand(randSeed); 

	// Compute wave centers on sphere with radius size
	for (int i = 0; i < count; i++) {
		WaveDesc &wv = waves[i];
		c[0] = frand();
		c[1] = (type == 0) ? frand() : 0.0f;
		c[2] = frand();
		d = size/(float)sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
		wv.cent[0] = c[0]*d;
		wv.cent[1] = c[1]*d;
		wv.cent[2] = c[2]*d;
		wv.period = (((float)(rand()&0x7FFF))/32768.0f)*
			(maxperiod-minperiod)+minperiod; 
		wv.rate = (float)sqrt(maxperiod/wv.period);
		}
	}

// This method returns the name of the 'i-th' sub-anim to appear in track view. 
TSTR Water::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_DS_COORDS);
		case 1: return GetString(IDS_DS_PARAMETERS);
		default: return GetSubTexmapTVName(i-2);
	}
}

// --- Methods inherited from ReferenceMaker ---
// Return the 'i-th' reference
RefTargetHandle Water::GetReference(int i) {
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
	}
}

// Save the 'i-th' reference
void Water::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
	}
}

// This method is responsible for responding to the change notification
// messages sent by the texmap dependants.
RefResult Water::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			texValidity.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				water_param_blk.InvalidateUI(changing_param);
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

			// One of the texmap dependants have changed.  We set our
			// validity interval to empty and invalidate the dialog
			// so it gets redrawn.
//			texValidity.SetEmpty();
//			if (hTarget != xyzGen) {
//				if (paramDlg) 
//					paramDlg->pmap->Invalidate();
//				}
			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			// This returns the 'dimension' of the parameter.  This is 
			// the type and order of magnitude of the parameter.
			GetParamDim *gpd = (GetParamDim *)partID;
			switch (gpd->index) {
				case PB_LEN_MIN:
				case PB_LEN_MAX:
				case PB_SIZE: gpd->dim =  stdWorldDim; 	break;
				case PB_AMP:
				case PB_NUM:
				case PB_PHASE:
					gpd->dim = defaultDim; break;
				case PB_COL1: 
				case PB_COL2: gpd->dim = stdColor255Dim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			// This returns the name that will appear in track view
			// of the parameter.
			GetParamName *gpn = (GetParamName *)partID;
			gpn->name = GetString(nameIDs[gpn->index]);
			return REF_STOP; 
		}
*/
	}
	return(REF_SUCCEED);
}

// Load/Save Chunk IDs
#define MTL_HDR_CHUNK			0x4000
#define WATER_VERS1_CHUNK		0x4001
#define MAPOFF_CHUNK			0x1000
#define PARAM2_CHUNK			0x1010

// This is called by the system to allow the plug-in to save its data
IOResult Water::Save(ISave *isave) { 
	IOResult res;

	// Save the common stuff from the base class
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK) 
		return res;
	isave->EndChunk();
	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();

/*
	// Save a version number chunk
	isave->BeginChunk(WATER_VERS1_CHUNK);
	isave->EndChunk();
	// Save the on/off status of the sub-texmaps
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		if (mapOn[i] == 0) {
			isave->BeginChunk(MAPOFF_CHUNK+i);
			isave->EndChunk();
		}
	}
*/
	return IO_OK;
}



class WaterPostLoad : public PostLoadCallback {
	public:
		Water *n;
		BOOL Param1;
		WaterPostLoad(Water *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( water_mapon1, 0, n->mapOn[0]);
				n->pblock->SetValue( water_mapon2, 0, n->mapOn[1]);
				}
			delete this; 
			} 
	};


// This is called by the system to allow the plug-in to load its data
IOResult Water::Load(ILoad *iload) { 
	IOResult res;
	int id;
	fileVersion = 0;
	BOOL Param1 = TRUE;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				// Load the common stuff from the base class
				res = MtlBase::Load(iload);
				break;
			case WATER_VERS1_CHUNK:
				// Set the version number
				fileVersion = 1;
				break;
			case PARAM2_CHUNK:
				// Set the version number
				Param1 = FALSE;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				// Set the sub-texmap on/off settings
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}

	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &water_param_blk, this, 1);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new WaterPostLoad(this,Param1));

	return IO_OK;
}

// --- Methods inherited from ReferenceTarget ---
// This method is called to have the plug-in clone itself.
RefTargetHandle Water::Clone(RemapDir &remap) {
	// Create a new instance of the plug-in class
	Water *newWater = new Water();

	// Copy superclass stuff
	*((MtlBase *)newWater) = *((MtlBase *)this);

	// Clone the items we reference
	newWater->ReplaceReference(0, remap.CloneRef(xyzGen));
	newWater->ReplaceReference(1, remap.CloneRef(pblock));
	newWater->col[0] = col[0];
	newWater->col[1] = col[1];
	newWater->count = count;
	newWater->size = size;
	newWater->minperiod = minperiod;
	newWater->maxperiod = maxperiod;
	newWater->amp = amp;
	newWater->phase = phase;
	newWater->type = type;
	newWater->randSeed = randSeed;
	newWater->texValidity.SetEmpty();	
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		newWater->subTex[i] = NULL;
		newWater->mapOn[i] = mapOn[i];
		if (subTex[i])
			newWater->ReplaceReference(i+2, remap.CloneRef(subTex[i]));
	}
	BaseClone(this, newWater, remap);
	// Return the new cloned texture
	return (RefTargetHandle)newWater;
}

// --- Methods inherited from MtlBase ---
// This method is called to return the validity interval of the texmap.
Interval Water::Validity(TimeValue t) { 
	Interval v;
	// Calling Update() sets texValidity.
	Update(t, v); 
	return texValidity; 
}

// This method is called to reset the texmap back to its default values.
void Water::Init() {
	// Reset the XYZGen or allocate a new one
	if (xyzGen) 
		xyzGen->Reset();
	else 
		ReplaceReference(0, GetNewDefaultXYZGen());	

	// This replaces the reference to the previous parameter block with
	// a new one.  Note that the previous one is automatically deleted
	// because when the last reference to an item is deleted, MAX deletes
	// the item itself.
//	ReplaceReference(1, CreateParameterBlock(pbdesc, 
//		PB_LENGTH, WATER_PB_VERSION));

//	if (paramDlg) 
//		paramDlg->pmap->SetParamBlock(pblock);

	// Set the inital parameters
	SetColor(0, DEFAULT_COLOR1, TimeValue(0));
	SetColor(1, DEFAULT_COLOR2, TimeValue(0));
	SetRandSeed(0x75cf);
	SetNum(DEFAULT_NUM_WAVESETS, TimeValue(0));
	SetSize(DEFAULT_WAVE_RADIUS, TimeValue(0));
	SetLenMin(DEFAULT_WAVE_LEN_MIN, TimeValue(0));
	SetLenMax(DEFAULT_WAVE_LEN_MAX, TimeValue(0));
	SetAmp(1.0f, TimeValue(0));
	SetPhase(0.0f, TimeValue(0));

	ReInit();

	type = 0;
	// Set the validity interval of the texture to empty
	texValidity.SetEmpty();
}


void Water::Reset() {
	waterCD.Reset(this, TRUE); // reset all pb2's
	// Delete the references to the two sub-texture maps
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

// This method gets called when the material or texture is to be displayed 
// in the material editor parameters area. 
ParamDlg* Water::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// Allocate a new instance of ParamDlg to manage the UI.  This will
	// create the rollup page in the materials editor.
//	WaterDlg *waterDlg = new WaterDlg(hwMtlEdit, imp, this);
	// Update the dialog display with the proper values of the texture.
//	waterDlg->LoadDialog();
//	paramDlg = waterDlg;
//	return waterDlg;
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = waterCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	water_param_blk.SetUserDlgProc(new WaterDlgProc(this));

	return masterDlg;
	
}

BOOL Water::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((xyzGenDlg!= NULL) && (dlg == xyzGenDlg))
		xyzGenDlg->SetThing(xyzGen);
	else 
		return FALSE;
	return TRUE;
}

// This method is called before rendering begins to allow the plug-in 
// to evaluate anything prior to the render so it can store this information.
void Water::Update(TimeValue t, Interval& ivalid) {		
	if (!texValidity.InInterval(t)) {
		texValidity.SetInfinite();
		xyzGen->Update(t, texValidity);
//		pblock->GetValue(PB_COL1, t, col[0], texValidity);
		pblock->GetValue(water_color1, t, col[0], texValidity);
		col[0].ClampMinMax();
//		pblock->GetValue(PB_COL2, t, col[1], texValidity);
		pblock->GetValue(water_color2, t, col[1], texValidity);
		col[1].ClampMinMax();
//		pblock->GetValue(PB_NUM, t, count, texValidity);
		pblock->GetValue(water_num, t, count, texValidity);
		ClampInt(count, (int) MIN_NUM, (int) MAX_NUM);
//		pblock->GetValue(PB_SIZE, t, size, texValidity);
		pblock->GetValue(water_size, t, size, texValidity);
		ClampFloat(size, MIN_SIZE, MAX_SIZE);
//		pblock->GetValue(PB_LEN_MIN, t, minperiod, texValidity);
		pblock->GetValue(water_len_min, t, minperiod, texValidity);
		ClampFloat(minperiod, MIN_LEN_MIN, MAX_LEN_MIN);	// > 6/11/02 - 2:42pm --MQM-- typo, was MIN_LEN_MIN, MAX_LEN_MAX
//		pblock->GetValue(PB_LEN_MAX, t, maxperiod, texValidity);
		pblock->GetValue(water_len_max, t, maxperiod, texValidity);
		ClampFloat(maxperiod, MIN_LEN_MAX, MAX_LEN_MAX);
//		pblock->GetValue(PB_AMP, t, amp, texValidity);
		pblock->GetValue(water_amp, t, amp, texValidity);
		ClampFloat(amp, MIN_AMP, MAX_AMP);
//		pblock->GetValue(PB_PHASE, t, phase, texValidity);
//		pblock->GetValue(PB_TYPE, t, type, texValidity);
		pblock->GetValue(water_phase, t, phase, texValidity);
		pblock->GetValue(water_type, t, type, texValidity);
		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t, texValidity);
//		pblock->GetValue(PB_SEED, t, randSeed, texValidity);
		pblock->GetValue(water_seed, t, randSeed, texValidity);

		pblock->GetValue(water_mapon1, t, mapOn[0], texValidity);
		pblock->GetValue(water_mapon2, t, mapOn[1], texValidity);

		ReInit();
		}
	}
	ivalid &= texValidity;
}

void Water::ClampFloat(float &f, float min, float max) {
	if (f < min) f = min;
	else if (f > max) f = max;
}

void Water::ClampInt(int &i, int min, int max) {
	if (i < min) i = min;
	else if (i > max) i = max;
}

// Returns a pointer to the 'i-th' sub-texmap managed by this texture.
Texmap *Water::GetSubTexmap(int i) { 
	return subTex[i]; 
}

// Stores the 'i-th' sub-texmap managed by the material or texture.
void Water::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2, m);
	if (i==0)
		{
		water_param_blk.InvalidateUI(water_map1);
		texValidity.SetEmpty();
		}
	else if (i==1)
		{
		water_param_blk.InvalidateUI(water_map2);
		texValidity.SetEmpty();
		}

//	if (paramDlg)
//		paramDlg->UpdateSubTexNames();
	}

// This name appears in the materials editor dialog when editing the
// 'i-th' sub-map.
TSTR Water::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_DS_COL1); 
		case 1:  return GetString(IDS_DS_COL2); 
		default: return TSTR(_T(""));
		}
	}
	 
// --- Methods inherited from Texmap ---
RGBA Water::EvalColor(ShadeContext& sc) {
	float d;
	float q[3];
	Point3 p, dp;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	xyzGen->GetXYZ(sc, p, dp);

	q[0] = p.x;
	q[1] = p.y;
	q[2] = p.z;
	d = ScalarWave(q);
	if (d>1.0f) d = 1.0f;

	// If we have sub-texmaps and they are enabled, get the colors from 
	// the sub-texmaps, otherwise get them from the color swatch
	RGBA c0 = (mapOn[0]&&subTex[0]) ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = (mapOn[1]&&subTex[1]) ? subTex[1]->EvalColor(sc): col[1];

	Col24 c;
	Col24 col1 = Col24FromColor(c0);
	Col24 col2 = Col24FromColor(c1);

	lerp_color(&c, &col1, &col2, d);
	return ColorFromCol24(c);
	}

Point3 Water::EvalNormalPerturb(ShadeContext& sc) {
	if (gbufID) 
		sc.SetGBufferID(gbufID);

	Point3 p, dp, np;
	xyzGen->GetXYZ(sc, p, dp);

	VectorWave(p, np);
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
	np = Point3( DotProd(np,M[0]),DotProd(np,M[1]),DotProd(np,M[2]));
	return sc.VectorFromNoScale(np,REF_OBJECT);
	}

// --- Methods inherited from Tex3D ---
void Water::ReadSXPData(TCHAR *name, void *sxpdata) {
	WaterState *state = (WaterState*)sxpdata;
	if (state != NULL && (state->version == WATER_SXP_VERSION)) {
		SetColor(0, ColorFromCol24(state->col1), TimeValue(0));
		SetColor(1, ColorFromCol24(state->col2), TimeValue(0));
		SetNum(state->count, TimeValue(0));
		SetSize(state->size, TimeValue(0));
		SetLenMin(state->minperiod, TimeValue(0));
		SetLenMax(state->maxperiod, TimeValue(0));
		SetAmp(state->amp, TimeValue(0));
		SetPhase(0.0f, TimeValue(0), TRUE); 
		}
	}

// --- Methods of Water ---
Water::Water() {
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
//	paramDlg = NULL;
	mapOn[0] = mapOn[1] = 1;
	waterCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	fileVersion = 0;
	type = 0;
	}

void Water::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Water::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif


void Water::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
//	pblock->SwapControllers(PB_COL1, PB_COL2);
	pblock->SwapControllers(water_color1,0, water_color2,0);
	water_param_blk.InvalidateUI(water_color1);
	water_param_blk.InvalidateUI(water_color2);
	water_param_blk.InvalidateUI(water_map1);
	water_param_blk.InvalidateUI(water_map2);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
	}

void Water::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
//	pblock->SetValue((i == 0) ? PB_COL1 : PB_COL2, t, c);
	pblock->SetValue((i == 0) ? water_color1 : water_color2, t, c);
	}

void Water::SetNum(int i, TimeValue t, BOOL init) { 
	count = i;
	waves.SetCount(count); 
	waves.Resize(count);
//	pblock->SetValue(PB_NUM, t, i);
	pblock->SetValue(water_num, t, i);
	if (init) ReInit();
	}

void Water::SetRandSeed(int i, BOOL init) { 
	randSeed  = i;
//	pblock->SetValue(PB_SEED, 0, i);
	pblock->SetValue(water_seed, 0, i);
	if (init) ReInit();
	}


void Water::SetSize(float f, TimeValue t, BOOL init) { 
	size = f; 
//	pblock->SetValue(PB_SIZE, t, f);
	pblock->SetValue(water_size, t, f);
	if (init) ReInit();
	}

void Water::SetLenMin(float f, TimeValue t, BOOL init) { 
	minperiod = f; 
//	pblock->SetValue(PB_LEN_MIN, t, f);
	pblock->SetValue(water_len_min, t, f);
	if (init) ReInit();
	}

void Water::SetLenMax(float f, TimeValue t, BOOL init) { 
	maxperiod = f; 
//	pblock->SetValue(PB_LEN_MAX, t, f);
	pblock->SetValue(water_len_max, t, f);
	if (init) ReInit();
	}

void Water::SetAmp(float f, TimeValue t, BOOL init) { 
	amp = f; 
//	pblock->SetValue(PB_AMP, t, f);
	pblock->SetValue(water_amp, t, f);
	if (init) ReInit();
	}

void Water::SetPhase(float f, TimeValue t, BOOL init) { 
	phase = f; 
//	pblock->SetValue(PB_PHASE, t, f);
	pblock->SetValue(water_phase, t, f);
	if (init) ReInit();
	}

#endif // NO_MAPTYPE_WATER
