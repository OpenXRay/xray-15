// cstm3dlg.cpp : implementation file
//

#include "stdafx.h"
#include "SDKAPWZ.h"
#include "cstm3dlg.h"
#include "SDKAPWZaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustom3Dlg dialog


CCustom3Dlg::CCustom3Dlg()
	: CAppWizStepDlg(CCustom3Dlg::IDD)
{
	//{{AFX_DATA_INIT(CCustom3Dlg)
	m_SdkPath = _T("");
	m_PlgPath = _T("");
	m_ExePath = _T("");
	m_Comments = FALSE;	
	m_HybConfig = FALSE;
	m_ParamMaps = FALSE;
	m_ExtensionChannel = FALSE;
	//}}AFX_DATA_INIT
}


void CCustom3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CAppWizStepDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom3Dlg)
	DDX_Control(pDX, IDC_EXTENSION, m_ctlExtCh);
	DDX_Control(pDX, IDC_PARAM_MAPS, m_CtlMaps);
	DDX_Control(pDX, IDC_PICTURE, m_PicFrame);
	DDX_Text(pDX, IDC_SDKPATH, m_SdkPath);
	DDX_Text(pDX, IDC_PLUGPATH, m_PlgPath);
	DDX_Text(pDX, IDC_EXEPATH, m_ExePath);
	DDX_Check(pDX, IDC_COMMENTS, m_Comments);	
	DDX_Check(pDX, IDC_PARAM_MAPS, m_ParamMaps);
	DDX_Check(pDX, IDC_EXTENSION, m_ExtensionChannel);
	//}}AFX_DATA_MAP
}

static const char* appwz_key_name	= "SOFTWARE\\Autodesk\\3ds max\\5.0\\SdkApWz";
static const char* sdk_key_name		= "SdkPath";
static const char* plg_key_name		= "PluginPath";
static const char* exe_key_name		= "ExePath";

// This is called whenever the user presses Next, Back, or Finish with this step
//  present.  Do all validation & data exchange from the dialog in this function.
BOOL CCustom3Dlg::OnDismiss()
{
	if (!UpdateData(TRUE))
		return FALSE;
	
	long ec; 
	CString buf;
	HKEY hKey; 
	DWORD dwDisposition;

	// TODO: Set template variables based on the dialog's data.
	set_key("SDKPATH",	m_SdkPath);
	set_key("PLGPATH", m_PlgPath);
	set_key("EXEPATH",	m_ExePath);
	
	// Store path names in the registry
	ec = RegCreateKeyEx(HKEY_LOCAL_MACHINE, appwz_key_name, 0, NULL,
				REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if (ec == ERROR_SUCCESS)
	{
		const char* sp = (LPCTSTR)m_SdkPath;
		const char* pp = (LPCTSTR)m_PlgPath;
		const char* ep = (LPCTSTR)m_ExePath;
		RegSetValueEx(hKey, sdk_key_name, 0, REG_SZ, (LPBYTE)sp, _tcslen(sp)+1);
		RegSetValueEx(hKey, plg_key_name, 0, REG_SZ, (LPBYTE)pp, _tcslen(pp)+1);
		RegSetValueEx(hKey, exe_key_name, 0, REG_SZ, (LPBYTE)pp, _tcslen(ep)+1);
		RegCloseKey(hKey); 
	}

	if(m_Comments)
		set_key("ADD_COMMENTS", "YES");
	else
		remove_key("ADD_COMMENTS");
	
	if(m_HybConfig)
		set_key("HYBCONFIG", "YES");
	else
		remove_key("HYBCONFIG");
	if(m_ExtensionChannel)
		set_key("EXTENSION","YES");
	else
		remove_key("EXTENSION");
	
	BOOL tex = lookup_key(_T("TEX_TYPE"),		buf);
	BOOL mtl = lookup_key(_T("MATERIAL_TYPE"),	buf);
	
	if(m_ParamMaps) // Set the necessary keys if implementing param maps
	{
		set_key("PARAM_MAPS",	_T("YES"));
		set_key("PARAM_NAME",	_T("GetString(IDS_PARAMS)"));
		
		// Should refer to SimpleObject2::pblock2 instead of old SimpleObject::pblock
		if (lookup_key(_T("SIMPLE_TYPE")) && (lookup_key(_T("PROCEDURAL_OBJECT_TYPE")) || lookup_key(_T("MODIFIER_TYPE"))))
		{
			set_key("PBLOCK",		_T("pblock2"));
			set_key("SIMPLE_OBJ",	_T("YES"));
		}
		else
			set_key("PBLOCK",		_T("pblock"));
		if (tex) // if a texture map plugin
		{
			set_key(_T("NUM_REFS"),_T("2+NSUBTEX"));
			set_key(_T("PB_REF"),	_T("1"));
		}
		else if(mtl) 
		{
			set_key(_T("NUM_REFS"),_T("1+NSUBMTL"));
			set_key(_T("PB_REF"),	_T("0"));			
		}
		else 
		{
			set_key(_T("NUM_REFS"),_T("1"));
			set_key(_T("PB_REF"),	_T("0"));
		}
	}
	else
	{
		remove_key("PARAM_MAPS");
		set_key("PBLOCK",		"NULL");
		set_key("PARAM_NAME",  _T("_T(\"\")"));
		
		if (tex) // If a texture map plugin
			set_key(_T("NUM_REFS"), _T("1+NSUBTEX"));
		else if(mtl) 
			set_key(_T("NUM_REFS"), _T("NSUBMTL"));
		else
			set_key(_T("NUM_REFS"), _T("0"));			
	}
	return TRUE;	// return FALSE if the dialog shouldn't be dismissed
}

BEGIN_MESSAGE_MAP(CCustom3Dlg, CAppWizStepDlg)
	//{{AFX_MSG_MAP(CCustom3Dlg)
	ON_BN_CLICKED(IDC_SDK_BROWSE, OnSdkBrowse)
	ON_BN_CLICKED(IDC_PLUGIN_BROWSE, OnPluginBrowse)
	ON_BN_CLICKED(IDC_EXE_BROWSE, OnExeBrowse)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCustom3Dlg::GetBrowseDirectory(TCHAR* title, CString& path)
{
	BROWSEINFO bfn;	
	TCHAR  path_name[MAX_PATH] = _T("");

    memset(&bfn, 0, sizeof(BROWSEINFO));
	bfn.hwndOwner = m_hWnd;
	bfn.pszDisplayName = path_name;
	bfn.lpszTitle = title;
	bfn.ulFlags = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST idl = SHBrowseForFolder(&bfn);

	if (idl)
	{
		SHGetPathFromIDList(idl, path_name);
		LPMALLOC pIMalloc;
		if (CoGetMalloc(MEMCTX_TASK, &pIMalloc) == S_OK)
		{
			pIMalloc->Free(idl);
			pIMalloc->Release();
		}
		path = path_name;
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CCustom3Dlg message handlers

void CCustom3Dlg::OnSdkBrowse() 
{
	if(GetBrowseDirectory("Choose MAXSDK folder", m_SdkPath))
		UpdateData(FALSE);	
}

void CCustom3Dlg::OnPluginBrowse() 
{
	if(GetBrowseDirectory("Choose plugins folder", m_PlgPath))
		UpdateData(FALSE);	
}

void CCustom3Dlg::OnExeBrowse() 
{
	if(GetBrowseDirectory("Choose 3dsmax.exe folder", m_ExePath))
		UpdateData(FALSE);
}

BOOL CCustom3Dlg::OnInitDialog() 
{
	CAppWizStepDlg::OnInitDialog();
	
	//UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CCustom3Dlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CAppWizStepDlg::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	long ec; 
	HKEY hKey; 
	DWORD type, len1, len2, len3;
	m_SdkPath = "";
	m_PlgPath = "";
	m_ExePath = "";
	m_ParamMaps = TRUE;
	
	// Get the path names from the registry
	ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE, appwz_key_name, 0, KEY_ALL_ACCESS, &hKey);
	if (ec == ERROR_SUCCESS)
	{
		unsigned char path[MAX_PATH];
		len1 = len2 = len3 = sizeof(path);		
		RegQueryValueEx(hKey, sdk_key_name,	0, &type, path, &len1);		
		m_SdkPath = path;
		RegQueryValueEx(hKey, plg_key_name, 0, &type, path, &len2);
		m_PlgPath = path;
		RegQueryValueEx(hKey, exe_key_name, 0, &type, path, &len3);	
		m_ExePath = path;		
		RegCloseKey(hKey); 
	}
	bPic.LoadBitmap(IDB_MAX3) ;
	pPicBMap = &bPic ;
	return 0;
}

void CCustom3Dlg::DrawBitmaps()
{
	CDC cpDcPic, *cdc;
	RECT rec ;
	m_PicFrame.GetWindowRect(&rec) ;
	cdc = m_PicFrame.GetDC() ;
	cpDcPic.CreateCompatibleDC(cdc);
	cpDcPic.SelectObject(pPicBMap) ;
	cdc->BitBlt(0,0,rec.right - rec.left, rec.bottom-rec.top,&cpDcPic,0,0,SRCCOPY) ;
}

void CCustom3Dlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
  		
   	DrawBitmaps();
  	// Forcing parameters maps the following types
	if (lookup_key("SIMPLE_TYPE")		||
		lookup_key("MATERIAL_TYPE")		||
		lookup_key("RENDER_EFFECT_TYPE")||
		lookup_key("TEX_TYPE")			||
		lookup_key("SHADOW_TYPE")		||
		lookup_key("SKIN_GIZMO_TYPE")	||
		lookup_key("MANIP_TYPE")		||
		lookup_key("UI_BY_MAX")) {
  		m_ParamMaps = TRUE;		
  		m_CtlMaps.EnableWindow(FALSE);
  	} else if (
  			lookup_key("STATIC_TYPE") ||
  			lookup_key("IMAGE_FILTER_COMPOSITOR_TYPE") ||
			lookup_key("GUP_TYPE") ||
			lookup_key("IK_TYPE") ||
			lookup_key("COLPICK_TYPE") ||
			lookup_key("FILE_EXPORT_TYPE") ||			
  			lookup_key("FILE_IMPORT_TYPE")) {
		set_key("NON_ANIM_TYPE", _T("YES"));
  		m_ParamMaps = FALSE;
  		m_CtlMaps.EnableWindow(FALSE);
  	} else {
  		m_CtlMaps.EnableWindow(TRUE);
	}
	if(lookup_key("MODIFIER_TYPE"))
		m_ctlExtCh.EnableWindow(TRUE);
	else
		m_ctlExtCh.EnableWindow(FALSE);

  	UpdateData(FALSE);
    // Do not call CAppWizStepDlg::OnPaint() for painting messages

}
