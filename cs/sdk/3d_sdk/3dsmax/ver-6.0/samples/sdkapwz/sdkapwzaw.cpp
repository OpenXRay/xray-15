// SDKAPWZaw.cpp : implementation file
//

#include "stdafx.h"
#include "SDKAPWZ.h"
#include "SDKAPWZaw.h"
#include "chooser.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// This is called immediately after the custom AppWizard is loaded.  Initialize
//  the state of the custom AppWizard here.
void CSDKAPWZAppWiz::InitCustomAppWiz()
{
	// Create a new dialog chooser; CDialogChooser's constructor initializes
	//  its internal array with pointers to the steps.
	m_pChooser = new CDialogChooser;

	// Set the maximum number of steps.
	SetNumberOfSteps(LAST_DLG);
	m_Dictionary[_T("PROJTYPE_DLL")] = _T("1");


	// TODO: Add any other custom AppWizard-wide initialization here.
}

// This is called just before the custom AppWizard is unloaded.
void CSDKAPWZAppWiz::ExitCustomAppWiz()
{
	// Deallocate memory used for the dialog chooser
	ASSERT(m_pChooser != NULL);
	delete m_pChooser;
	m_pChooser = NULL;

	// TODO: Add code here to deallocate resources used by the custom AppWizard
}

// This is called when the user clicks "Create..." on the New Project dialog
//  or "Next" on one of the custom AppWizard's steps.
CAppWizStepDlg* CSDKAPWZAppWiz::Next(CAppWizStepDlg* pDlg)
{
	// Delegate to the dialog chooser
	return m_pChooser->Next(pDlg);
}

// This is called when the user clicks "Back" on one of the custom
//  AppWizard's steps.
CAppWizStepDlg* CSDKAPWZAppWiz::Back(CAppWizStepDlg* pDlg)
{
	// Delegate to the dialog chooser
	return m_pChooser->Back(pDlg);
}

static CString GetPluginLibs(CString ext)
{
	CString libs, pmap;
	if(ext == _T("bmi") || ext == _T("bmf") || ext == _T("dlv") || ext == _T("bms") || ext == _T("dlh"))
			libs = _T("bmm.lib core.lib maxutil.lib maxscrpt.lib");
	else if(ext == _T("dlc"))
			libs = _T("core.lib expr.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib");
	else if(ext == _T("dle") || ext == _T("dlf") || ext == _T("dli"))
			libs = _T("core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib");
	else if(ext == _T("dlm") || ext == _T("dlo") || ext == _T("dlr") || ext == _T("dlt") || ext == _T("dlb") || ext == _T("dlk"))
			libs = _T("bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib");
	else if(ext == _T("dlu") || ext == _T("gup"))
		libs = _T("bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib");
	else if (ext == _T("flt"))
		libs = _T("bmm.lib core.lib flt.lib maxutil.lib maxscrpt.lib");
	
	//if (lookup_key("PARAM_MAPS", pmap))	
	libs += " paramblk2.lib";
	return libs;
}

void CSDKAPWZAppWiz::CustomizeProject(IBuildProject* pProject)
{
	// TODO: Add code here to customize the project.  If you don't wish
	//  to customize project, you may remove this virtual override.
	
	// This is called immediately after the default Debug and Release
	//  configurations have been created for each platform.  You may customize
	//  existing configurations on this project by using the methods
	//  of IBuildProject and IConfiguration such as AddToolSettings,
	//  RemoveToolSettings, and AddCustomBuildStep. These are documented in
	//  the Developer Studio object model documentation.

	// WARNING!!  IBuildProject and all interfaces you can get from it are OLE
	//  COM interfaces.  You must be careful to release all new interfaces
	//  you acquire.  In accordance with the standard rules of COM, you must
	//  NOT release pProject, unless you explicitly AddRef it, since pProject
	//  is passed as an "in" parameter to this function.  See the documentation
	//  on CCustomAppWiz::CustomizeProject for more information.
	
	//The following sets the project settings for 'Addtional include directories:'
	//maxsdk/include, maxsdk/lib and 'Output file name:' 3dsmax/plugin/*.*

	CString sPath, pPath, ePath, sInc, sRC, sL, sG, pName, pExt;
	IConfigurations *pConfigs;
	long count;
	IConfiguration *pConfig;		
	CString base_address;	

	// Generate a random 64K mutliple base address
	int K64 = 64*1024;
	base_address.Format(_T("0x%x"), ((rand() + rand() + ((rand()+(rand()<<16))))/K64)*K64);
	lookup_key(_T("root"),		pName);
	lookup_key(_T("PLUGEXT"),	pExt);
	lookup_key(_T("SDKPATH"),	sPath);
	lookup_key(_T("PLGPATH"),	pPath);
	lookup_key(_T("EXEPATH"),	ePath);
	
//	pName.SetAt(0, toupper(pName[0]));

	// 3dsmax.exe and maxsdk include paths
	sInc =  "/I" + sPath + "\\include "; 

	// Linker settings
	sL = 
		//output filename
		"/out:" + pPath + "\\" + pName + "." + pExt + " " +
			
		//sdk libs
		" comctl32.lib " + GetPluginLibs(pExt) + 

		//maxsdk libpath
		" /LIBPATH:" + sPath + "\\lib " + " /DLL /base:" + base_address;

	// General Settings
	sG = CString("0");		//To specify Not Using MFC

	BSTR bszComp = CString("cl.exe").AllocSysString();
	BSTR bszLink = CString("link.exe").AllocSysString();	
	BSTR bszMfc  = CString("mfc").AllocSysString();
	
	// Compiler settings
	BSTR bszSettingsC[] = { 
		("/MDd /G6 /LD " + sInc).AllocSysString(), // Debug
		("/MD /G6 /LD "  + sInc).AllocSysString(), // Hybrid
		("/MD /G6 /LD "  + sInc).AllocSysString()  // Release
		};

	BSTR bszRemSettingsC = CString(_T("/GX /D_MBCS /GZ")).AllocSysString();
	BSTR bszHyb = CString("Hybrid").AllocSysString();
	BSTR bszSettingsL = sL.AllocSysString();
	BSTR bszSettingsG = sG.AllocSysString();
	
	COleVariant res;

	pProject->AddConfiguration(bszHyb, res);
	pProject->get_Configurations(&pConfigs);		
	pConfigs->get_Count(&count);	
	for (long i = 1; i <= count; i++)
	{
		COleVariant varInd(i);
		pConfigs->Item(varInd , &pConfig);
		
		COleVariant var(0L,VT_ERROR);
		var.scode=DISP_E_PARAMNOTFOUND;

		pConfig->AddToolSettings(bszMfc, bszSettingsG, var);		
		pConfig->AddToolSettings(bszLink, bszSettingsL, var);
		
		COleVariant varStr;

		pConfig->get_Name(&V_BSTR(&varStr));
		
		varStr.vt = VT_BSTR;

		if (varStr == COleVariant(pName + " - Win32 Debug"))
		{
			pConfig->AddToolSettings(bszComp, bszSettingsC[0], var);
			SysFreeString(bszSettingsC[0]);
		}
		else if (varStr == COleVariant(pName + " - Win32 Hybrid"))
		{
			pConfig->AddToolSettings(bszComp, bszSettingsC[1], var);
			SysFreeString(bszSettingsC[1]);
		}
		else // release
		{
			pConfig->AddToolSettings(bszComp, bszSettingsC[2], var);
			SysFreeString(bszSettingsC[2]);

			// For release config add "/release" flag to linker settings
			BSTR rel = CString(_T("/release")).AllocSysString();
			pConfig->AddToolSettings(bszLink, rel, var);
			SysFreeString(rel);
		}
		pConfig->RemoveToolSettings(bszComp, bszRemSettingsC, var);		
		pConfig->Release();
	}
	SysFreeString(bszComp);
	SysFreeString(bszLink);
	SysFreeString(bszMfc);	
	SysFreeString(bszRemSettingsC);
	SysFreeString(bszHyb);
	SysFreeString(bszSettingsL);
	SysFreeString(bszSettingsG);	
	pConfigs->Release();

#if (1)
	//The following opens the Root.cpp document in the DevStudio
	IApplication *pApp;
	IDocuments *pDocs;
	IDocuments *pDoc;
	BSTR bszRoot = (pName + CString(_T(".cpp"))).AllocSysString();
	COleVariant varOpenAs(CString(_T("Text")));
	COleVariant varReadOnly(0L, VT_BOOL);

	pProject->get_Application((IDispatch **)&pApp);
	pApp->get_Documents((IDispatch **)&pDocs);
	pDocs->Open(bszRoot, varOpenAs, varReadOnly, (IDispatch **)&pDoc);
	
	SysFreeString(bszRoot);
	pApp->Release();
	pDocs->Release();
	pDoc->Release();
#endif
}


// Here we define one instance of the CSDKAPWZAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global SDKAPWZaw.
CSDKAPWZAppWiz SDKAPWZaw;
// Some useful functions
BOOL lookup_key(CString key, CString& val)
{
	return SDKAPWZaw.m_Dictionary.Lookup(key, val);
}

void set_key(CString key, CString val)
{
	SDKAPWZaw.m_Dictionary.SetAt(key, val);
}

BOOL remove_key(CString key)
{
	return SDKAPWZaw.m_Dictionary.RemoveKey(key);
}

CStringArray ClassList;