// cstm1dlg.cpp : implementation file
//

#include "stdafx.h"
#include "SDKAPWZ.h"
#include "cstm1dlg.h"
#include "SDKAPWZaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog


CCustom1Dlg::CCustom1Dlg()
	: CAppWizStepDlg(CCustom1Dlg::IDD)
{
	//{{AFX_DATA_INIT(CCustom1Dlg)
	//}}AFX_DATA_INIT
}


void CCustom1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CAppWizStepDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom1Dlg)
	DDX_Control(pDX, IDC_PICTURE, m_PicFrame);
	DDX_Control(pDX, IDC_PLUGIN_TYPES, m_PlugTypes);
	//}}AFX_DATA_MAP
}

struct PlugType
{
	char* caption;		// caption of the plugin class
	char* key;			// Associate key type
	char* ext;			// Extension of this plugin (eg: dlr, dlo, etc.)
	char* sid;			// Super Class ID
	char* bclass1;		// super class name that plugin should be derived from
	char* bclass2;		// alternative super class name
	bool  supported;	// if this type of plugin is fully supported by appwizard
	bool  rollout;		// TRUE if the dialog is a rollout (eg:object plugins)
	bool  staticType;	// TRUE if static instance of this plugin type has to be created
};

// Get the following table from pluglist.xls, Column I
static PlugType plugins[] = {	
	//	caption						key								 ext	sid							bclass1					bclass2		supported  rollout staticType
	{ "Atmospheric",				"ATMOSPHERIC_TYPE",				"dlr",	"ATMOSPHERIC_CLASS_ID",		"Atmospheric",			"",				true,	true,  false },
	{ "Anti-Aliasing Filters",		"FILTER_KERNEL_TYPE",			"dlk",	"FILTER_KERNEL_CLASS_ID",	"FilterKernel",			"",				true,	true,  false },
	{ "Cameras",					"CAMERA_TYPE",					"dlo",	"CAMERA_CLASS_ID",			"CameraObject",			"GenCamera",	false,	true,  false },
	{ "Color Selector",				"COLPICK_TYPE",					"dlu",	"COLPICK_CLASS_ID",			"ColPick",				"",				true,	false, false },	
	{ "Construction Grid Objects",	"GRID_OBJECT_TYPE",				"dlo",	"UNKNOWN_CLASS_ID",			"None",					"",				false,	true,  false },
	{ "Controllers",				"CONTROLLER_TYPE",				"dlc",	"CTRL_POSITION_CLASS_ID",	"Control",				"StdControl",	false,	true,  false },
	{ "File Export",				"FILE_EXPORT_TYPE",				"dle",	"SCENE_EXPORT_CLASS_ID",	"SceneExport",			"",				true,	false, false },
	{ "File Import",				"FILE_IMPORT_TYPE",				"dli",	"SCENE_IMPORT_CLASS_ID",	"SceneImport",			"",				true,	false, false },
	{ "File List",					"FILE_LIST_TYPE",				"",		"UNKNOWN_CLASS_ID",			"None",					"",				false,	true,  false },
//	{ "Front End Controllers",		"FRONT_END_CONTROLLER_TYPE",	"dlu",	"FRONTEND_CONTROL_CLASS_ID","FrontEndController",	"",				true,	true,  true  },
	{ "Global Utility Plug-Ins",	"GUP_TYPE",						"gup",	"GUP_CLASS_ID",				"GUP",					"",				true,	false, false },	
	{ "Helper Objects",				"HELPER_OBJECT_TYPE",			"dlo",	"HELPER_CLASS_ID",			"HelperObject",			"ConstObject",	false,	true,  false },
	{ "IK Solvers",					"IK_TYPE",						"dlc",	"IK_SOLVER_CLASS_ID",		"IKSolver",				"",				true,	false,  false },
	{ "Image Filter / Compositor",	"IMAGE_FILTER_COMPOSITOR_TYPE", "flt",	"FLT_CLASS_ID",				"ImageFilter",			"",				true,	false, false },
	{ "Image Loader / Saver",		"IMAGE_LOADER_SAVER_TYPE",		"bmi",	"BMM_IO_CLASS_ID",			"BitmapIO",				"",				false,	true,  false },
	{ "Image Viewer",				"IMAGE_VIEWER_TYPE",			"dlf",	"BMM_FILTER_CLASS_ID",		"ViewFile",				"",				false,	true,  false },
	{ "Lights",						"LIGHT_TYPE",					"dlo",	"LIGHT_CLASS_ID",			"LightObject",			"GenLight",		false,	true,  false },
	{ "Manipulators",				"MANIP_TYPE",					"dlo",	"HELPER_CLASS_ID",			"Manipulator",			"SimpleManipulator",		true,	true,  false },
	{ "Materials",					"MATERIAL_TYPE",				"dlt",	"MATERIAL_CLASS_ID",		"Mtl",					"",				true,	true,  false },
	{ "Modifiers",					"MODIFIER_TYPE",				"dlm",	"OSM_CLASS_ID",				"Modifier",				"SimpleMod2",	true,	true,  false },
	{ "NURBS Objects:",				"NURBS_OBJECT_TYPE",			"dlo",	"UNKNOWN_CLASS_ID",			"None",					"",				false,	true,  false },
	{ "Particle Systems / Effects",	"PARTICLE_TYPE",				"dlo",	"GEOMOBJECT_CLASS_ID",		"ParticleObject",		"SimpleParticle",false, true,  false },
	{ "Patch Objects",				"PATCH_OBJECT_TYPE",			"dlo",	"GEOMOBJECT_CLASS_ID",		"PatchObject",			"",				false,	true,  false },
	{ "Procedural Objects",			"PROCEDURAL_OBJECT_TYPE",		"dlo",	"GEOMOBJECT_CLASS_ID",		"GeomObject",			"SimpleObject2",true,	true,  false },
	{ "Renderer",					"RENDERER_TYPE",				"dlr",	"RENDERER_CLASS_ID",		"Renderer",				"",				false,	true,  false },
	{ "Rendering Effects",			"RENDER_EFFECT_TYPE",			"dlv",	"RENDER_EFFECT_CLASS_ID",	"Effect",				"",				true,	true,  false },
	{ "Samplers",					"SAMPLER_TYPE",					"dlh",	"SAMPLER_CLASS_ID",			"Sampler",				"",				true,	true,  false },
	{ "Shaders",					"SHADER_TYPE",					"dlb",  "SHADER_CLASS_ID ",			"Shader",				"",				true,	true,  false },	
	{ "Shadow Generator",			"SHADOW_TYPE",					"dlo",	"SHADOW_TYPE_CLASS_ID",		"ShadowType",			"",				true,	true,  false },	
	{ "Skin Deformer Gizmo",		"SKIN_GIZMO_TYPE",				"dlm",	"REF_TARGET_CLASS_ID",		"GizmoClass",			"",				true,	true,  false },	
	{ "Sound Plug-Ins",				"SOUND_TYPE",					"dlo",	"SOUNDOBJ_CLASS_ID",		"SoundObj",				"",				false,	true,  false },
	{ "Space Warps",				"SPACE_WARP_TYPE",				"dlm",	"WSM_CLASS_ID",				"SimpleWSMMod",			"",				true,	true,  false },
//	{ "Spline Shapes",				"SPLINE_SHAPE_TYPE",			"dlo",	"SHAPE_CLASS_ID",			"SimpleSpline",			"SimpleShape",	true,	true,  false },
	{ "Textures 2D",				"TEXTURE_2D_TYPE",				"dlt",	"TEXMAP_CLASS_ID",			"Texmap",				"",				true,	true,  false },
	{ "Textures 3D",				"TEXTURE_3D_TYPE",				"dlt",	"TEXMAP_CLASS_ID",			"Texmap",				"Tex3D",		true,	true,  false },
	{ "Track View Utility",			"TRACK_VIEW_UTILITY_TYPE",		"dlu",	"TRACKVIEW_UTILITY_CLASS_ID","TrackViewUtility",	"",				true,	false, true  },
	{ "Utility",					"UTILITY_TYPE",					"dlu",	"UTILITY_CLASS_ID",			"UtilityObj",			"",				true,	true,  true  }
};

#define plugin_count (sizeof(plugins)/sizeof(plugins[1]))

// This is called whenever the user presses Next, Back, or Finish with this step
//  present.  Do all validation & data exchange from the dialog in this function.
BOOL CCustom1Dlg::OnDismiss()
{
	if (!UpdateData(TRUE))
		return FALSE;
	
	CString sel;
	m_PlugTypes.GetText(m_PlugTypes.GetCurSel(), sel);
	
	// Remove the Plugin type related keys
	for (int i=0; i < plugin_count; i++)
		remove_key(plugins[i].key);

	remove_key(_T("FLOATING_DIALOG"));
	remove_key(_T("STATIC_TYPE"));
	remove_key(_T("TEX_TYPE")); 	
	remove_key(_T("UI_BY_MAX")); 	

	//Emptying the base class list for Class ListBox in Dlg2
	ClassList.RemoveAll();
	ClassList.FreeExtra();
	ClassList.SetSize(0, 1);
	
	// Set plugin specific parameters
	for (i=0; i < plugin_count; i++)
	{		
		if (_tcsicmp(sel, plugins[i].caption)!=0) continue;

		PlugType *plg = &plugins[i];
		
		if (_tcsicmp(plg->sid, _T("TEXMAP_CLASS_ID"))==0)
			set_key(_T("TEX_TYPE"), _T("YES"));
		
		// For following plugin types the UI is automatically generated by Max
		if (_tcsicmp(plg->key, _T("SAMPLER_TYPE"))==0 || 
			_tcsicmp(plg->key, _T("SHADER_TYPE"))==0 ||
			_tcsicmp(plg->key, _T("FILTER_KERNEL_TYPE"))==0)
			set_key(_T("UI_BY_MAX"), _T("YES"));

		set_key(plg->key, _T("YES"));
		set_key(_T("PLUGEXT"), plg->ext);
		set_key(_T("SUPER_CLASS_ID"), plg->sid);
		
		// Add base class names to the base class list
		if (_tcsicmp(plg->bclass1, "")==0)
			ClassList.Add("None");
		else
			ClassList.Add(plg->bclass1);
		
		if (_tcsicmp(plg->bclass2, "")!=0) ClassList.Add(plg->bclass2);					
		
		if (!plg->rollout) set_key(_T("FLOATING_DIALOG"), _T("YES"));
		if (plg->staticType) set_key(_T("STATIC_TYPE"), _T("YES"));
		
		if (!plg->supported) MessageBox( 
				_T("This plugin type is not currently supported, minimal code will be generated."),
				_T("Unsupported Plugin Type"));
	}
		//Deleting bitmaps
		
		return TRUE;	// return FALSE if the dialog shouldn't be dismissed
}


BEGIN_MESSAGE_MAP(CCustom1Dlg, CAppWizStepDlg)
	//{{AFX_MSG_MAP(CCustom1Dlg)
	ON_LBN_SELCHANGE(IDC_PLUGIN_TYPES, OnSelchangePluginTypes)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg message handlers


void CCustom1Dlg::OnSelchangePluginTypes() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CCustom1Dlg::OnInitDialog() 
{
	CAppWizStepDlg::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (int i=0; i < plugin_count; i++)
		m_PlugTypes.AddString(plugins[i].caption);
	m_PlugTypes.SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustom1Dlg::DrawBitmaps()
{
	CDC cpDcVw, cpDcTxt, *cdc  ;
	RECT rec ;
	m_PicFrame.GetWindowRect(&rec) ;
	cdc = m_PicFrame.GetDC() ;
	cpDcVw.CreateCompatibleDC(cdc);
	
	// Draw the text Geometry bitmap
	cpDcVw.SelectObject(pGeoBMap) ;
	cdc->BitBlt(0,0,rec.right - rec.left, rec.bottom-rec.top,&cpDcVw,0,0,SRCCOPY) ;
	
	cpDcTxt.CreateCompatibleDC(cdc);
	cpDcTxt.SelectObject(pShpBMap) ;
	cdc->BitBlt(0,165,rec.right - rec.left, rec.bottom-rec.top,&cpDcTxt,0,0,SRCCOPY) ;
	//cdc->StretchBlt(0, 0, rec.right-rec.left, rec.bottom-rec.top, &cpDcTxt, 
	//	0, 0, rec.right-rec.left, rec.bottom-rec.top, SRCCOPY);
}

int CCustom1Dlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CAppWizStepDlg::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	bGeo.LoadBitmap(IDB_MAX1) ;
	//bShp.LoadBitmap(IDB_SHP) ;
	
	pGeoBMap = &bGeo ; 
	pShpBMap = &bShp ;

	return 0;
}

void CCustom1Dlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CAppWizStepDlg::OnPaint() for painting messages
	DrawBitmaps();
	HWND FinishButton = ::GetDlgItem(::GetParent(m_hWnd),440);
	::EnableWindow(FinishButton,false);
}

