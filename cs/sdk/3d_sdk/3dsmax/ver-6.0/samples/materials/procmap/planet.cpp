/*===========================================================================*\
 |    File: Planet.cpp
 |
 | Purpose: A 3D Map for creating planet surface textures.
 |          This is a port of the 3D Studio/DOS SXP by Tom Hudson.
 |
 | History: Mark Meier, Began 02/03/97.
 |          MM, Updated for R3 11/11/98.
 		    Updated to Param Block2 by Peter Watje 12/1/1998
\*===========================================================================*/
/*===========================================================================*\
 | Include Files
\*===========================================================================*/
#include "procmaps.h"
#include "iparamm2.h"
#include "resource.h"
#include "resourceOverride.h"

/*===========================================================================*\
 | Miscellaneous Defines
\*===========================================================================*/

#define SHOW_3DMAPS_WITH_2D

// The unique ClassID
static Class_ID planetClassID(PLANET_CLASS_ID, 0);

// This is the number of colors used
#define NUM_COLORS 8

// This is the version number of the IPAS SXP files that can be read
#define PLANET_SXP_VERSION 0x8273F5EC

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
struct PlanetState {
	ulong version;
	float size;
	float island;
	float percent;
	int blend,seed;
	char file[13],filler[3];
	Col24 palette[8];
	float land;
};
#pragma pack()

// These are various resource IDs
static int colID[NUM_COLORS] = { IDC_COL1, IDC_COL2, IDC_COL3, IDC_COL4,
	IDC_COL5, IDC_COL6, IDC_COL7, IDC_COL8 };

// Forward references
class Planet;
//class PlanetDlgProc;

/*===========================================================================*\
 | Planet 3D Texture Map Plug-In Class
\*===========================================================================*/
class Planet : public Tex3D { 
	// This allows the class that manages the UI to access the private 
	// data members of this class.
//	friend class PlanetDlg;

	// These are the current colors from the color swatch controls.
	Color col[NUM_COLORS];

	// These are the parameters managed by the parameter map
	float size;
	float land;
	float island;
	float percent;
	Point3 col1, col2, col3, col4, col5, col6, col7, col8;

	// This points to the XYZGen instance used to handle the 
	// 'Coordinates' rollup in the materials editor.
	// This is reference #0 of this class.
	XYZGen *xyzGen;
	// This is the parameter block which manages the data for the
	// spinner and color swatch controls.
	// This is reference #1 of this class.
	// This holds the validity interval of the texmap.
	Interval texValidity;
	// This is the version of the texture loaded from disk.
	int fileVersion;
	// This points to the ParamDlg instance used to manage the UI
//	PlanetDlg *paramDlg;
	
#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif
	public:
		// --- Methods inherited from Animatable ---
		int blend, seed;
		static ParamDlg* xyzGenDlg;	
		IParamBlock2 *pblock;
		Class_ID ClassID() { return planetClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_PLANET); }  
		void DeleteThis() { delete this; }	

		// We have 2 sub-animatables.  These are the xyzGen, 
		// and the pblock
		int NumSubs() { return 2; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// --- Methods inherited from ReferenceMaker ---
		// We have 2 references.  These are the xyzGen, 
		// and the pblock
 		int NumRefs() { return 2; }
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

		// --- Methods inherited from Texmap ---
		XYZGen *GetTheXYZGen() { return xyzGen; }
		RGBA EvalColor(ShadeContext& sc);
		Point3 EvalNormalPerturb(ShadeContext& sc);
		float NoiseFunc(float x, float y, float z);
		float BumpFunc(Point3 p);

		// --- Methods inherited from Tex3D ---
		void ReadSXPData(TCHAR *name, void *sxpdata);

		// --- Methods of Planet ---
		Planet();
		~Planet() {
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}
		void NotifyChanged();
		void SetSize(float i, TimeValue t);
		void SetColor(int i, Color c, TimeValue t);
		void SetPercent(float f, TimeValue t);
		void SetIsland(float f, TimeValue t); 
		void SetSeed(int i, TimeValue t);
		void ClampFloat(float &f, float min, float max);
		void ClampInt(int &i, int min, int max);

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

// This is the Class Descriptor for the Planet 3D Texture plug-in
class PlanetClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
		void *			Create(BOOL loading) { 	return new Planet; }
		const TCHAR *	ClassName() { return GetString(IDS_DS_PLANET_CDESC); } // mjm - 2.3.99
		SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
		Class_ID 		ClassID() { return planetClassID; }
		const TCHAR* 	Category() { return TEXMAP_CAT_3D; }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
		const TCHAR*	InternalName() { return _T("planet"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};

static PlanetClassDesc planetCD;

ClassDesc *GetPlanetDesc() { return &planetCD; }
ParamDlg* Planet::xyzGenDlg;	

/*===========================================================================*\
 | Class to Manage the User Interface in the Materials Editor
\*===========================================================================*/
/*
class PlanetDlg: public ParamDlg {
	public:
		// This is our UI rollup page window handle in the materials editor
		HWND hParamDlg;
		// Window handle of the materials editor dialog itself
		HWND hMedit;
		// Interface for calling methods provided by MAX
		IMtlParams *ip;
		// The current Planet being edited.
		Planet *theTex;
		// Parameter Map for handling UI controls
		IParamMap *pmap;
		// Custom conrols for the colors
		IColorSwatch *cs[NUM_COLORS];
		// This is used inside the SetTime method to only update the UI
		// controls when the time slider has changed
		TimeValue curTime; 
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
		void DeleteThis() { delete this; }

		// --- PlanetDlg Methods ---
		PlanetDlg(HWND hwMtlEdit, IMtlParams *imp, Planet *m); 
		~PlanetDlg();
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		void LoadDialog();
		void UpdateMtlDisplay() { ip->MtlChanged(); }
		void Invalidate();
};
*/
/*===========================================================================*\
 | Noise Functions and Data Structures
\*===========================================================================*/
#define NOISE_DIM 20
#define FNOISE_DIM 20.0f

static float noise_table[NOISE_DIM+1][NOISE_DIM+1][NOISE_DIM+1];

static void init_noise(int seed) {
	int i,j,k,ii,jj,kk;
	srand(seed);
	for (i=0; i<=NOISE_DIM; i++)
		for (j=0; j<=NOISE_DIM; j++)
			for (k=0; k<=NOISE_DIM; k++) {
				noise_table[i][j][k] = (float)(rand()&0x7FFF);
				ii = (i==NOISE_DIM)?0:i; 
				jj = (j==NOISE_DIM)?0:j; 
				kk = (k==NOISE_DIM)?0:k; 
				noise_table[i][j][k] = noise_table[ii][jj][kk];
			}
}

static float noise(float x, float y, float z) {
	int ix,iy,iz;
	float fx,fy,fz,mx,my,mz;
	float n,n00,n01,n10,n11,n0,n1;
	mx = (float)fmod(x,FNOISE_DIM); if (mx<0) mx += FNOISE_DIM;
	my = (float)fmod(y,FNOISE_DIM); if (my<0) my += FNOISE_DIM;
	mz = (float)fmod(z,FNOISE_DIM); if (mz<0) mz += FNOISE_DIM;
	ix = (int)mx;
	iy = (int)my;
	iz = (int)mz;
	fx = (float)fmod(mx,1.0f);
	fy = (float)fmod(my,1.0f);
	fz = (float)fmod(mz,1.0f);
	n = noise_table[ix][iy][iz];
	n00 = n + fx*(noise_table[ix+1][iy][iz]-n);
	n = noise_table[ix][iy][iz+1];
	n01 = n + fx*(noise_table[ix+1][iy][iz+1]-n);
	n = noise_table[ix][iy+1][iz];
	n10 = n + fx*(noise_table[ix+1][iy+1][iz]-n);
	n = noise_table[ix][iy+1][iz+1];
	n11 = n + fx*(noise_table[ix+1][iy+1][iz+1]-n);
	n0 = n00 + fy*(n10-n00);
	n1 = n01 + fy*(n11-n01);
	return(((float)(n0+fz*(n1-n0)))/32768.0f);
}


/*===========================================================================*\
 | Parameter Map Related Data and Methods
\*===========================================================================*/
// Parameter block indices
/*
#define PB_SIZE		0
#define PB_ISLAND	1 
#define PB_PERCENT	2
#define PB_SEED		3
#define PB_COL1		4
#define PB_COL2		5
#define PB_COL3		6
#define PB_COL4		7
#define PB_COL5		8
#define PB_COL6		9
#define PB_COL7		10
#define PB_COL8		11
*/
// Spinner limits
#define MIN_SIZE 0.0f
#define MAX_SIZE 999999999.0f

#define MIN_ISLAND 0.0f
#define MAX_ISLAND 100.0f

#define MIN_PERCENT 0.0f
#define MAX_PERCENT 100.0f

#define MIN_SEED 0
#define MAX_SEED 99999

// Paramter block version number
#define PLANET_PB_VERSION 2

enum { planet_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	planet_color1, planet_color2,
	planet_color3, planet_color4,
	planet_color5, planet_color6,
	planet_color7, planet_color8,
	planet_size, planet_island,		
	planet_percent, planet_seed,		
	planet_blend,
	planet_coords,	  // access for UVW mapping
};




static ParamBlockDesc2 planet_param_blk ( planet_params, _T("parameters"),  0, &planetCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_PLANET, IDS_DS_PLANET_PARAMS, 0, 0, NULL, 
	// params


	planet_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL1,	
		p_default,		Color(0.04, 0.08, 0.31), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL1, 
		end,
	planet_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL2,	
		p_default,		Color(0.04f, 0.12f, 0.31f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL2, 
		end,
	planet_color3,	 _T("color3"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL3,	
		p_default,		Color(0.04f, 0.16f, 0.31f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL3, 
		end,
	planet_color4,	 _T("color4"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL4,	
		p_default,		Color(0.04f, 0.39f, 0.05f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL4, 
		end,
	planet_color5,	 _T("color5"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL5,	
		p_default,		Color(0.39f, 0.31f, 0.05f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL5, 
		end,
	planet_color6,	 _T("color6"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL6,	
		p_default,		Color(0.31f, 0.08f, 0.03f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL6, 
		end,
	planet_color7,	 _T("color7"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL7,	
		p_default,		Color(0.39f, 0.31f, 0.20f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL7, 
		end,
	planet_color8,	 _T("color8"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COL8,	
		p_default,		Color(0.39f, 0.39f, 0.39f), 
		p_ui,			TYPE_COLORSWATCH, IDC_COL8, 
		end,
	planet_size,	_T("continentSize"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_CONTSIZE,
		p_default,		40.f,
		p_range,		MIN_SIZE, MAX_SIZE,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZE_EDIT,IDC_SIZE_SPIN, 0.1f, 
		end,
	planet_island,	_T("islandFactor"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_ISLAND,
		p_default,		0.5f,
		p_range,		MIN_ISLAND, MAX_ISLAND,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ISLAND_EDIT,IDC_ISLAND_SPIN, 0.1f, 
		end,
	planet_percent,	_T("oceanPercent"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_PERCENT,
		p_default,		60.f,
		p_range,		MIN_PERCENT, MAX_PERCENT,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PERCENT_EDIT,IDC_PERCENT_SPIN, 0.1f, 
		end,
	planet_seed,	_T("randomSeed"),   TYPE_INT,			0,	IDS_PW_RSEED,
		p_default,		12345,
		p_range,		MIN_SEED, MAX_SEED,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_SEED_EDIT,IDC_SEED_SPIN,  1.0f, 
		end,
	planet_blend,	_T("blendWaterLand"), TYPE_BOOL,			0,				IDS_PW_BLENDWL,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND,
		end,
	planet_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDINATES,
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
		PB_ISLAND, 
		EDITTYPE_FLOAT, 
		IDC_ISLAND_EDIT,IDC_ISLAND_SPIN, 
		MIN_ISLAND, MAX_ISLAND, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_PERCENT, 
		EDITTYPE_FLOAT, 
		IDC_PERCENT_EDIT,IDC_PERCENT_SPIN, 
		MIN_PERCENT, MAX_PERCENT, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(
		PB_SEED, 
		EDITTYPE_INT, 
		IDC_SEED_EDIT,IDC_SEED_SPIN, 
		MIN_SEED, MAX_SEED, 
		SPIN_AUTOSCALE), 

	ParamUIDesc(PB_COL1, TYPE_COLORSWATCH, IDC_COL1),
	ParamUIDesc(PB_COL2, TYPE_COLORSWATCH, IDC_COL2),
	ParamUIDesc(PB_COL3, TYPE_COLORSWATCH, IDC_COL3),
	ParamUIDesc(PB_COL4, TYPE_COLORSWATCH, IDC_COL4),
	ParamUIDesc(PB_COL5, TYPE_COLORSWATCH, IDC_COL5),
	ParamUIDesc(PB_COL6, TYPE_COLORSWATCH, IDC_COL6),
	ParamUIDesc(PB_COL7, TYPE_COLORSWATCH, IDC_COL7),
	ParamUIDesc(PB_COL8, TYPE_COLORSWATCH, IDC_COL8)

  };

*/
// The number of descriptors in the paramDesc array
#define PARAMDESC_LENGTH 12


// Parameter block parameters	
static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE,  planet_size }, // size 
	{ TYPE_FLOAT, NULL, TRUE,  planet_island}, // island
	{ TYPE_FLOAT, NULL, TRUE,  planet_percent }, // percent
	{ TYPE_INT,   NULL, FALSE, planet_seed }, // seed
	{ TYPE_RGBA,  NULL, TRUE,  planet_color1 }, // color 1
	{ TYPE_RGBA,  NULL, TRUE,  planet_color2}, // color 2
	{ TYPE_RGBA,  NULL, TRUE,  planet_color3 }, // color 3
	{ TYPE_RGBA,  NULL, TRUE,  planet_color4 }, // color 4
	{ TYPE_RGBA,  NULL, TRUE,  planet_color5 }, // color 5
	{ TYPE_RGBA,  NULL, TRUE,  planet_color6 }, // color 6
	{ TYPE_RGBA,  NULL, TRUE,  planet_color7 }, // color 7
	{ TYPE_RGBA,  NULL, TRUE,  planet_color8 }  // color 8
};
// The number of parameters in the parameter block
#define PB_LENGTH 12

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,12,1)	// Version 1 params
	};

// The names of the parameters in the parameter block
static int nameIDs[] = {
	IDS_DS_CONTSIZE, IDS_DS_ISLAND, IDS_DS_PERCENT, IDS_DS_SEED, 
	IDS_DS_COL1, IDS_DS_COL2,IDS_DS_COL3,IDS_DS_COL4,
	IDS_DS_COL5,IDS_DS_COL6,IDS_DS_COL7,IDS_DS_COL8 };

/*
// This is the class that allows the sub-map buttons to be processed.
class PlanetDlgProc : public ParamMapUserDlgProc {
	public:
		PlanetDlg *theDlg;
		PlanetDlgProc(PlanetDlg *s) { theDlg = s; }
		BOOL DlgProc(TimeValue t, IParamMap *map,
			HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() { delete this; }
};

// This is the dialog proc to process the texmap buttons
BOOL PlanetDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	theDlg->isActive = TRUE;
	BOOL res = theDlg->PanelProc(hWnd, msg, wParam, lParam);
	theDlg->isActive = FALSE;
	return res;
}

static int colNameIDs[] = {
	IDS_DS_COL1, IDS_DS_COL2,IDS_DS_COL3,IDS_DS_COL4,
	IDS_DS_COL5,IDS_DS_COL6,IDS_DS_COL7,IDS_DS_COL8 };

BOOL PlanetDlg::PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG: {
			for (int i = 0; i < NUM_COLORS; i++) 
   				cs[i] = GetIColorSwatch(GetDlgItem(hParamDlg, colID[i]),
   					theTex->col[i], GetString(colNameIDs[i]));
			return TRUE;
		}
		break;

		case WM_COMMAND:  
		    switch (id) {
				case IDC_BLEND:
					theTex->blend = GetCheckBox(hWnd, id);
					theTex->NotifyChanged();
					UpdateMtlDisplay();
					break;

			}
			break;

		// We need to trap the case of the 'Seed' value changing because
		// we need to update the noise table with new values.
		case WM_CUSTEDIT_ENTER: {
			int editID = wParam;
			if (editID == IDC_SEED_EDIT) {
				init_noise(theTex->seed);
				theTex->NotifyChanged();
			    UpdateMtlDisplay();
				break;
			}
		}
		case CC_SPINNER_BUTTONUP: {
			int spinID = LOWORD(wParam);
			if (spinID == IDC_SEED_SPIN) {
				init_noise(theTex->seed);
				theTex->NotifyChanged();
			    UpdateMtlDisplay();
				break;
			}
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
 | PlanetDlg Methods
\*===========================================================================*/
// --- PlanetDlg Methods ---
// Constructor.
// This is called from within the Planet::CreateParamDlg method.  That
// method is passed the handle to the materials editor dialog, and an
// interface for calling methods of MAX.  These are passed in here and stored.
/*
PlanetDlg::PlanetDlg(HWND hwMtlEdit, IMtlParams *imp, Planet *m) { 
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
		theTex->pblock, ip, hInstance, MAKEINTRESOURCE(IDD_PLANET),
		GetString(IDS_DS_PLANET_PARAMS), 0);

	// Save the window handle of the rollup page
	hParamDlg = pmap->GetHWnd();

	// Establish the dialog proc to handle the custom button controls
	pmap->SetUserDlgProc(new PlanetDlgProc(this));
}

// Destructor.
// This is called after the user changes to another sample slot in
// the materials editor that does not contain a Planet texture.
// Note that it is not called if they do go to another Planet -- in
// that case, the parameters in the rollup page are updated, but
// the entire page is not deleted.  This is accomplished by simply
// changing the parameter block pointer (done inside PlanetDlg::SetThing()).
PlanetDlg::~PlanetDlg() {
	theTex->paramDlg = NULL;
	// Delete the XYZGen class we created
	xyzGenDlg->DeleteThis();
	// Delete the parameter map
	DestroyMParamMap(pmap);
	pmap = NULL;
}

// Update the dialog display with the values of the texture we are
// currently editing.
void PlanetDlg::LoadDialog() {
	if (theTex) {
		Interval ivalid;
		theTex->Update(curTime, ivalid);

		ISpinnerControl *spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SIZE_SPIN));
		spin->SetValue(theTex->size, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_ISLAND_SPIN));
		spin->SetValue(theTex->island, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_PERCENT_SPIN));
		spin->SetValue(theTex->percent, FALSE);
		ReleaseISpinner(spin);

		spin = (ISpinnerControl *)
			GetISpinner(GetDlgItem(hParamDlg, IDC_SEED_SPIN));
		spin->SetValue(theTex->seed, FALSE);
		ReleaseISpinner(spin);

		cs[0]->SetColor(theTex->col[0]);
		cs[1]->SetColor(theTex->col[1]);
		cs[2]->SetColor(theTex->col[2]);
		cs[3]->SetColor(theTex->col[3]);
		cs[4]->SetColor(theTex->col[4]);
		cs[5]->SetColor(theTex->col[5]);
		cs[6]->SetColor(theTex->col[6]);
		cs[7]->SetColor(theTex->col[7]);
		CheckDlgButton(hParamDlg, IDC_BLEND, 
			 ((theTex->blend) ? BST_CHECKED : BST_UNCHECKED));
	}
}

// This method invalidates the rollup page so it will get redrawn
void PlanetDlg::Invalidate() { 
	InvalidateRect(hParamDlg, NULL, FALSE); 
	valid = FALSE; 
}

// --- Methods inherited from ParamDlg ---
// Returns the Class_ID of the plug-in this dialog manages
Class_ID PlanetDlg::ClassID() {
	return planetClassID; 
}

// This sets the current texture being edited to the texture passed
void PlanetDlg::SetThing(ReferenceTarget *m) {
	assert(m->ClassID() == planetClassID);
	assert(m->SuperClassID() == TEXMAP_CLASS_ID);
	if (theTex) 
		theTex->paramDlg = NULL;

	// Set the pointer to the texmap being edited to the one passed.
	theTex = (Planet *)m;

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
ReferenceTarget *PlanetDlg::GetThing() {
	return (ReferenceTarget *)theTex; 
}

// This method is called when the current time has changed.  
// This gives the developer an opportunity to update any user 
// interface data that may need adjusting due to the change in time.
void PlanetDlg::SetTime(TimeValue t) {
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
void PlanetDlg::ReloadDialog() {
	Interval ivalid;
	theTex->Update(curTime, ivalid);
	LoadDialog();
}

// This method is called when the dialog box becomes active or inactive. 
void PlanetDlg::ActivateDlg(BOOL onOff) {
	for (int i = 0; i < NUM_COLORS; i++) {
		cs[i]->Activate(onOff);
	}
}
*/
/*===========================================================================*\
 | Planet Methods
\*===========================================================================*/
// --- Methods inherited from Animatable ---
// This method returns a pointer to the 'i-th' sub-anim.  
Animatable* Planet::SubAnim(int i) {
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return 0;
	}
}

// This method returns the name of the 'i-th' sub-anim to appear in track view. 
TSTR Planet::SubAnimName(int i) {
	switch (i) {
		case 0: return GetString(IDS_DS_COORDS);
		case 1: return GetString(IDS_DS_PARAMETERS);
		default: return TSTR(_T(""));
	}
}

// --- Methods inherited from ReferenceMaker ---
// Return the 'i-th' reference
RefTargetHandle Planet::GetReference(int i) {
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default: return 0;
	}
}

// Save the 'i-th' reference
void Planet::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
	}
}

// This method is responsible for responding to the change notification
// messages sent by the texmap dependants.
RefResult Planet::NotifyRefChanged(Interval changeInt, 
	RefTargetHandle hTarget, PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:

			texValidity.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				// MM: 2/99, If the seed changed update the noise table
				if (changing_param == planet_seed) {
					pblock->GetValue(planet_seed, 0, seed, texValidity);
					init_noise(seed);
				}
				planet_param_blk.InvalidateUI(changing_param);
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
//			if (hTarget == pblock) {
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
				case PB_PERCENT:
					gpd->dim = stdPercentDim; break;
				case PB_SIZE: gpd->dim =  stdWorldDim; 	break;
				case PB_ISLAND:
				case PB_SEED:
					gpd->dim = defaultDim; break;
				case PB_COL1: 
				case PB_COL2: 
				case PB_COL3: 
				case PB_COL4: 
				case PB_COL5: 
				case PB_COL6: 
				case PB_COL7: 
				case PB_COL8: 
					gpd->dim = stdColor255Dim; break;
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
#define PLANET_VERS1_CHUNK		0x4001
#define BLEND_CHUNK				0x1000
#define PARAM2_CHUNK			0x1010

// This is called by the system to allow the plug-in to save its data
IOResult Planet::Save(ISave *isave) { 
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

	isave->BeginChunk(PLANET_VERS1_CHUNK);
	isave->EndChunk();
	if (blend) {
		isave->BeginChunk(BLEND_CHUNK);
		isave->EndChunk();

	}
*/
	return IO_OK;
}

class PlanetPostLoad : public PostLoadCallback {
	public:
		Planet *n;
		BOOL Param1;
		PlanetPostLoad(Planet *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( planet_blend, 0, n->blend);
				}
			delete this; 


			} 
	};



// This is called by the system to allow the plug-in to load its data
IOResult Planet::Load(ILoad *iload) { 
	IOResult res;
	int id;
	fileVersion = 0;
	blend = 0;
	
	BOOL Param1 = TRUE;

	while (IO_OK == (res = iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				// Load the common stuff from the base class
				res = MtlBase::Load(iload);
				break;
			case BLEND_CHUNK:
				blend = 1; 
				break;
			case PLANET_VERS1_CHUNK:
				// Set the version number
				fileVersion = 1;
				break;
			case PARAM2_CHUNK:
				// Set the version number
				Param1 = FALSE;
				break;

		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &planet_param_blk, this, 1);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new PlanetPostLoad(this,Param1));

	return IO_OK;
}

// --- Methods inherited from ReferenceTarget ---
// This method is called to have the plug-in clone itself.
RefTargetHandle Planet::Clone(RemapDir &remap) {
	// Create a new instance of the plug-in class
	Planet *newPlanet = new Planet();

	// Copy superclass stuff
	*((MtlBase *)newPlanet) = *((MtlBase *)this);

	// Clone the items we reference
	newPlanet->ReplaceReference(0, remap.CloneRef(xyzGen));
	newPlanet->ReplaceReference(1, remap.CloneRef(pblock));
	newPlanet->col[0] = col[0];
	newPlanet->col[1] = col[1];
	newPlanet->col[2] = col[2];
	newPlanet->col[3] = col[3];
	newPlanet->col[4] = col[4];
	newPlanet->col[5] = col[5];
	newPlanet->col[6] = col[6];
	newPlanet->col[7] = col[7];
	newPlanet->size = size;
	newPlanet->island = island;
	newPlanet->percent = percent;
	newPlanet->seed = seed;
	newPlanet->texValidity.SetEmpty();	
	BaseClone(this, newPlanet, remap);
	// Return the new cloned texture
	return (RefTargetHandle)newPlanet;
}

// --- Methods inherited from MtlBase ---
// This method is called to return the validity interval of the texmap.
Interval Planet::Validity(TimeValue t) { 
	Interval v;
	// Calling Update() sets texValidity.
	Update(t, v); 
	return texValidity; 
}

// This method is called to reset the texmap back to its default values.
void Planet::Init() {
	// Reset the XYZGen or allocate a new one
	if (xyzGen) 
		xyzGen->Reset();
	else 
		ReplaceReference(0, GetNewDefaultXYZGen());	


//	ReplaceReference(1, CreateParameterBlock(pbdesc, 
//		PB_LENGTH, PLANET_PB_VERSION));
//	if (paramDlg) 
//		paramDlg->pmap->SetParamBlock(pblock);

	// Set the inital parameters
	// {10,20,80},{10,30,80},{10,40,90},{10,100,12},
	// {100,80,12},{80,20,8},{100,80,50},{100,100,100}
	SetColor(0, Color(0.04f, 0.08f, 0.31f), TimeValue(0));
	SetColor(1, Color(0.04f, 0.12f, 0.31f), TimeValue(0));
	SetColor(2, Color(0.04f, 0.16f, 0.31f), TimeValue(0));
	SetColor(3, Color(0.04f, 0.39f, 0.05f), TimeValue(0));
	SetColor(4, Color(0.39f, 0.31f, 0.05f), TimeValue(0));
	SetColor(5, Color(0.31f, 0.08f, 0.03f), TimeValue(0));
	SetColor(6, Color(0.39f, 0.31f, 0.20f), TimeValue(0));
	SetColor(7, Color(0.39f, 0.39f, 0.39f), TimeValue(0));

	SetSize(40.0f, TimeValue(0));
	SetIsland(0.5f, TimeValue(0));
	SetPercent(60.0f, TimeValue(0));
	SetSeed(12345, TimeValue(0));
	blend = 1;

	// Set the validity interval of the texture to empty
	texValidity.SetEmpty();
}

void Planet::Reset() {
	planetCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}

// This method gets called when the material or texture is to be displayed 
// in the material editor parameters area. 
ParamDlg* Planet::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// Allocate a new instance of ParamDlg to manage the UI.  This will
	// create the rollup page in the materials editor.
//	PlanetDlg *planetDlg = new PlanetDlg(hwMtlEdit, imp, this);
	// Update the dialog display with the proper values of the texture.
//	planetDlg->LoadDialog();
//	paramDlg = planetDlg;
//	return planetDlg;	
	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = planetCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	return masterDlg;

}

BOOL Planet::SetDlgThing(ParamDlg* dlg)
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
void Planet::Update(TimeValue t, Interval& ivalid) {		
	if (!texValidity.InInterval(t)) {
		texValidity.SetInfinite();
		xyzGen->Update(t, texValidity);
		for (int i = 0; i < NUM_COLORS; i++) {
//			pblock->GetValue(i+PB_COL1, t, col[i], texValidity);
			pblock->GetValue(i+planet_color1, t, col[i], texValidity);
			col[i].ClampMinMax();
			}

//		pblock->GetValue(PB_SIZE, t, size, texValidity);
		pblock->GetValue(planet_size, t, size, texValidity);
		ClampFloat(size, MIN_SIZE, MAX_SIZE);
//		pblock->GetValue(PB_ISLAND, t, island, texValidity);
		pblock->GetValue(planet_island, t, island, texValidity);
		ClampFloat(island, MIN_ISLAND, MAX_ISLAND);
//		pblock->GetValue(PB_PERCENT, t, percent, texValidity);
		pblock->GetValue(planet_percent, t, percent, texValidity);
		ClampFloat(percent, MIN_PERCENT, MAX_PERCENT);
		land = percent/100.0f;
//		pblock->GetValue(PB_SEED, t, seed, texValidity);
		pblock->GetValue(planet_seed, t, seed, texValidity);
		ClampInt(seed, (int) MIN_SEED, (int) MAX_SEED);
		pblock->GetValue(planet_blend, t, blend, texValidity);
	}
	ivalid &= texValidity;
}

void Planet::ClampFloat(float &f, float min, float max) {
	if (f < min) f = min;
	else if (f > max) f = max;
}

void Planet::ClampInt(int &i, int min, int max) {
	if (i < min) i = min;
	else if (i > max) i = max;
}

float Planet::NoiseFunc(float x, float y, float z) {
 	return  noise(x, y, z) + noise(x*island, y*island, z*island)/5.0f;
	}

// --- Methods inherited from Texmap ---
RGBA Planet::EvalColor(ShadeContext& sc) {
	float d, x, y, z;
	RGBA color;

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
	x = p.x/size;
	y = p.y/size;
	z = p.z/size;
	d = NoiseFunc(x, y, z);
	if (d < land) {
		float frac;
		int index;

		d = d/land*3.0f;
		index = (int)d;
		frac = d-(float)index;
		if (index < 2)
			color = (1.0f-frac)*col[index]+frac*col[index+1];
		else {
			if (blend)
				color = (1.0f-frac)*col[2]+frac*col[3];
			else
				color = col[2];
			}
		}
	else {
		float divfac, frac;
		int index;
		
		divfac = 1.0f-land;
		if (divfac==0.0) divfac = .000001f;
		d = (d-land)/divfac*5;
		index = (int)d;
		frac = d-(float)index;
		if (index < 4)
			color = (1.0f-frac)*col[index+3]+frac*col[index+4];
		else
			color = col[7];
		}
	return color;
	}

float Planet::BumpFunc(Point3 p) {
	float f = NoiseFunc(p.x,p.y,p.z);
	return (f<land)?land:f;
	}

Point3 Planet::EvalNormalPerturb(ShadeContext& sc) {
	float del,d;
	Point3 p,dp;
	if (!sc.doMaps) return Point3(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
	xyzGen->GetXYZ(sc,p,dp);
	if (size == 0.0f) 
		size = 0.0001f;
	p /= size;
	del = 10.0f;
	d = BumpFunc(p);
	Point3 np;
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
    np.x = (BumpFunc(p+del*M[0]) - d)/del;
	np.y = (BumpFunc(p+del*M[1]) - d)/del;
	np.z = (BumpFunc(p+del*M[2]) - d)/del;
	return sc.VectorFromNoScale(np*100.0f,REF_OBJECT);
    }

// --- Methods inherited from Tex3D ---
void Planet::ReadSXPData(TCHAR *name, void *sxpdata) {
	PlanetState *state = (PlanetState*)sxpdata;
	if (state != NULL && (state->version == PLANET_SXP_VERSION)) {
		SetSize(state->size, TimeValue(0));
		SetPercent(state->percent, TimeValue(0));
		SetIsland(state->island, TimeValue(0));
		SetSeed(state->seed, TimeValue(0));
	}
}

// --- Methods of Planet ---
Planet::Planet() {
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	pblock = NULL;
	xyzGen = NULL;
//	paramDlg = NULL;
	planetCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	fileVersion = 0;
	// Allocate data structures, compute lookup tables, etc.
	init_noise(seed);
}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Planet::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif

void Planet::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void Planet::SetSize(float f, TimeValue t) { 
	size = f; 
//	pblock->SetValue(PB_SIZE, t, f);
	pblock->SetValue(planet_size, t, f);
}

void Planet::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
//	pblock->SetValue(i+PB_COL1, t, c);
	pblock->SetValue(i+planet_color1, t, c);
}

void Planet::SetPercent(float f, TimeValue t) { 
	percent = f; 
	// Also set land as percent/100.
	land = f/100.0f;
//	pblock->SetValue(PB_PERCENT, t, f);
	pblock->SetValue(planet_percent, t, f);
}

void Planet::SetIsland(float f, TimeValue t) { 
	island = f; 
//	pblock->SetValue(PB_ISLAND, t, f);
	pblock->SetValue(planet_island, t, f);
}

void Planet::SetSeed(int i, TimeValue t) { 
	seed = i; 
//	pblock->SetValue(PB_SEED, t, i);
	pblock->SetValue(planet_seed, t, i);
}


