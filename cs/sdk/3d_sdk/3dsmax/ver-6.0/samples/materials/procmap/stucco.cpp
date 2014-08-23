/*===========================================================================*\
 |    File: Stucco.cpp
 |
 | Purpose: A 3D Map for creating 'stucco' type patterns.
 |          This is a port of the 3D Studio/DOS SXP by Dan Silva.
 |
 | History: Mark Meier, Began 02/05/97.
 |          MM, Last Change 02/05/97.
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

/*===========================================================================*\
 | Miscellaneous Defines
\*===========================================================================*/

#define SHOW_3DMAPS_WITH_2D

// The unique ClassID
static Class_ID stuccoClassID(STUCCO_CLASS_ID, 0);

// This is the number of colors used
#define NUM_COLORS 2

// This is the number of sub-texmaps used
#define NUM_SUB_TEXMAPS 2

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

#define STUCCO_VERS 0xfaaaa3c2

#pragma pack(1)
struct StuccoState {
	ulong version;
	float size;
	float threshold;
	float thickness;
	float del;
	Col24 col1, col2;
};
#pragma pack()

// These are various resource IDs
static int colID[2] = { IDC_COL1, IDC_COL2 };
static int subTexId[NUM_SUB_TEXMAPS] = { IDC_TEX1, IDC_TEX2 };
static int mapOnId[NUM_SUB_TEXMAPS] = { IDC_MAPON1, IDC_MAPON2 };

// Forward references
//class Stucco;
//class StuccoDlgProc;

/*===========================================================================*\
 | Stucco 3D Texture Map Plug-In Class
\*===========================================================================*/
class Stucco : public Tex3D { 
	// This allows the class that manages the UI to access the private 
	// data members of this class.
//	friend class StuccoDlg;

	// These are the current colors from the color swatch controls.
	Color col[NUM_COLORS];

	// These are the parameters managed by the parameter map
	float size;
	float thresh;
	float thick;
	float del;
	Point3 col1, col2;

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
//	StuccoDlg *paramDlg;

#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif

	public:
		static ParamDlg* xyzGenDlg;	
	// This is the parameter block which manages the data for the
	// spinner and color swatch controls.
	// This is reference #1 of this class.
		IParamBlock2 *pblock;
	// Indicates if a sub-texmap is to be used or not
		BOOL mapOn[NUM_SUB_TEXMAPS];

		// --- Methods inherited from Animatable ---
		Class_ID ClassID() { return stuccoClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_STUCCO); }  
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
		void ReadSXPData(TCHAR *name, void *sxpdata);

		// --- Methods inherited from Texmap ---
		XYZGen *GetTheXYZGen() { return xyzGen; }
		RGBA EvalColor(ShadeContext& sc);
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

		// --- Methods of Stucco ---
		Stucco();
		~Stucco() { 
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}

		float Func(Point3 p, float scl);
		float EvalFunc(ShadeContext &sc);
		void SwapInputs(); 
		void NotifyChanged();
		void SetSize(float f, TimeValue t);
		void SetThick(float f, TimeValue t);
		void SetThresh(float f, TimeValue t);
		void SetColor(int i, Color c, TimeValue t);
		void ClampFloat(float &f, float min, float max);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

};

// This is the Class Descriptor for the Stucco 3D Texture plug-in
class StuccoClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
		void *			Create(BOOL loading) { 	return new Stucco; }
		const TCHAR *	ClassName() { return GetString(IDS_DS_STUCCO_CDESC); } // mjm - 2.3.99
		SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
		Class_ID 		ClassID() { return stuccoClassID; }
		const TCHAR* 	Category() { return TEXMAP_CAT_3D; }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
		const TCHAR*	InternalName() { return _T("stucco"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};
static StuccoClassDesc stuccoCD;

ClassDesc *GetStuccoDesc() { return &stuccoCD; }
ParamDlg* Stucco::xyzGenDlg;	

/*===========================================================================*\
 | Class to Manage the User Interface in the Materials Editor
\*===========================================================================*/
/*
class StuccoDlg: public ParamDlg {
	public:
		// This is our UI rollup page window handle in the materials editor
		HWND hParamDlg;
		// Window handle of the materials editor dialog itself
		HWND hMedit;
		// Interface for calling methods provided by MAX
		IMtlParams *ip;
		// The current Stucco being edited.
		Stucco *theTex;
		// Parameter Map for handling UI controls
		IParamMap *pmap;
		// Custom buttons for texture maps
		ICustButton *iCustButton[NUM_SUB_TEXMAPS];
		// Custom conrols for the colors
		IColorSwatch *cs[NUM_COLORS];
		// This is used inside the SetTime method to only update the UI
		// controls when the time slider has changed
		TimeValue curTime; 
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

		// --- StuccoDlg Methods ---
		StuccoDlg(HWND hwMtlEdit, IMtlParams *imp, Stucco *m); 
		~StuccoDlg();
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
#define PB_THRESH	1
#define PB_THICK	2 
#define PB_COL1		3
#define PB_COL2		4
*/

// Spinner limits
#define MIN_SIZE 0.001f
#define MAX_SIZE 999999999.0f
#define MIN_THRESH 0.0f
#define MAX_THRESH 1.0f

#define MIN_THICK 0.0f
#define MAX_THICK 1.0f

// Paramter block version number
#define STUCCO_PB_VERSION 2

enum { stucco_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	stucco_size,stucco_thickness,stucco_threshold,
	stucco_color1, stucco_color2,
	stucco_map1, stucco_map2,
	stucco_mapon1,stucco_mapon2,
	stucco_coords,	  // access for UVW mapping
};

static ParamBlockDesc2 stucco_param_blk ( stucco_params, _T("parameters"),  0, &stuccoCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_STUCCO, IDS_DS_STUCCO_PARAMS, 0, 0, NULL, 
	// params


	stucco_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_SIZE,
		p_default,		20.f,
		p_range,		MIN_SIZE, MAX_SIZE,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZE_EDIT,IDC_SIZE_SPIN, 0.1f, 
		end,
	stucco_thickness,	_T("thickness"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_THICKNESS,
		p_default,		.15f,
		p_range,		MIN_THICK, MAX_THICK,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_THICK_EDIT,IDC_THICK_SPIN, 0.01f, 
		end,
	stucco_threshold,	_T("threshold"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_THRESH,
		p_default,		0.57f,
		p_range,		MIN_THRESH, MAX_THRESH,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_THRESH_EDIT,IDC_THRESH_SPIN, 0.01f, 
		end,
	stucco_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL1,	
		p_default,		Color(0.f, 0.f, 0.0f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL1, 
		end,
	stucco_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL2,	
		p_default,		Color(0.9f, 0.9f, 0.9f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL2, 
		end,
	stucco_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX1,
		end,
	stucco_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX2,
		end,
	stucco_mapon1,	_T("map1On"), TYPE_BOOL,			0,				IDS_PW_MAPON1,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	stucco_mapon2,	_T("map2On"), TYPE_BOOL,			0,				IDS_PW_MAPON2,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	stucco_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDINATES,
		p_refno,		0, 
		end,

	end
);

/*
// Array of parameter descriptors
static ParamUIDesc paramDesc[] = {
	ParamUIDesc(
		PB_SIZE, 
		EDITTYPE_FLOAT, 
		IDC_SIZE_EDIT,IDC_SIZE_SPIN, 
		MIN_SIZE, MAX_SIZE, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_THRESH, 
		EDITTYPE_FLOAT, 
		IDC_THRESH_EDIT,IDC_THRESH_SPIN, 
		MIN_THRESH, MAX_THRESH, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_THICK, 
		EDITTYPE_FLOAT, 
		IDC_THICK_EDIT,IDC_THICK_SPIN, 
		MIN_THICK, MAX_THICK, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(PB_COL1, TYPE_COLORSWATCH, IDC_COL1),
	ParamUIDesc(PB_COL2, TYPE_COLORSWATCH, IDC_COL2)
};
*/
// The number of descriptors in the paramDesc array
#define PARAMDESC_LENGTH 5

// Parameter block parameters	
static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE, stucco_size }, // size 
	{ TYPE_FLOAT, NULL, TRUE, stucco_threshold }, // thresh
	{ TYPE_FLOAT, NULL, TRUE, stucco_thickness }, // thick
	{ TYPE_RGBA,  NULL, TRUE, stucco_color1 }, // color 1
	{ TYPE_RGBA,  NULL, TRUE, stucco_color2 }  // color 2
};
// The number of parameters in the parameter block
#define PB_LENGTH 5


static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,5,1)	// Version 1 params
	};

// The names of the parameters in the parameter block
static int nameIDs[] = {
	IDS_DS_SIZE,IDS_DS_THRESH,IDS_DS_THICKNESS,
	IDS_DS_COL1, IDS_DS_COL2 };
/*
// This is the class that allows the sub-map buttons to be processed.
class StuccoDlgProc : public ParamMapUserDlgProc {
	public:
		StuccoDlg *theDlg;
		StuccoDlgProc(StuccoDlg *s) { theDlg = s; }
		BOOL DlgProc(TimeValue t, IParamMap *map,
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { delete this; }
};

// This is the dialog proc to process the texmap buttons
BOOL StuccoDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	theDlg->isActive = TRUE;
	BOOL res = theDlg->PanelProc(hWnd, msg, wParam, lParam);
	theDlg->isActive = FALSE;
	return res;
}

BOOL StuccoDlg::PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
 | StuccoDlg Methods
\*===========================================================================*/
// --- StuccoDlg Methods ---
// Constructor.
// This is called from within the Stucco::CreateParamDlg method.  That
// method is passed the handle to the materials editor dialog, and an
// interface for calling methods of MAX.  These are passed in here and stored.
/*
StuccoDlg::StuccoDlg(HWND hwMtlEdit, IMtlParams *imp, Stucco *m) { 
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
		theTex->pblock, ip, hInstance, MAKEINTRESOURCE(IDD_STUCCO),
		GetString(IDS_DS_STUCCO_PARAMS), 0);

	// Save the window handle of the rollup page
	hParamDlg = pmap->GetHWnd();

	// Establish the dialog proc to handle the custom button controls
	pmap->SetUserDlgProc(new StuccoDlgProc(this));
}

// Destructor.
// This is called after the user changes to another sample slot in
// the materials editor that does not contain a Stucco texture.
// Note that it is not called if they do go to another Stucco -- in
// that case, the parameters in the rollup page are updated, but
// the entire page is not deleted.  This is accomplished by simply
// changing the parameter block pointer (done inside StuccoDlg::SetThing()).
StuccoDlg::~StuccoDlg() {
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
int StuccoDlg::FindSubTexFromHWND(HWND hw) {
	for (int i=0; i<NUM_SUB_TEXMAPS; i++) {
		if (hw == iCustButton[i]->GetHwnd()) return i;
		}	
	return -1;
	}

// This is called when the dialog is loaded to set the names of the
// textures displayed
void StuccoDlg::UpdateSubTexNames() {
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
void StuccoDlg::LoadDialog() {
	if (theTex) {
		Interval ivalid;
		theTex->Update(curTime, ivalid);

		ISpinnerControl *spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SIZE_SPIN));
		spin->SetValue(theTex->size, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_THRESH_SPIN));
		spin->SetValue(theTex->thresh, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_THICK_SPIN));
		spin->SetValue(theTex->thick, FALSE);
		ReleaseISpinner(spin);

		cs[0]->SetColor(theTex->col[0]);
		cs[1]->SetColor(theTex->col[1]);

		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) 
			SetCheckBox(hParamDlg, mapOnId[i], theTex->mapOn[i]);

		UpdateSubTexNames();
	}
}

// This method invalidates the rollup page so it will get redrawn
void StuccoDlg::Invalidate() { 
	InvalidateRect(hParamDlg, NULL, FALSE); 
	valid = FALSE; 
}

// --- Methods inherited from ParamDlg ---
// Returns the Class_ID of the plug-in this dialog manages
Class_ID StuccoDlg::ClassID() {
	return stuccoClassID; 
}

// This sets the current texture being edited to the texture passed
void StuccoDlg::SetThing(ReferenceTarget *m) {
	assert(m->ClassID() == stuccoClassID);
	assert(m->SuperClassID() == TEXMAP_CLASS_ID);
	if (theTex) 
		theTex->paramDlg = NULL;

	// Set the pointer to the texmap being edited to the one passed.
	theTex = (Stucco *)m;

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
ReferenceTarget *StuccoDlg::GetThing() {
	return (ReferenceTarget *)theTex; 
}

// This method is called when the current time has changed.  
// This gives the developer an opportunity to update any user 
// interface data that may need adjusting due to the change in time.
void StuccoDlg::SetTime(TimeValue t) {
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
void StuccoDlg::ReloadDialog() {
	Interval ivalid;
	theTex->Update(curTime, ivalid);
	LoadDialog();
}

// This method is called when the dialog box becomes active or inactive. 
void StuccoDlg::ActivateDlg(BOOL onOff) {
	for (int i = 0; i < NUM_COLORS; i++) {
		cs[i]->Activate(onOff);
	}
}
*/

//dialog stuff to get the Set Ref button
class StuccoDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Stucco *stucco;		
		StuccoDlgProc(Stucco *m) {stucco = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};



BOOL StuccoDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_SWAP:
					{
					stucco = (Stucco*)map->GetParamBlock()->GetOwner(); 

					stucco->SwapInputs();
					}
				break;
				}
			break;
		}
	return FALSE;
	}

/*===========================================================================*\
 | Stucco Methods
\*===========================================================================*/
// --- Methods inherited from Animatable ---
// This method returns a pointer to the 'i-th' sub-anim.  
Animatable* Stucco::SubAnim(int i) {
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
	}
}

// This method returns the name of the 'i-th' sub-anim to appear in track view. 
TSTR Stucco::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_DS_COORDS);
		case 1: return GetString(IDS_DS_PARAMETERS);
		default: return GetSubTexmapTVName(i-2);
	}
}

// --- Methods inherited from ReferenceMaker ---
// Return the 'i-th' reference
RefTargetHandle Stucco::GetReference(int i) {
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
	}
}

// Save the 'i-th' reference
void Stucco::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
	}
}

// This method is responsible for responding to the change notification
// messages sent by the texmap dependants.
RefResult Stucco::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			texValidity.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				stucco_param_blk.InvalidateUI(changing_param);
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
//				if (paramDlg) {
//					paramDlg->pmap->Invalidate();
//					}
//				}
			break;

/*
		case REFMSG_GET_PARAM_DIM: {
			// This returns the 'dimension' of the parameter.  This is 
			// the type and order of magnitude of the parameter.
			GetParamDim *gpd = (GetParamDim *)partID;
			switch (gpd->index) {
				case PB_SIZE:		gpd->dim =  stdWorldDim; 
					break;
				case PB_THRESH:
				case PB_THICK:
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
#define STUCCO_VERS1_CHUNK		0x4001
#define MAPOFF_CHUNK			0x1000
#define PARAM2_CHUNK			0x1010

// This is called by the system to allow the plug-in to save its data
IOResult Stucco::Save(ISave *isave) { 
	IOResult res;

	// Save the common stuff from the base class
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res != IO_OK) 
		return res;
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
/*	// Save a version number chunk
	isave->BeginChunk(STUCCO_VERS1_CHUNK);
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

class StuccoPostLoad : public PostLoadCallback {
	public:
		Stucco *n;
		BOOL Param1;
		StuccoPostLoad(Stucco *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( stucco_mapon1, 0, n->mapOn[0]);
				n->pblock->SetValue( stucco_mapon2, 0, n->mapOn[1]);
				}
			delete this; 


			} 
	};


// This is called by the system to allow the plug-in to load its data
IOResult Stucco::Load(ILoad *iload) { 
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
			case STUCCO_VERS1_CHUNK:
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
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &stucco_param_blk, this, 1);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new StuccoPostLoad(this,Param1));
	return IO_OK;
}

// --- Methods inherited from ReferenceTarget ---
// This method is called to have the plug-in clone itself.
RefTargetHandle Stucco::Clone(RemapDir &remap) {
	// Create a new instance of the plug-in class
	Stucco *newStucco = new Stucco();

	// Copy superclass stuff
	*((MtlBase *)newStucco) = *((MtlBase *)this);

	// Clone the items we reference
	newStucco->ReplaceReference(0, remap.CloneRef(xyzGen));
	newStucco->ReplaceReference(1, remap.CloneRef(pblock));
	newStucco->col[0] = col[0];
	newStucco->col[1] = col[1];
	newStucco->size = size;
	newStucco->thresh = thresh;
	newStucco->thick = thick;
	newStucco->texValidity.SetEmpty();	
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		newStucco->subTex[i] = NULL;
		newStucco->mapOn[i] = mapOn[i];
		if (subTex[i])
			newStucco->ReplaceReference(i+2, remap.CloneRef(subTex[i]));
		}
	BaseClone(this, newStucco, remap);
	// Return the new cloned texture
	return (RefTargetHandle)newStucco;
	}

// --- Methods inherited from MtlBase ---
// This method is called to return the validity interval of the texmap.
Interval Stucco::Validity(TimeValue t) { 
	Interval v;
	// Calling Update() sets texValidity.
	Update(t, v); 
	return texValidity; 
	}

// This method is called to reset the texmap back to its default values.
void Stucco::Init() {
	// Reset the XYZGen or allocate a new one
	if (xyzGen) 
		xyzGen->Reset();
	else 
		ReplaceReference(0, GetNewDefaultXYZGen());	

	// Set the inital parameters
	SetColor(0, Color(0.0f, 0.0f, 0.0f), TimeValue(0));
	SetColor(1, Color(0.9f, 0.9f, 0.9f), TimeValue(0));
	SetSize(20.0f, TimeValue(0));
	SetThresh(0.57f, TimeValue(0));
	SetThick(0.15f, TimeValue(0));

	// Set the validity interval of the texture to empty
	texValidity.SetEmpty();
	}

void Stucco::Reset() {
	stuccoCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

Stucco::Stucco() {
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
//	paramDlg = NULL;
	mapOn[0] = mapOn[1] = 1;
	stuccoCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	fileVersion = 0;
	del = 0.1f; // This is a constant for now...
	}


#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Stucco::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif

// This method gets called when the material or texture is to be displayed 
// in the material editor parameters area. 
ParamDlg* Stucco::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// Allocate a new instance of ParamDlg to manage the UI.  This will
	// create the rollup page in the materials editor.
//	StuccoDlg *stuccoDlg = new StuccoDlg(hwMtlEdit, imp, this);
	// Update the dialog display with the proper values of the texture.
//	stuccoDlg->LoadDialog();
//	paramDlg = stuccoDlg;
//	return stuccoDlg;	
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = stuccoCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	stucco_param_blk.SetUserDlgProc(new StuccoDlgProc(this));

	return masterDlg;
	

}

static Color ColrFromCol24(Col24 a) {
	Color c;
	c.r = (float)a.r/255.0f;
	c.g = (float)a.g/255.0f;
	c.b = (float)a.b/255.0f;
	return c;
	}


void Stucco::ReadSXPData(TCHAR *name, void *sxpdata) {
	StuccoState *state = (StuccoState*)sxpdata;
	if (state!=NULL && (state->version==STUCCO_VERS)) {
		SetColor(0, ColrFromCol24(state->col1),0);
		SetColor(1, ColrFromCol24(state->col2),0);
		SetSize(state->size,0);
		SetThick(state->thickness,0);
		SetThresh(state->threshold,0);
		del = state->del;
		}
	}

BOOL Stucco::SetDlgThing(ParamDlg* dlg)
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
void Stucco::Update(TimeValue t, Interval& ivalid) {		
	if (!texValidity.InInterval(t)) {
		texValidity.SetInfinite();
		xyzGen->Update(t, texValidity);
//		pblock->GetValue(PB_COL1, t, col[0], texValidity);
		pblock->GetValue(stucco_color1, t, col[0], texValidity);
		col[0].ClampMinMax();
//		pblock->GetValue(PB_COL2, t, col[1], texValidity);
		pblock->GetValue(stucco_color2, t, col[1], texValidity);
		col[1].ClampMinMax();
//		pblock->GetValue(PB_SIZE, t, size, texValidity);
		pblock->GetValue(stucco_size, t, size, texValidity);
		ClampFloat(size, MIN_SIZE, MAX_SIZE);
//		pblock->GetValue(PB_THRESH, t, thresh, texValidity);
		pblock->GetValue(stucco_threshold, t, thresh, texValidity);
		ClampFloat(thresh, MIN_THRESH, MAX_THRESH);
//		pblock->GetValue(PB_THICK, t, thick, texValidity);
		pblock->GetValue(stucco_thickness, t, thick, texValidity);
		pblock->GetValue(stucco_mapon1, t, mapOn[0], texValidity);
		pblock->GetValue(stucco_mapon2, t, mapOn[1], texValidity);
		ClampFloat(thick, MIN_THICK, MAX_THICK);
		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t, texValidity);
		}
	}
	ivalid &= texValidity;
}

void Stucco::ClampFloat(float &f, float min, float max) {
	if (f < min) f = min;
	else if (f > max) f = max;
}

// Returns a pointer to the 'i-th' sub-texmap managed by this texture.
Texmap *Stucco::GetSubTexmap(int i) { 
	return subTex[i]; 
}

// Stores the 'i-th' sub-texmap managed by the material or texture.
void Stucco::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2, m);
	if (i==0)
		{
		stucco_param_blk.InvalidateUI(stucco_map1);
		texValidity.SetEmpty();
		}
	else if (i==1)
		{
		stucco_param_blk.InvalidateUI(stucco_map2);
		texValidity.SetEmpty();
		}

//	if (paramDlg)
//		paramDlg->UpdateSubTexNames();
}

// This name appears in the materials editor dialog when editing the
// 'i-th' sub-map.
TSTR Stucco::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_DS_COL1); 
		case 1:  return GetString(IDS_DS_COL2); 
		default: return TSTR(_T(""));
	}
}
	 

static float compscl(Point3 dp, float size) {
	float f;
	float scl = (float)fabs(dp.x);
	if ((f = (float)fabs(dp.y)) > scl) 
		scl = f;
	if ((f = (float)fabs(dp.z)) > scl) 
		scl = f;
	scl /= size;
	return scl;
	}

#define CRV  (.2)
#define K   (.5/(1-CRV))
#define K1  (K/CRV)

/*
STUCCO function

works off a noise function 'f'in range [0..1.0]

when f<=threshold returns 0.0;

state.thickness is the fraction of the remaining interval [thresh..1.0] 
taken up by a transition from 0 to 1.0;
Over this interval, a 2nd order curve (piecewise parabolic) is used.
over the first CRV part of the transition its parablolic, then for
the next 1-2*CRV part its linear, then for the last CRV part it's
an inverted parabola.
-------------------------------------------------------
*/
float Stucco::Func(Point3 p, float scl) {
	float f,t;
	f = 0.5f*(noise3(p)+1.0f); /* get number from  0 to 1.0 */

	if (f <= thresh) 
		return(float) 0.0f;
	t = thick+0.5f*scl;
	f = (f-thresh)/t;
	if (f >= 1.0f) 
		return (float) (1.0f);
	if (f < CRV) 	{
		return (float) (K1*f*f);
		}
	else 
	if (f < (1.0f-CRV)) {
	   return (float) (K*((2.0f*f)-CRV));
		}
	else {
		f = 1.0f-f;
		return (float) (1.0f - K1*(f*f));
		}
	}

float Stucco::EvalFunc(ShadeContext &sc) {
	Point3 p, dp;
	xyzGen->GetXYZ(sc, p, dp);
	if (size == 0.0f) 
		size = 0.0001f;
	p /= size;
	float scl = compscl(dp, size);
	return Func(p, scl);
	}

// --- Methods inherited from Texmap ---
RGBA Stucco::EvalColor(ShadeContext& sc) {
	float f;
	Point3 p, dp;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	xyzGen->GetXYZ(sc, p, dp);

	if (size == 0.0f) 
		size = 0.0001f;
	p /= size;

	float scl = compscl(dp, size);
	f = Func(p, scl);

	// If we have sub-texmaps and they are enabled, get the colors from 
	// the sub-texmaps, otherwise get them from the color swatch
	RGBA c0 = (mapOn[0]&&subTex[0]) ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = (mapOn[1]&&subTex[1]) ? subTex[1]->EvalColor(sc): col[1];

	Col24 c;
	Col24 col1 = Col24FromColor(c0);
	Col24 col2 = Col24FromColor(c1);

	lerp_color(&c, &col1, &col2, f);
	return ColorFromCol24(c);
}

Point3 Stucco::EvalNormalPerturb(ShadeContext& sc) {
	float d,k;
	Point3 p, dp, np;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	xyzGen->GetXYZ(sc, p, dp);

	float scl = compscl(dp, size);
	p /= size;
	d = Func(p, scl);
	k = 0.25f/del;

	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
	np.x = (Func(p + del*M[0], scl) - d)*k;
	np.y = (Func(p + del*M[1], scl) - d)*k;
	np.z = (Func(p + del*M[2], scl) - d)*k;
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

// --- Methods of Stucco ---
void Stucco::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void Stucco::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
//	pblock->SwapControllers(PB_COL1, PB_COL2);
	pblock->SwapControllers(stucco_color1,0, stucco_color2,0);
	stucco_param_blk.InvalidateUI(stucco_color1);
	stucco_param_blk.InvalidateUI(stucco_color2);
	stucco_param_blk.InvalidateUI(stucco_map1);
	stucco_param_blk.InvalidateUI(stucco_map2);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
}

void Stucco::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
//	pblock->SetValue((i == 0) ? PB_COL1 : PB_COL2, t, c);
	pblock->SetValue((i == 0) ? stucco_color1 : stucco_color2, t, c);
}

void Stucco::SetThick(float f, TimeValue t) { 
	thick = f; 
//	pblock->SetValue(PB_THICK, t, f);
	pblock->SetValue(stucco_thickness, t, f);
}

void Stucco::SetThresh(float f, TimeValue t) { 
	thresh = f; 
//	pblock->SetValue(PB_THRESH, t, f);
	pblock->SetValue(stucco_threshold, t, f);
}

void Stucco::SetSize(float f, TimeValue t) { 
	size = f; 
//	pblock->SetValue(PB_SIZE, t, f);
	pblock->SetValue(stucco_size, t, f);
}

