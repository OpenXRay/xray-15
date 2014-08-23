/*===========================================================================*\
 | 
 |  FILE:	Plugin Management Tool
 |			3D Studio MAX R4.0
 | 
 |  AUTH:   Harry Denholm
 |			Ravi Karra
 |			All Rights Reserved
 |
 |  HIST:	Started 19-10-99
 | 
\*===========================================================================*/


#include "plugman.h"
#include "utillib.h"

extern PlugMgrUtility sPlugMgrUtility; //in plugman.cpp

/*===========================================================================*\
 |	Used for swap in-out at load time to refresh command UI
\*===========================================================================*/

class nullCD:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create( BOOL loading ) { return NULL; }
	const TCHAR *	ClassName() { return _T("NULL"); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID 		ClassID() { return Class_ID(0x35591eda, 0x596b602c); }
	const TCHAR* 	Category() { return _T("");  }
};
static nullCD theNullCD;


void DeepRefreshUI(Interface *ip)
{
		// for commandpanel update
		ip->AddClass(&theNullCD);
		ip->DeleteClass(&theNullCD);

		GetCUIFrameMgr()->RecalcLayout(TRUE);
}



/*===========================================================================*\
 |	Search and match for a good DllDesc from given name
\*===========================================================================*/

DllDesc *GetDescFromPlugname(TCHAR *s, Interface *ip)
{
	DllDir *dd = ip->GetDllDirectory();

	int gotIdx = -2;
	for(int a=0;a<dd->Count();a++)
	{
		DllDesc *tsDesc = &(*dd)[a];
		if(_tcsicmp(tsDesc->fname,s)==0) 
		{
			gotIdx = a;
		}
	}
	if(gotIdx!=-2)
	{
		DllDesc *myDesc = &(*dd)[gotIdx];
		return myDesc;
	}
	else return NULL;
}

/*===========================================================================*\
 |	Check for unloadability, and then free DLL handle and mark as unloaded
\*===========================================================================*/

bool UnloadDLL( TCHAR *s, HWND hWnd, Interface *ip)
{
	int i;
	DllDir *dd = ip->GetDllDirectory();
	DllDesc *myDesc = GetDescFromPlugname(s,ip);

	ComputeClassUse(ip);

	TCHAR erName[255];
	sprintf(erName,_T("%s : %s"), GetString(IDS_UNLOAD_ERROR), s);

	if(myDesc)
	{
		if(myDesc->loaded)
		{

			int na=0;
			for(i=0;i<myDesc->NumberOfClasses();i++)
			{
				ClassDesc *myCD = (*myDesc)[i];
				if (!myCD) continue; //RK:07/20/00, there are cases where ClassDesc is NULL
				ClassEntry *myCE = dd->ClassDir().FindClassEntry(myCD->SuperClassID(),myCD->ClassID());
				if(myCE) { na+=myCE->UseCount(); }
			}

			if(na==0)
			{
				int noc = myDesc->NumberOfClasses();

				//ClassDesc** unloadables = new ClassDesc[noc];
				//unloadables.ZeroCount();

				//for(i=0;i<noc;i++)
				//{
				//	ClassDesc *myCD = (*myDesc)[i];
				//	unloadables[i] = &myCD;
				//}

				int unloadCount = 0;
				for(i=0;i<noc;i++)
				{
					ClassDesc* cd = (*myDesc)[i];
					if (!cd) continue; //RK:07/20/00, there are cases where ClassDesc is NULL
					unloadCount += (ip->DeleteClass(cd));
				}

				if(unloadCount != noc)
				{
					MessageBox(hWnd,GetString(IDS_UNLOAD_FAILED),erName,MB_OK);
					return true;
				}

				// save directory and path information into cache variables
				TCHAR out[MAX_PATH];
				GetModuleFileName(myDesc->handle,out,MAX_PATH-1);
				TCHAR path[MAX_PATH];
				BMMSplitFilename(out,path,NULL,NULL);

				myDesc->directory.printf("%s",path);
				myDesc->tDescription = myDesc->Description();


				// remove it from the newly loaded list
				for (int n = sPlugMgrUtility.newlyLoaded.Count()-1; n >= 0; n++)
					if (sPlugMgrUtility.newlyLoaded[n] == myDesc) sPlugMgrUtility.newlyLoaded.Delete(n, 1);

				// unload the DLL handle
				myDesc->Free();
				myDesc->loaded = FALSE;

			}
			else
			{
				MessageBox(hWnd,GetString(IDS_CANNOT_UNLOAD),erName,MB_OK);
				return false;
			}

		}
		else
		{
			MessageBox(hWnd,GetString(IDS_ALREADY_UNLOADED),erName,MB_OK);
			return false;
		}
	}
	else
	{
		MessageBox(hWnd,GetString(IDS_DLLDESC_NOT_FOUND),erName,MB_OK);
		return false;
	}
	return true;
}

/*===========================================================================*\
 |	Load a new DLL with a file selector
\*===========================================================================*/

bool LoadNewDLL( HWND hWnd, Interface *ip)
{
		TCHAR FileName[MAX_PATH];
		TCHAR FileTitle[100];
		FilterList FileFilter;

		OPENFILENAME ofn;
		memset(&ofn,0,sizeof(ofn));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner=hWnd;
		ofn.hInstance=hInstance;
#ifdef DESIGN_VER
		FileFilter.Append(GetString(IDS_MAX_PLUGINS_VIZ));
#else
		FileFilter.Append(GetString(IDS_MAX_PLUGINS));
#endif // DESIGN_VER
		FileFilter.Append(_T("*.DL?;*.BM?;*.FLT"));
		FileFilter.Append(GetString(IDS_ALL_FILES));
		FileFilter.Append(_T("*.*"));
		ofn.lpstrFilter = FileFilter;

		ofn.lpstrCustomFilter=NULL;
		ofn.nMaxCustFilter=0;
		ofn.nFilterIndex=1;
		ofn.lpstrFile=FileName;
		ofn.nMaxFile=500;
		ofn.lpstrFileTitle = FileTitle;
		ofn.nMaxFileTitle=99;
		ofn.lpstrInitialDir=NULL;
		ofn.lpstrTitle = GetString(IDS_CHOOSE_PLUGIN);
		ofn.Flags = OFN_FILEMUSTEXIST;

		FileName[0]='\0';
		GetOpenFileName(&ofn);

		if(FileName[0]=='\0'){return false;}
		DllDir *dd = ip->GetDllDirectory();

		TCHAR path[MAX_PATH];
		TCHAR file[MAX_PATH];
		TCHAR ext[50];
		BMMSplitFilename(FileName,path,file,ext);
		_tcscat(file,ext);

		BOOL alreadyLoaded = FALSE;
		//TCHAR out[MAX_PATH];		
		for(int q=0;q<dd->Count();q++)
		{	
			DllDesc *dsc = &(*dd)[q];
			//GetModuleFileName(dsc->handle,out,MAX_PATH-1);
			if(_tcsicmp(file,dsc->fname)==0) 
				alreadyLoaded = TRUE;

			if(_tcsicmp(dsc->fname,file)==0) dsc->fname = file;
		}
		if(alreadyLoaded)
		{
			TSTR erName = GetString(IDS_LOAD_ERROR);
			MessageBox(hWnd,GetString(IDS_ALREADY_LOADED),erName,MB_OK);
			return false;
		}
		else
		{
			return dd->LoadADll(file,TRUE);			
		}
	return true;
}

/*===========================================================================*\
 |	Load an existing DLL
\*===========================================================================*/

bool LoadExistingDLL( TCHAR *s, HWND hWnd, Interface *ip)
{
		DllDir *dd = ip->GetDllDirectory();

		TCHAR theDir[MAX_PATH];

		BOOL alreadyLoaded = FALSE;
		for(int q=0;q<dd->Count();q++)
		{	
			DllDesc *dsc = &(*dd)[q];
			if(_tcsicmp(dsc->fname,s)==0)
			{
				_tcscpy(dsc->fname,s);
				_tcscpy(theDir,dsc->directory);
			}
		}
		if(alreadyLoaded)
		{
			MessageBox(hWnd,GetString(IDS_ALREADY_LOADED),GetString(IDS_LOAD_ERROR),MB_OK);
			return false;
		}
		else
		{
			TCHAR fn[MAX_PATH];
			sprintf(fn,"%s%s",theDir,s);
			return dd->LoadADll(fn,TRUE);
		}	
}


/*===========================================================================*\
 |	Check to see if a plugin has active ties to the superclasslists
\*===========================================================================*/

BOOL pluginHasClassesRegistered( int masterCode, Interface *ip )
{
	DllDir *dd = ip->GetDllDirectory();
	ClassDirectory *cd = &dd->ClassDir();

	for(int i=0;i<cd->Count();i++)
	{
		SubClassList *scl = &(*cd)[i];

		for(int q=0;q<scl->Count(ACC_ALL);q++)
		{
			ClassEntry *ce = &(*scl)[q];
			if(ce->DllNumber() == masterCode) return TRUE;
		}
	}
	return FALSE;
}






/*===========================================================================*\
 |	REDEFER CODE - WORK IN PROGRESS
\*===========================================================================*/


#define VENDORNAMELENGTH 256
#define PLUGINDESCRLENGTH 256
#define LIBDESCLENGTH 256
#define CLASSSTRINGLENGTH 256
#define VALUEDATALENGTH 256

#define USE_UTIL_REG_KEY

#ifdef USE_UTIL_REG_KEY
#define DEFERLOADREGKEY0_3	UtilityInterface::GetRegistryKeyBase()
#define DEFERLOADREGKEY4	GetString(IDS_PM_REG_PLUGINS)

#else	// USE_UTIL_REG_KEY
// Registry place for deferred loading

#define DEFERLOADREGKEY0	_T("SOFTWARE")
#define DEFERLOADREGKEY1	_T("Autodesk")
#ifdef DESIGN_VER
#define DEFERLOADREGKEY2	_T("VIZ")
#else
#define DEFERLOADREGKEY2	_T("3ds max")
#endif
#define DEFERLOADREGKEY3	_T("5.0")
#define DEFERLOADREGKEY4	_T("Plug-ins")

#endif // USE_UTIL_REG_KEY

static HKEY keychain[8];

LONG FindRegPlace() {
    long ok;
#define CLASSSTRINGLENGTH 256

#ifdef USE_UTIL_REG_KEY
    ok = RegOpenKeyEx(HKEY_CURRENT_USER,
                      DEFERLOADREGKEY0_3,
                      0,
                      KEY_READ,
                      &keychain[3]);

#else	// if not USE_UTIL_REG_KEY

    ok = RegOpenKeyEx(HKEY_CURRENT_USER,
                      DEFERLOADREGKEY0,
                      0,
                      KEY_READ,
                      &keychain[0]);
    if (ok == ERROR_SUCCESS) {
        ok = RegOpenKeyEx(keychain[0],
                          DEFERLOADREGKEY1,
                          0,
                          KEY_READ,
                          &keychain[1]);
        if (ok == ERROR_SUCCESS) {
            ok = RegOpenKeyEx(keychain[1],
                              DEFERLOADREGKEY2,
                              0,
                              KEY_READ,
                              &keychain[2]);
            if (ok == ERROR_SUCCESS) {
                ok = RegOpenKeyEx(keychain[2],
                                  DEFERLOADREGKEY3,
                                  0,
                                  KEY_READ,
                                  &keychain[3]);
#endif	// USE_UTIL_REG_KEY
                if (ok == ERROR_SUCCESS) {
                    ok = RegOpenKeyEx(keychain[3],
                                       DEFERLOADREGKEY4,
                                       0,
                                       KEY_READ,
                                       &keychain[4]);
                    if (ok == ERROR_SUCCESS)
                        return ok;
                    RegCloseKey(keychain[3]);
                    }
#ifndef USE_UTIL_REG_KEY
                RegCloseKey(keychain[2]);
                }
            RegCloseKey(keychain[1]);
            }
        RegCloseKey(keychain[0]);
        }
#endif	// USE_UTIL_REG_KEY
    return ok;
    }

void LoseRegPlace() {
    RegCloseKey(keychain[4]);
    RegCloseKey(keychain[3]);
#ifndef USE_UTIL_REG_KEY
    RegCloseKey(keychain[2]);
    RegCloseKey(keychain[1]);
    RegCloseKey(keychain[0]);
#endif	// USE_UTIL_REG_KEY
    }

bool GetRegSZ(TCHAR *key, TSTR &val) {
	DWORD valueDataLength = VALUEDATALENGTH;
	TCHAR valueData[VALUEDATALENGTH];
	DWORD valueType;

	if (RegQueryValueEx(keychain[7],
						key,
						NULL,
						&valueType,
						(unsigned char *)&valueData,
						&valueDataLength)
				== ERROR_SUCCESS
			&& valueType == REG_SZ) {
		val = valueData;
		return TRUE;
		}
	return FALSE;
	}

bool GetRegDW(TCHAR *key, DWORD &val) {
	DWORD dwordDataLength = 4;
	DWORD dwordData;
	DWORD valueType;

	if (RegQueryValueEx(keychain[7],
						key,
						NULL,
						&valueType,
						(unsigned char *)&dwordData,
						&dwordDataLength)
				== ERROR_SUCCESS
			&& valueType == REG_DWORD) {
		val = dwordData;
		return TRUE;
		}
	return FALSE;
	}
