/*===========================================================================*\
 |    File: Smoke.cpp
 |
 | Purpose: A 3D Map for creating amorphous fractal based turbulent patterns.
 |          This is a port of the 3D Studio/DOS SXP by Dan Silva.
 |
 | History: Mark Meier, Began 02/03/97.
 |          MM, Last Change 02/03/97.
  		    Updated to Param Block2 by Peter Watje 12/1/1998
\*===========================================================================*/
/*===========================================================================*\
 | Include Files
\*===========================================================================*/
#include "procmaps.h"
#include "iparamm2.h"
#include "resource.h"
#include "resourceOverride.h"
#include "macrorec.h"

#define SHOW_3DMAPS_WITH_2D

/*===========================================================================*\
 | Miscellaneous Defines
\*===========================================================================*/
// The unique ClassID
static Class_ID smokeClassID(SMOKE_CLASS_ID, 0);

// This is the number of colors used
#define NUM_COLORS 2

// This is the number of sub-texmaps used
#define NUM_SUB_TEXMAPS 2

// This is the version number of the IPAS SXP files that can be read
#define SMOKE_SXP_VERSION 0xDE08

#define MAXNITS 20

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
struct SmokeState {
	ulong version;
	float size, hfboost, speed;
	Col24 col1, col2;
	int nits;
	float power;
	long startframe, endframe;
};
#pragma pack()

#define frand() ( (((float)(rand()&0x7FFF))-16384.0f) /16384.0f)



// These are various resource IDs
static int colID[2] = { IDC_COL1, IDC_COL2 };
static int subTexId[NUM_SUB_TEXMAPS] = { IDC_TEX1, IDC_TEX2 };
static int mapOnId[NUM_SUB_TEXMAPS] = { IDC_MAPON1, IDC_MAPON2 };

// Forward references
//class Smoke;
//class SmokeDlgProc;

/*===========================================================================*\
 | Smoke 3D Texture Map Plug-In Class
\*===========================================================================*/
class Smoke : public Tex3D { 
	// This allows the class that manages the UI to access the private 
	// data members of this class.
//	friend class SmokeDlg;

	// These are the current colors from the color swatch controls.
	Color col[NUM_COLORS];

	// These are the parameters managed by the parameter map
	float size;
	int iter;
	int seed;
	float power;
	float phase;
	Point3 col1, col2;
	float xvel[MAXNITS];
	float yvel[MAXNITS];
	float zvel[MAXNITS];
	float lastpow;


	float hfboost; // This is currently a constant

	// This points to the XYZGen instance used to handle the 
	// 'Coordinates' rollup in the materials editor.
	// This is reference #0 of this class.
	XYZGen *xyzGen;
	// These are the sub-texmaps.  If these are set by the user
	// then the color of our texture is affected by the sub-texmaps
	// and not the color swatches.
	// These are reference #2 and #3 of this class.
	Texmap *subTex[NUM_SUB_TEXMAPS];
	// Indicates if a sub-texmap is to be used or not
	// This holds the validity interval of the texmap.
	Interval texValidity;
	// This is the version of the texture loaded from disk.
	int fileVersion;
	// This points to the ParamDlg instance used to manage the UI
//	SmokeDlg *paramDlg;

#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif
	public:
	// This is the parameter block which manages the data for the
	// spinner and color swatch controls.
	// This is reference #1 of this class.
		BOOL mapOn[NUM_SUB_TEXMAPS];
		static ParamDlg* xyzGenDlg;	
		IParamBlock2 *pblock;
		// --- Methods inherited from Animatable ---
		Class_ID ClassID() { return smokeClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_SMOKE); }  
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
		RGBA EvalColor(ShadeContext& sc);
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// --- Methods inherited from Tex3D ---
		void ReadSXPData(TCHAR *name, void *sxpdata);

		// --- Methods of Smoke ---
		Smoke();
		~Smoke() {
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif		
			}
		void SwapInputs(); 
		void NotifyChanged();
		void SetPhase(float f, TimeValue t);
		void SetSize(float f, TimeValue t);
		void SetExp(float f, TimeValue t);
		void SetIter(int i, TimeValue t);
		void SetColor(int i, Color c, TimeValue t);
		void ClampFloat(float &f, float min, float max);
		void ClampInt(int &i, int min, int max);
		float SmokeFunc(Point3 p, int iter);
		void InitVel(int seed);

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

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

};

// This is the Class Descriptor for the Smoke 3D Texture plug-in
class SmokeClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
		void *			Create(BOOL loading) { 	return new Smoke; }
		const TCHAR *	ClassName() { return GetString(IDS_DS_SMOKE_CDESC); } // mjm - 2.3.99
		SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
		Class_ID 		ClassID() { return smokeClassID; }
		const TCHAR* 	Category() { return TEXMAP_CAT_3D; }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
		const TCHAR*	InternalName() { return _T("smoke"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};
static SmokeClassDesc smokeCD;
ParamDlg* Smoke::xyzGenDlg;	

ClassDesc *GetSmokeDesc() { return &smokeCD; }

/*===========================================================================*\
 | Class to Manage the User Interface in the Materials Editor
\*===========================================================================*/
/*
class SmokeDlg: public ParamDlg {
	public:
		// This is our UI rollup page window handle in the materials editor
		HWND hParamDlg;
		// Window handle of the materials editor dialog itself
		HWND hMedit;
		// Interface for calling methods provided by MAX
		IMtlParams *ip;
		// The current Smoke being edited.
		Smoke *theTex;
		// Parameter Map for handling UI controls
		IParamMap *pmap;
		// Custom buttons for texture maps
		ICustButton *iCustButton[NUM_SUB_TEXMAPS];
		// Custom conrols for the colors
		IColorSwatch *cs[NUM_COLORS];
		// This is used inside the SetTime method to only update the UI
		// controls when the time slider has changed
		TimeValue curTime; 
		// This is used to implement Drag-and-Drop for sub-Texmaps.
		TexDADMgr dadMgr;
		// Point to the XYZGenDlg we use
		ParamDlg *xyzGenDlg;
		BOOL valid;
		BOOL isActive;

		// --- Methods inherited from ParamDlg ---
		Class_ID ClassID();
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing();
		void SetTime(TimeValue t);
		void ReloadDialog();
		void ActivateDlg(BOOL onOff);
		int FindSubTexFromHWND(HWND hw);
		void DeleteThis() { delete this; }

		// --- SmokeDlg Methods ---
		SmokeDlg(HWND hwMtlEdit, IMtlParams *imp, Smoke *m); 
		~SmokeDlg();
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
#define PB_SIZE		0
#define PB_ITER		1
#define PB_EXP		2 
#define PB_PHASE	3
#define PB_COL1		4
#define PB_COL2		5
*/
// Spinner limits
#define MIN_SIZE 0.001f
#define MAX_SIZE 999999999.0f

#define MIN_ITER 1
#define MAX_ITER 20

#define MIN_EXP 0.001f
#define MAX_EXP 5.0f

#define MIN_SPEED 0.0f
#define MAX_SPEED 100.0f

// Paramter block version number
#define SMOKE_PB_VERSION 2

enum { smoke_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	smoke_size, smoke_iteration,		
	smoke_exponent, smoke_phase,		
	smoke_color1, smoke_color2,
	smoke_map1, smoke_map2,
	smoke_mapon1,smoke_mapon2,
	smoke_coords,	  // access for UVW mapping
};


static ParamBlockDesc2 smoke_param_blk ( smoke_params, _T("parameters"),  0, &smokeCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_SMOKE, IDS_DS_SMOKE_PARAMS, 0, 0, NULL, 
	// params


	smoke_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_SIZE,
		p_default,		40.f,
		p_range,		MIN_SIZE, MAX_SIZE,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZE_EDIT,IDC_SIZE_SPIN, 0.1f, 
		end,
	smoke_iteration,	_T("iterations"),   TYPE_INT,			P_ANIMATABLE,	IDS_DS_ITER,
		p_default,		5,
		p_range,		MIN_ITER, MAX_ITER, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_ITER_EDIT,IDC_ITER_SPIN, 1.0f, 
		end,
	smoke_exponent,	_T("exponent"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_EXPONENT,
		p_default,		1.5f,
		p_range,		MIN_EXP, MAX_EXP,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EXP_EDIT,IDC_EXP_SPIN, 0.1f, 
		end,
	smoke_phase,	_T("phase"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_PHASE,
		p_default,		0.f,
		p_range,		MIN_SPEED, MAX_SPEED, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SPEED_EDIT,IDC_SPEED_SPIN,  0.1f, 
		end,
	smoke_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL1,	
		p_default,		Color(0.0, 0.0, 0.0), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL1, 
		end,
	smoke_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL2,	
		p_default,		Color(1.0f, 1.0f, 1.0f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL2, 
		end,
	smoke_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX1,
		end,
	smoke_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX2,
		end,
	smoke_mapon1,	_T("map1On"), TYPE_BOOL,			0,				IDS_PW_MAPON1,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	smoke_mapon2,	_T("map2On"), TYPE_BOOL,			0,				IDS_PW_MAPON2,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	smoke_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDINATES,
		p_refno,		0, 
		end,

	end
);

// Array of parameter descriptors
/*
static ParamUIDesc paramDesc[] = {
	ParamUIDesc(
		PB_SIZE, 
		EDITTYPE_FLOAT, 
		IDC_SIZE_EDIT,IDC_SIZE_SPIN, 
		MIN_SIZE, MAX_SIZE, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_ITER, 
		EDITTYPE_INT, 
		IDC_ITER_EDIT,IDC_ITER_SPIN, 
		MIN_ITER, MAX_ITER, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_EXP, 
		EDITTYPE_FLOAT, 
		IDC_EXP_EDIT,IDC_EXP_SPIN, 
		MIN_EXP, MAX_EXP, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_PHASE, 
		EDITTYPE_FLOAT, 
		IDC_SPEED_EDIT,IDC_SPEED_SPIN, 
		MIN_SPEED, MAX_SPEED, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(PB_COL1, TYPE_COLORSWATCH, IDC_COL1),
	ParamUIDesc(PB_COL2, TYPE_COLORSWATCH, IDC_COL2)
};
*/
// The number of descriptors in the paramDesc array
#define PARAMDESC_LENGTH 6

// Parameter block parameters	
static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE, smoke_size }, // size 
	{ TYPE_INT,   NULL, TRUE, smoke_iteration }, // iter
	{ TYPE_FLOAT, NULL, TRUE, smoke_exponent }, // exponent
	{ TYPE_FLOAT, NULL, TRUE, smoke_phase }, // phase
	{ TYPE_RGBA,  NULL, TRUE, smoke_color1 }, // color 1
	{ TYPE_RGBA,  NULL, TRUE, smoke_color2 }  // color 2
};
// The number of parameters in the parameter block
#define PB_LENGTH 6

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,6,1)	// Version 1 params
	};

// The names of the parameters in the parameter block
static int nameIDs[] = {
	IDS_DS_SIZE, IDS_DS_ITER, IDS_DS_EXPONENT, IDS_DS_PHASE, IDS_DS_COL1, IDS_DS_COL2
	};

/*
// This is the class that allows the sub-map buttons to be processed.
class SmokeDlgProc : public ParamMapUserDlgProc {
	public:
		SmokeDlg *theDlg;
		SmokeDlgProc(SmokeDlg *s) { theDlg = s; }
		BOOL DlgProc(TimeValue t, IParamMap *map,
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { delete this; }
};

// This is the dialog proc to process the texmap buttons
BOOL SmokeDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	theDlg->isActive = TRUE;
	BOOL res = theDlg->PanelProc(hWnd, msg, wParam, lParam);
	theDlg->isActive = FALSE;
	return res;
}

BOOL SmokeDlg::PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
					pb->SetValue(PB_COL1, curTime, theTex->col[0]);
					pb->SetValue(PB_COL2, curTime, theTex->col[1]);
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
 | SmokeDlg Methods
\*===========================================================================*/
// --- SmokeDlg Methods ---
// Constructor.
// This is called from within the Smoke::CreateParamDlg method.  That
// method is passed the handle to the materials editor dialog, and an
// interface for calling methods of MAX.  These are passed in here and stored.
/*
SmokeDlg::SmokeDlg(HWND hwMtlEdit, IMtlParams *imp, Smoke *m) { 
	dadMgr.Init(this);
	hMedit = hwMtlEdit;
	ip = imp;
	theTex = m; 
    valid = FALSE;
    isActive = FALSE;
	curTime = ip->GetTime();

	// This call allocates a new instance of the XYZGen class
	xyzGenDlg = theTex->xyzGen->CreateParamDlg(hMedit, imp);

	// Creates a parameter map to handle the display of texture map 
	// parameters in the material editor
	pmap = CreateMParamMap(paramDesc, PARAMDESC_LENGTH,
		theTex->pblock, ip, hInstance, MAKEINTRESOURCE(IDD_SMOKE),
		GetString(IDS_DS_SMOKE_PARAMS), 0);

	// Save the window handle of the rollup page
	hParamDlg = pmap->GetHWnd();

	// Establish the dialog proc to handle the custom button controls
	pmap->SetUserDlgProc(new SmokeDlgProc(this));
}

// Destructor.
// This is called after the user changes to another sample slot in
// the materials editor that does not contain a Smoke texture.
// Note that it is not called if they do go to another Smoke -- in
// that case, the parameters in the rollup page are updated, but
// the entire page is not deleted.  This is accomplished by simply
// changing the parameter block pointer (done inside SmokeDlg::SetThing()).
SmokeDlg::~SmokeDlg() {
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

// This is called when the dialog is loaded to set the names of the
// textures displayed
void SmokeDlg::UpdateSubTexNames() {
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
void SmokeDlg::LoadDialog() {
	if (theTex) {
		Interval ivalid;
		theTex->Update(curTime, ivalid);

		ISpinnerControl *spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SIZE_SPIN));
		spin->SetValue(theTex->size, FALSE);
		ReleaseISpinner(spin);
		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_ITER_SPIN));
		spin->SetValue(theTex->iter, FALSE);
		ReleaseISpinner(spin);
		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_EXP_SPIN));
		spin->SetValue(theTex->power, FALSE);
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
void SmokeDlg::Invalidate() { 
	InvalidateRect(hParamDlg, NULL, FALSE); 
	valid = FALSE; 
}

// --- Methods inherited from ParamDlg ---
// Returns the Class_ID of the plug-in this dialog manages
Class_ID SmokeDlg::ClassID() {
	return smokeClassID; 
}

// This sets the current texture being edited to the texture passed
void SmokeDlg::SetThing(ReferenceTarget *m) {
	assert(m->ClassID() == smokeClassID);
	assert(m->SuperClassID() == TEXMAP_CLASS_ID);
	if (theTex) 
		theTex->paramDlg = NULL;

	// Set the pointer to the texmap being edited to the one passed.
	theTex = (Smoke *)m;

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
ReferenceTarget *SmokeDlg::GetThing() {
	return (ReferenceTarget *)theTex; 
}

// This method is called when the current time has changed.  
// This gives the developer an opportunity to update any user 
// interface data that may need adjusting due to the change in time.
void SmokeDlg::SetTime(TimeValue t) {
	Interval ivalid;
	if (t != curTime) {
		xyzGenDlg->SetTime(t);
		curTime = t;
		theTex->Update(curTime, ivalid);
		LoadDialog();
		InvalidateRect(hParamDlg, NULL, 0);
	}
}

// This method should place values into all the parameter dialog's controls, 
// edit fields etc.  
void SmokeDlg::ReloadDialog() {
	Interval ivalid;
	theTex->Update(curTime, ivalid);
	LoadDialog();
}

// This method is called when the dialog box becomes active or inactive. 
void SmokeDlg::ActivateDlg(BOOL onOff) {
	for (int i = 0; i < NUM_COLORS; i++) {
		cs[i]->Activate(onOff);
	}
}

// This returns the index of the sub texmap whose window handle is passed.
// This is used by the Drag-and-Drop manager class (TexDADMgr).
int SmokeDlg::FindSubTexFromHWND(HWND hw) {
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		if (hw == iCustButton[i]->GetHwnd()) return i;
	}	
	return -1;
}
*/

//dialog stuff to get the Set Ref button
class SmokeDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Smoke *smoke;		
		SmokeDlgProc(Smoke *m) {smoke = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			smoke = (Smoke*)m;
			}

	};



BOOL SmokeDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_SWAP:
					{
					smoke = (Smoke*)map->GetParamBlock()->GetOwner(); 

					smoke->SwapInputs();
					}
				break;
				}
			break;
		}
	return FALSE;
	}



/*===========================================================================*\
 | Smoke Methods
\*===========================================================================*/
// --- Methods inherited from Animatable ---
// This method returns a pointer to the 'i-th' sub-anim.  
Animatable* Smoke::SubAnim(int i) {
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
	}
}

// This method returns the name of the 'i-th' sub-anim to appear in track view. 
TSTR Smoke::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_DS_COORDS);
		case 1: return GetString(IDS_DS_PARAMETERS);
		default: return GetSubTexmapTVName(i-2);
	}
}

// --- Methods inherited from ReferenceMaker ---
// Return the 'i-th' reference
RefTargetHandle Smoke::GetReference(int i) {
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
	}
}

// Save the 'i-th' reference
void Smoke::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
	}
}

// This method is responsible for responding to the change notification
// messages sent by the texmap dependants.
RefResult Smoke::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			texValidity.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				smoke_param_blk.InvalidateUI(changing_param);
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
//			}
			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			// This returns the 'dimension' of the parameter.  This is 
			// the type and order of magnitude of the parameter.
			GetParamDim *gpd = (GetParamDim *)partID;
			switch (gpd->index) {
				case PB_SIZE: gpd->dim =  stdWorldDim; 	break;
				case PB_ITER:
				case PB_EXP:
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
#define SMOKE_VERS1_CHUNK		0x4001
#define MAPOFF_CHUNK			0x1000
#define PARAM2_CHUNK			0x1010

// This is called by the system to allow the plug-in to save its data
IOResult Smoke::Save(ISave *isave) { 
	IOResult res;

	// Save the common stuff from the base class
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK) 
		return res;
	isave->EndChunk();
	// Save a version number chunk
	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
/*
	isave->BeginChunk(SMOKE_VERS1_CHUNK);
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



class SmokePostLoad : public PostLoadCallback {
	public:
		Smoke *n;
		BOOL Param1;
		SmokePostLoad(Smoke *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( smoke_mapon1, 0, n->mapOn[0]);
				n->pblock->SetValue( smoke_mapon2, 0, n->mapOn[1]);
				}
			delete this; 


			} 
	};


// This is called by the system to allow the plug-in to load its data
IOResult Smoke::Load(ILoad *iload) { 
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
			case SMOKE_VERS1_CHUNK:
				// Set the version number
				fileVersion = 1;
				break;
			case PARAM2_CHUNK:
				// Set the version number
				Param1=FALSE;
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
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &smoke_param_blk, this, 1);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new SmokePostLoad(this,Param1));

	return IO_OK;
}

// --- Methods inherited from ReferenceTarget ---
// This method is called to have the plug-in clone itself.
RefTargetHandle Smoke::Clone(RemapDir &remap) {
	// Create a new instance of the plug-in class
	Smoke *newSmoke = new Smoke();

	// Copy superclass stuff
	*((MtlBase *)newSmoke) = *((MtlBase *)this);

	// Clone the items we reference
	newSmoke->ReplaceReference(0, remap.CloneRef(xyzGen));
	newSmoke->ReplaceReference(1, remap.CloneRef(pblock));
	newSmoke->col[0] = col[0];
	newSmoke->col[1] = col[1];
	newSmoke->size = size;
	newSmoke->power = power;
	newSmoke->iter = iter;
	newSmoke->phase = phase;
	newSmoke->texValidity.SetEmpty();	
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		newSmoke->subTex[i] = NULL;
		newSmoke->mapOn[i] = mapOn[i];
		if (subTex[i])
			newSmoke->ReplaceReference(i+2, remap.CloneRef(subTex[i]));
	}
	BaseClone(this, newSmoke , remap);
	// Return the new cloned texture
	return (RefTargetHandle)newSmoke;
}

// --- Methods inherited from MtlBase ---
// This method is called to return the validity interval of the texmap.
Interval Smoke::Validity(TimeValue t) { 
	Interval v;
	// Calling Update() sets texValidity.
	Update(t, v); 
	return texValidity; 
}

// This method is called to reset the texmap back to its default values.
void Smoke::Init() {
	// Reset the XYZGen or allocate a new one
	if (xyzGen) 
		xyzGen->Reset();
	else 
		ReplaceReference(0, GetNewDefaultXYZGen());	

	// Set the inital parameters
	SetColor(0, Color(0.0f, 0.0f, 0.0f), TimeValue(0));
	SetColor(1, Color(0.9f, 0.9f, 0.9f), TimeValue(0));
	SetExp(1.5f, TimeValue(0));
	SetIter(5, TimeValue(0));
	SetSize(40.0f, TimeValue(0));
	SetPhase(0.0f, TimeValue(0));
	seed = 0x8563;

	// Set the validity interval of the texture to empty
	texValidity.SetEmpty();
	}

void Smoke::Reset() {
	smokeCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

Smoke::Smoke() {
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
//	paramDlg = NULL;
	mapOn[0] = mapOn[1] = 1;
	lastpow = -1;
	smokeCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	fileVersion = 0;
	hfboost = 1.2f; // This is currently a constant
	InitVel(seed);
}


// This method gets called when the material or texture is to be displayed 
// in the material editor parameters area. 
ParamDlg* Smoke::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// Allocate a new instance of ParamDlg to manage the UI.  This will
	// create the rollup page in the materials editor.
//	SmokeDlg *smokeDlg = new SmokeDlg(hwMtlEdit, imp, this);
	// Update the dialog display with the proper values of the texture.
//	smokeDlg->LoadDialog();
//	paramDlg = smokeDlg;
//	return smokeDlg;	
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = smokeCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	smoke_param_blk.SetUserDlgProc(new SmokeDlgProc(this));

	return masterDlg;

}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Smoke::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif


BOOL Smoke::SetDlgThing(ParamDlg* dlg)
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
void Smoke::Update(TimeValue t, Interval& ivalid) {		
	if (!texValidity.InInterval(t)) {
		texValidity.SetInfinite();
		xyzGen->Update(t, texValidity);
//		pblock->GetValue(PB_COL1, t, col[0], texValidity);
		pblock->GetValue(smoke_color1, t, col[0], texValidity);
		col[0].ClampMinMax();
//		pblock->GetValue(PB_COL2, t, col[1], texValidity);
		pblock->GetValue(smoke_color2, t, col[1], texValidity);
		col[1].ClampMinMax();
//		pblock->GetValue(PB_SIZE, t, size, texValidity);
		pblock->GetValue(smoke_size, t, size, texValidity);
		ClampFloat(size, MIN_SIZE, MAX_SIZE);
//		pblock->GetValue(PB_EXP, t, power, texValidity);
		pblock->GetValue(smoke_exponent, t, power, texValidity);
		ClampFloat(power, MIN_EXP, MAX_EXP);
//		pblock->GetValue(PB_PHASE, t, phase, texValidity);
//		pblock->GetValue(PB_ITER, t, iter, texValidity);
		pblock->GetValue(smoke_phase, t, phase, texValidity);
		pblock->GetValue(smoke_iteration, t, iter, texValidity);
		ClampInt(iter, (int) MIN_ITER, (int) MAX_ITER);
		pblock->GetValue(smoke_mapon1, t, mapOn[0], texValidity);
		pblock->GetValue(smoke_mapon2, t, mapOn[1], texValidity);
		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t, texValidity);
		}
	}
	ivalid &= texValidity;

}

void Smoke::ClampFloat(float &f, float min, float max) {
	if (f < min) f = min;
	else if (f > max) f = max;
}

void Smoke::ClampInt(int &i, int min, int max) {
	if (i < min) i = min;
	else if (i > max) i = max;
}

// Returns a pointer to the 'i-th' sub-texmap managed by this texture.
Texmap *Smoke::GetSubTexmap(int i) { 
	return subTex[i]; 
}

// Stores the 'i-th' sub-texmap managed by the material or texture.
void Smoke::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2, m);
	if (i==0)
		{
		smoke_param_blk.InvalidateUI(smoke_map1);
		texValidity.SetEmpty();
		}
	else if (i==1)
		{
		smoke_param_blk.InvalidateUI(smoke_map2);
		texValidity.SetEmpty();
		}

//	if (paramDlg)
//		paramDlg->UpdateSubTexNames();
}

// This name appears in the materials editor dialog when editing the
// 'i-th' sub-map.
TSTR Smoke::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_DS_COL1); 
		case 1:  return GetString(IDS_DS_COL2); 
		default: return TSTR(_T(""));
		}
	}

void Smoke::InitVel(int seed) {
	srand(seed);
	for (int i=0; i<MAXNITS; i++) {
		xvel[i] = frand();
		yvel[i] = frand();
		zvel[i] = frand();
		}
	}

float Smoke::SmokeFunc(Point3 p, int iter) {
	float s, mag, ft, r[3], d, hfb;
	int i;
	s = 1.0f;
	mag = 0.0f;
	hfb = 2.0f*1.2f; // *hfboost;
	ft = 1.0f;
	float x = p.x;
	float y = p.y;
	float z = p.z;
	for (i = 0; i < iter; i++) {
		float k = ft*phase;
		r[0] = x + xvel[i]*k;
		r[1] = y + yvel[i]*k;
		r[2] = z + zvel[i]*k;
		mag += (float)fabs(noise3(r))/s;
		x *= 2.0f; 
		y *= 2.0f;	
		z *= 2.0f;
		s *= 2.0f; 
		ft *= hfb; // Make motion a little greater for fine detail
		}
	d = mag;
	if (d>1.0f) return 1.0f;
	return (float )pow(d,power);
	}
	 
// --- Methods inherited from Texmap ---
RGBA Smoke::EvalColor(ShadeContext& sc) {
	float d;
	Point3 p, dp;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	xyzGen->GetXYZ(sc, p, dp);

	if (size == 0.0f) 
		size = 1.0f;

	d = SmokeFunc(p/size, iter);

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

Point3 Smoke::EvalNormalPerturb(ShadeContext& sc) {
	float del, d;
	Point3 p, dp, np;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	xyzGen->GetXYZ(sc, p, dp);

	p /= size;

	del = 0.1f;
	d = SmokeFunc(p, iter);
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
	np.x = (SmokeFunc(p+del*M[0], iter) - d)/del;
	np.y = (SmokeFunc(p+del*M[1], iter) - d)/del;
	np.z = (SmokeFunc(p+del*M[2], iter) - d)/del;
	np = sc.VectorFromNoScale(np,REF_OBJECT);
	Texmap *sub0 = mapOn[0]?subTex[0]:NULL;
	Texmap *sub1 = mapOn[1]?subTex[1]:NULL;
	if (sub0||sub1) {
		// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
		float a,b;
		Point3 da,db;
		if (sub0) { 	
			a = sub0->EvalMono(sc); 	
			da = sub0->EvalNormalPerturb(sc);		
			}
		else {	 
			a = Intens(col[0]);	 
			da = Point3(0.0f,0.0f,0.0f);		 
			}
		if (sub1) {
			b = sub1->EvalMono(sc); 	
			db = sub1->EvalNormalPerturb(sc);	
			}
		else {	 
			b = Intens(col[1]);	 
			db= Point3(0.0f,0.0f,0.0f);		 
			}
		np = (b-a)*np + d*(db-da) + da;
		}
	else 
		np *= Intens(col[1])-Intens(col[0]);
	return np;
}

// --- Methods inherited from Tex3D ---
void Smoke::ReadSXPData(TCHAR *name, void *sxpdata) {
	SmokeState *state = (SmokeState*)sxpdata;
	if (state != NULL && (state->version == SMOKE_SXP_VERSION)) {
		SetColor(0, ColorFromCol24(state->col1), TimeValue(0));
		SetColor(1, ColorFromCol24(state->col2), TimeValue(0));
		//SetSpeed(state->speed, TimeValue(0));
		SetPhase(0.0f, TimeValue(0));
		SetSize(state->size, TimeValue(0));
		SetIter(state->nits, TimeValue(0));
		SetExp(state->power, TimeValue(0));
	}
}

// --- Methods of Smoke ---
void Smoke::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void Smoke::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
//	pblock->SwapControllers(PB_COL1, PB_COL2);
	pblock->SwapControllers(smoke_color1,0,smoke_color2 ,0);
	smoke_param_blk.InvalidateUI(smoke_color1);
	smoke_param_blk.InvalidateUI(smoke_color2);
	smoke_param_blk.InvalidateUI(smoke_map1);
	smoke_param_blk.InvalidateUI(smoke_map2);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
}

void Smoke::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
//	pblock->SetValue((i == 0) ? PB_COL1 : PB_COL2, t, c);
	pblock->SetValue((i == 0) ? smoke_color1 : smoke_color2, t, c);
}

void Smoke::SetPhase(float f, TimeValue t) { 
	phase = f; 
//	pblock->SetValue(PB_PHASE, t, f);
	pblock->SetValue(smoke_phase, t, f);
}

void Smoke::SetSize(float f, TimeValue t) { 
	size = f; 
//	pblock->SetValue(PB_SIZE, t, f);
	pblock->SetValue(smoke_size, t, f);
}

void Smoke::SetExp(float f, TimeValue t) { 
	power = f; 
//	pblock->SetValue(PB_EXP, t, f);
	pblock->SetValue(smoke_exponent, t, f);
}

void Smoke::SetIter(int i, TimeValue t) { 
	iter = i; 
//	pblock->SetValue(PB_ITER, t, i);
	pblock->SetValue(smoke_iteration, t, i);
}

