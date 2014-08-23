/*===========================================================================*\
 |    File: Speckle.cpp
 |
 | Purpose: A 3D Map for creating speckled surface textures.
 |          This is a port of the 3D Studio/DOS SXP by Dan Silva.
 |
 | History: Mark Meier, Began 01/07/97.
 |          MM, Updated to used Parameter Maps on 01/29/97.
 |          MM, Last Change 01/30/97.
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
// The unique ClassID
static Class_ID speckleClassID(SPECKLE_CLASS_ID, 0);

#define DEFAULT_SPECKLE_SIZE	0.1f
#define DEFAULT_COLOR1			Color(0.0f, 0.0f, 0.0f)
#define DEFAULT_COLOR2			Color(1.0f, 1.0f, 1.0f)
//#define DEFAULT_SPECKLE_SIZE	60.f
//#define DEFAULT_COLOR1			Color(0.2f, 0.5f, 1.0f)
//#define DEFAULT_COLOR2			Color(0.7f, 0.8f, 0.8f)

// This is the number of colors used
#define NUM_COLORS 2

// This is the number of sub-texmaps used
#define NUM_SUB_TEXMAPS 2

// This is a scale factor applied to the values computed by the 
// function that does the speckling
#define SCALE_FACTOR 10.0f

// This is the version number of the IPAS SXP files that can be read
#define SPECKLE_SXP_VERSION 0xE001

#define SHOW_3DMAPS_WITH_2D

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

#pragma pack(1)
struct SpeckleState {
	ulong version;
	float size;
	Col24 col1, col2;
};
#pragma pack()

// These are various resource IDs
static int colID[2] = { IDC_COL1, IDC_COL2 };
//static int subTexId[NUM_SUB_TEXMAPS] = { IDC_TEX1, IDC_TEX2 };
//static int mapOnId[NUM_SUB_TEXMAPS] = { IDC_MAPON1, IDC_MAPON2 };

// Forward references
//class Speckle;
//class SpeckleDlgProc;

/*===========================================================================*\
 | Speckle 3D Texture Map Plug-In Class
\*===========================================================================*/
class Speckle : public Tex3D { 
	// This allows the class that manages the UI to access the private 
	// data members of this class.
//	friend class SpeckleDlg;

	// This is the function that actually computes the speckling.
	float SpeckleFunc(Point3 p);

	// These are the current colors from the color swatch controls.
	Color col[NUM_COLORS];
	// This is the size parameter managed by the parameter map
	float size;
	// Color values manipulated by the parameter map
	Point3 col1, col2;
	// This points to the XYZGen instance used to handle the 
	// 'Coordinates' rollup in the materials editor.
	// This is reference #0 of this class.
	XYZGen *xyzGen;
	// This is the parameter block which manages the data for the
	// spinner and color swatch controls.
	// This is reference #1 of this class.
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
//	SpeckleDlg *paramDlg;

#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif
	public:
		static ParamDlg* xyzGenDlg;	
		BOOL mapOn[NUM_SUB_TEXMAPS];
		IParamBlock2 *pblock;
		// --- Methods inherited from Animatable ---
		Class_ID ClassID() { return speckleClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s =  GetString(IDS_DS_SPECKLE); }  
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

		// --- Methods of Speckle ---
		Speckle();
		~Speckle() {
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}

		void SwapInputs(); 
		void NotifyChanged();
		void SetSize(float f, TimeValue t);
		void SetColor(int i, Color c, TimeValue t);
		void ClampFloat(float &f, float min, float max);

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

// This is the Class Descriptor for the Speckle 3D Texture plug-in
class SpeckleClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return 1; }
		void *			Create(BOOL loading) { 	return new Speckle; }
		const TCHAR *	ClassName() { return GetString(IDS_DS_SPECKLE_CDESC); } // mjm - 2.3.99
		SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
		Class_ID 		ClassID() { return speckleClassID; }
		const TCHAR* 	Category() { return TEXMAP_CAT_3D; }

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
		const TCHAR*	InternalName() { return _T("speckle"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};
static SpeckleClassDesc speckleCD;
ClassDesc *GetSpeckleDesc() { return &speckleCD; }
ParamDlg* Speckle::xyzGenDlg;	

/*===========================================================================*\
 | Class to Manage the User Interface in the Materials Editor
\*===========================================================================*/
/*
class SpeckleDlg: public ParamDlg {
	public:
		// This is our UI rollup page window handle in the materials editor
		HWND hParamDlg;
		// Window handle of the materials editor dialog itself
		HWND hMedit;
		// Interface for calling methods provided by MAX
		IMtlParams *ip;
		// The current Speckle being edited.
		Speckle *theTex;
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

		// --- SpeckleDlg Methods ---
		SpeckleDlg(HWND hwMtlEdit, IMtlParams *imp, Speckle *m); 
		~SpeckleDlg();
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		void UpdateSubTexNames();
		void LoadDialog();
		void UpdateMtlDisplay() { ip->MtlChanged(); }
		void Invalidate();
};
*/
/*===========================================================================*\
 | Parameter Map Related Data and Methods
\*===========================================================================*/
// Parameter block indices
/*
#define PB_SIZE 0
#define PB_COL1 1
#define PB_COL2 2
*/
// Spinner limits
#define MIN_SIZE 0.001f
#define MAX_SIZE 999999999.0f

// Paramter block version number
#define SPECKLE_PB_VERSION 2

enum { speckle_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	speckle_size,
	speckle_color1, speckle_color2,
	speckle_map1, speckle_map2,
	speckle_mapon1,speckle_mapon2,
	speckle_coords,	  // access for UVW mapping
};

static ParamBlockDesc2 speckle_param_blk ( speckle_params, _T("parameters"),  0, &speckleCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_SPECKLE, IDS_DS_SPECKLE_PARAMS, 0, 0, NULL, 
	// params


	speckle_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_SIZE,
		p_default,		DEFAULT_SPECKLE_SIZE,
		p_range,		MIN_SIZE, MAX_SIZE,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZE_EDIT,IDC_SIZE_SPIN, 0.1f, 
		end,
	speckle_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL1,	
		p_default,		DEFAULT_COLOR1, 
		p_ui,			TYPE_COLORSWATCH, IDC_COL1, 
		end,
	speckle_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL2,	
		p_default,		DEFAULT_COLOR2, 
		p_ui,			TYPE_COLORSWATCH, IDC_COL2, 
		end,
	speckle_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX1,
		end,
	speckle_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX2,
		end,
	speckle_mapon1,	_T("map1On"), TYPE_BOOL,			0,				IDS_PW_MAPON1,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	speckle_mapon2,	_T("map2On"), TYPE_BOOL,			0,				IDS_PW_MAPON2,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	speckle_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDINATES,
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

	ParamUIDesc(PB_COL1, TYPE_COLORSWATCH, IDC_COL1),
	ParamUIDesc(PB_COL2, TYPE_COLORSWATCH, IDC_COL2)
};
*/
// The number of descriptors in the paramDesc array
#define PARAMDESC_LENGTH 3

// Parameter block parameters	
static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE, speckle_size }, // size 
	{ TYPE_RGBA,  NULL, TRUE, speckle_color1 }, // color 1
	{ TYPE_RGBA,  NULL, TRUE, speckle_color2 }  // color 2
};
// The number of parameters in the parameter block
#define PB_LENGTH 3

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,3,1)	// Version 1 params
	};

// The names of the parameters in the parameter block
static int nameIDs[] = { IDS_DS_SIZE, IDS_DS_COL1, IDS_DS_COL2 };

/*
// This is the class that allows the sub-map buttons to be processed.
class SpeckleDlgProc : public ParamMapUserDlgProc {
	public:
		SpeckleDlg *theDlg;
		SpeckleDlgProc(SpeckleDlg *s) { theDlg = s; }
		BOOL DlgProc(TimeValue t, IParamMap *map,
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { delete this; }
};

// This is the dialog proc to process the texmap buttons
BOOL SpeckleDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	theDlg->isActive = TRUE;
	BOOL res = theDlg->PanelProc(hWnd, msg, wParam, lParam);
	theDlg->isActive = FALSE;
	return res;
}

BOOL SpeckleDlg::PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
 | SpeckleDlg Methods
\*===========================================================================*/
// --- SpeckleDlg Methods ---
// Constructor.
// This is called from within the Speckle::CreateParamDlg method.  That
// method is passed the handle to the materials editor dialog, and an
// interface for calling methods of MAX.  These are passed in here and stored.
/*
SpeckleDlg::SpeckleDlg(HWND hwMtlEdit, IMtlParams *imp, Speckle *m) { 
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
		theTex->pblock, ip, hInstance, MAKEINTRESOURCE(IDD_SPECKLE),
		GetString(IDS_DS_SPECKLE_PARAMS), 0);

	// Save the window handle of the rollup page
	hParamDlg = pmap->GetHWnd();

	// Establish the dialog proc to handle the custom button controls
	pmap->SetUserDlgProc(new SpeckleDlgProc(this));
}

// Destructor.
// This is called after the user changes to another sample slot in
// the materials editor that does not contain a Speckle texture.
// Note that it is not called if they do go to another Speckle -- in
// that case, the parameters in the rollup page are updated, but
// the entire page is not deleted.  This is accomplished by simply
// changing the parameter block pointer (done inside SpeckleDlg::SetThing()).
SpeckleDlg::~SpeckleDlg() {
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
int SpeckleDlg::FindSubTexFromHWND(HWND hw) {
	for (int i=0; i<NUM_SUB_TEXMAPS; i++) {
		if (hw == iCustButton[i]->GetHwnd()) return i;
		}	
	return -1;
	}


// This is called when the dialog is loaded to set the names of the
// textures displayed
void SpeckleDlg::UpdateSubTexNames() {
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
void SpeckleDlg::LoadDialog() {
	if (theTex) {
		Interval ivalid;
		theTex->Update(curTime, ivalid);

		ISpinnerControl *spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SIZE_SPIN));
		spin->SetValue(theTex->size, FALSE);
		ReleaseISpinner(spin);

		cs[0]->SetColor(theTex->col[0]);
		cs[1]->SetColor(theTex->col[1]);

		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) 
			SetCheckBox(hParamDlg, mapOnId[i], theTex->mapOn[i]);

		UpdateSubTexNames();
	}
}

// This method invalidates the rollup page so it will get redrawn
void SpeckleDlg::Invalidate() { 
	InvalidateRect(hParamDlg, NULL, FALSE); 
	valid = FALSE; 
}

// --- Methods inherited from ParamDlg ---
// Returns the Class_ID of the plug-in this dialog manages
Class_ID SpeckleDlg::ClassID() {
	return speckleClassID; 
}

// This sets the current texture being edited to the texture passed
void SpeckleDlg::SetThing(ReferenceTarget *m) {
	assert(m->ClassID() == speckleClassID);
	assert(m->SuperClassID() == TEXMAP_CLASS_ID);
	if (theTex) 
		theTex->paramDlg = NULL;

	// Set the pointer to the texmap being edited to the one passed.
	theTex = (Speckle *)m;

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
ReferenceTarget *SpeckleDlg::GetThing() {
	return (ReferenceTarget *)theTex; 
}

// This method is called when the current time has changed.  
// This gives the developer an opportunity to update any user 
// interface data that may need adjusting due to the change in time.
void SpeckleDlg::SetTime(TimeValue t) {
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
void SpeckleDlg::ReloadDialog() {
	Interval ivalid;
	theTex->Update(curTime, ivalid);
	LoadDialog();
}

// This method is called when the dialog box becomes active or inactive. 
void SpeckleDlg::ActivateDlg(BOOL onOff) {
	for (int i = 0; i < NUM_COLORS; i++) {
		cs[i]->Activate(onOff);
	}
}
*/
//dialog stuff to get the Set Ref button
class SpeckleDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Speckle *speckle;		
		SpeckleDlgProc(Speckle *m) {speckle = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			speckle = (Speckle*)m;
			}

	};



BOOL SpeckleDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_SWAP:
					{
					speckle = (Speckle*)map->GetParamBlock()->GetOwner(); 

					speckle->SwapInputs();
					}
				break;
				}
			break;
		}
	return FALSE;
	}


/*===========================================================================*\
 | Speckle Methods
\*===========================================================================*/
// --- Methods inherited from Animatable ---
// This method returns a pointer to the 'i-th' sub-anim.  
Animatable* Speckle::SubAnim(int i) {
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
	}
}

// This method returns the name of the 'i-th' sub-anim to appear in track view. 
TSTR Speckle::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_DS_COORDS);
		case 1: return GetString(IDS_DS_PARAMETERS);
		default: return GetSubTexmapTVName(i-2);
	}
}

// --- Methods inherited from ReferenceMaker ---
// Return the 'i-th' reference
RefTargetHandle Speckle::GetReference(int i) {
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
	}
}

// Save the 'i-th' reference
void Speckle::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
	}
}

// This method is responsible for responding to the change notification
// messages sent by the texmap dependants.
RefResult Speckle::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			texValidity.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				speckle_param_blk.InvalidateUI(changing_param);
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
#define SPECKLE_VERS1_CHUNK		0x4001
#define MAPOFF_CHUNK			0x1000
#define PARAM2_CHUNK			0x1010

// This is called by the system to allow the plug-in to save its data
IOResult Speckle::Save(ISave *isave) { 
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
	isave->BeginChunk(SPECKLE_VERS1_CHUNK);
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


class SpecklePostLoad : public PostLoadCallback {
	public:
		Speckle *n;
		BOOL Param1;
		SpecklePostLoad(Speckle *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( speckle_mapon1, 0, n->mapOn[0]);
				n->pblock->SetValue( speckle_mapon2, 0, n->mapOn[1]);
				}
			delete this; 


			} 
	};



// This is called by the system to allow the plug-in to load its data
IOResult Speckle::Load(ILoad *iload) { 
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
			case SPECKLE_VERS1_CHUNK:
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
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &speckle_param_blk, this, 1);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new SpecklePostLoad(this,Param1));

	return IO_OK;
}

// --- Methods inherited from ReferenceTarget ---
// This method is called to have the plug-in clone itself.
RefTargetHandle Speckle::Clone(RemapDir &remap) {
	// Create a new instance of the plug-in class
	Speckle *newSpeckle = new Speckle();

	// Copy the superclass stuff
	*((MtlBase *)newSpeckle) = *((MtlBase *)this);

	// Clone the items we reference
	newSpeckle->ReplaceReference(0,remap.CloneRef(xyzGen));
	newSpeckle->ReplaceReference(1,remap.CloneRef(pblock));
	newSpeckle->col[0] = col[0];
	newSpeckle->col[1] = col[1];
	newSpeckle->size = size;
	newSpeckle->texValidity.SetEmpty();	
	for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
		newSpeckle->subTex[i] = NULL;
		newSpeckle->mapOn[i] = mapOn[i];
		if (subTex[i])
			newSpeckle->ReplaceReference(i+2, remap.CloneRef(subTex[i]));
	}
	BaseClone(this, newSpeckle, remap);
	// Return the new cloned texture
	return (RefTargetHandle)newSpeckle;
}

// --- Methods inherited from MtlBase ---
// This method is called to return the validity interval of the texmap.
Interval Speckle::Validity(TimeValue t) { 
	Interval v;
	// Calling Update() sets texValidity.
	Update(t, v); 
	return texValidity; 
}

// This method is called to reset the texmap back to its default values.
void Speckle::Init() {
	// Reset the XYZGen or allocate a new one
	if (xyzGen) 
		xyzGen->Reset();
	else 
		ReplaceReference(0, GetNewDefaultXYZGen());	

	// Set the inital colors and size
	SetColor(0, DEFAULT_COLOR1, TimeValue(0));
	SetColor(1, DEFAULT_COLOR2, TimeValue(0));
	SetSize(DEFAULT_SPECKLE_SIZE, TimeValue(0));
	// Set the validity interval of the texture to empty
	texValidity.SetEmpty();
}

void Speckle::Reset() {
	speckleCD.Reset(this, TRUE);	// reset all pb2's
	// Delete the references to the two sub-texture maps
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

Speckle::Speckle() {
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
//	paramDlg = NULL;
	mapOn[0] = mapOn[1] = 1;
	speckleCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	fileVersion = 0;
}

// This method gets called when the material or texture is to be displayed 
// in the material editor parameters area. 
ParamDlg* Speckle::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// Allocate a new instance of ParamDlg to manage the UI.  This will
	// create the rollup page in the materials editor.
//	SpeckleDlg *speckleDlg = new SpeckleDlg(hwMtlEdit, imp, this);
	// Update the dialog display with the proper values of the texture.
//	speckleDlg->LoadDialog();
//	paramDlg = speckleDlg;
//	return speckleDlg;
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = speckleCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	speckle_param_blk.SetUserDlgProc(new SpeckleDlgProc(this));

	return masterDlg;
	
}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Speckle::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif


// This method is called before rendering begins to allow the plug-in 
// to evaluate anything prior to the render so it can store this information.
void Speckle::Update(TimeValue t, Interval& ivalid) {		
	if (!texValidity.InInterval(t)) {
		texValidity.SetInfinite();
		xyzGen->Update(t, texValidity);
//		pblock->GetValue(PB_COL1, t, col[0], texValidity);
		pblock->GetValue(speckle_color1, t, col[0], texValidity);
		col[0].ClampMinMax();
//		pblock->GetValue(PB_COL2, t, col[1], texValidity);
		pblock->GetValue(speckle_color2, t, col[1], texValidity);
		col[1].ClampMinMax();
//		pblock->GetValue(PB_SIZE, t, size, texValidity);
		pblock->GetValue(speckle_size, t, size, texValidity);
		pblock->GetValue(speckle_mapon1, t, mapOn[0], texValidity);
		pblock->GetValue(speckle_mapon2, t, mapOn[1], texValidity);
		ClampFloat(size, MIN_SIZE, MAX_SIZE); 
		for (int i = 0; i < NUM_SUB_TEXMAPS; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t, texValidity);
		}
	}
	ivalid &= texValidity;
}

BOOL Speckle::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((xyzGenDlg!= NULL) && (dlg == xyzGenDlg))
		xyzGenDlg->SetThing(xyzGen);
	else 
		return FALSE;
	return TRUE;
}


void Speckle::ClampFloat(float &f, float min, float max) {
	if (f < min) f = min;
	else if (f > max) f = max;
}

// Returns a pointer to the 'i-th' sub-texmap managed by this texture.
Texmap *Speckle::GetSubTexmap(int i) { 
	return subTex[i]; 
}

// Stores the 'i-th' sub-texmap managed by the material or texture.
void Speckle::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2, m);
	if (i==0)
		{
		speckle_param_blk.InvalidateUI(speckle_map1);
		texValidity.SetEmpty();
		}
	else if (i==1)
		{
		speckle_param_blk.InvalidateUI(speckle_map2);
		texValidity.SetEmpty();
		}

//	if (paramDlg)
//		paramDlg->UpdateSubTexNames();
}

// This name appears in the materials editor dialog when editing the
// 'i-th' sub-map.
TSTR Speckle::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_DS_COL1); 
		case 1:  return GetString(IDS_DS_COL2); 
		default: return TSTR(_T(""));
	}
}
	 
// --- Methods inherited from Texmap ---
RGBA Speckle::EvalColor(ShadeContext& sc) {
	// After being evaluated, if a map or material has a non-zero gbufID, 
	// it should call ShadeContext::SetGBuffer() to store it into 
	// the shade context.
	if (gbufID) 
		sc.SetGBufferID(gbufID);

	// Use the XYZGen instance to get a transformed point from the
	// ShadeContext.
	Point3 p, dp;
	xyzGen->GetXYZ(sc, p, dp);
	if (size == 0.0f) 
		size = 0.0001f;
	p *= SCALE_FACTOR/size;

	float d = SpeckleFunc(p);
	if (d>1.0f) d = 1.0f;

	// If we have sub-texmaps and they are enabled, get the colors from 
	// the sub-texmaps, otherwise get them from the color swatch
	RGBA c0 = (mapOn[0]&&subTex[0]) ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = (mapOn[1]&&subTex[1]) ? subTex[1]->EvalColor(sc): col[1];

	// Composite the colors together and return the result.
	return (1.0f-d)*c0 + d*c1;
}

Point3 Speckle::EvalNormalPerturb(ShadeContext& sc) {
	float del, d;
	Point3 p, dp;
	Point3 np;

	if (gbufID) 
		sc.SetGBufferID(gbufID);

	xyzGen->GetXYZ(sc, p, dp);
	if (size == 0.0f) 
		size = 0.0001f;
	p *= SCALE_FACTOR/size;

	del = 0.1f;
	d = SpeckleFunc(p);
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
    np.x = (SpeckleFunc(p+del*M[0]) - d)/del;
	np.y = (SpeckleFunc(p+del*M[1]) - d)/del;
	np.z = (SpeckleFunc(p+del*M[2]) - d)/del;

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
void Speckle::ReadSXPData(TCHAR *name, void *sxpdata) {
	SpeckleState *state = (SpeckleState*)sxpdata;
	if (state != NULL && (state->version == SPECKLE_SXP_VERSION)) {
		SetColor(0, ColorFromCol24(state->col1), TimeValue(0));
		SetColor(1, ColorFromCol24(state->col2), TimeValue(0));
		SetSize(state->size, TimeValue(0));
	}
}

// --- Methods of Speckle ---

float Speckle::SpeckleFunc(Point3 p) {
	float sum, s, q[3];

	s = 1.0f;
	sum = 0.0f;
	q[0] = p.x; q[1] = p.y; q[2] = p.z;
	for (int i = 0; i < 6; i++) {
		sum += NOISE(q)/s;
		s *= 2.0f;
		q[0] *= 2.0f; q[1] *= 2.0f; q[2] *= 2.0f;
	}
	return(sum);
}

void Speckle::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void Speckle::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
//	pblock->SwapControllers(PB_COL1, PB_COL2);
	pblock->SwapControllers(speckle_color1,0, speckle_color2,0);
	speckle_param_blk.InvalidateUI(speckle_color1);
	speckle_param_blk.InvalidateUI(speckle_color2);
	speckle_param_blk.InvalidateUI(speckle_map1);
	speckle_param_blk.InvalidateUI(speckle_map2);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
}

void Speckle::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
//	pblock->SetValue((i == 0) ? PB_COL1 : PB_COL2, t, c);
	pblock->SetValue((i == 0) ? speckle_color1 : speckle_color2, t, c);
}

void Speckle::SetSize(float f, TimeValue t) { 
	size = f; 
//	pblock->SetValue(PB_SIZE, t, f);
	pblock->SetValue(speckle_size, t, f);
}

