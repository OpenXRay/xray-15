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

#include "PlugMan.h"
#include "notify.h"

static int listHeight,pdHeight,pdWidth;

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

extern GetModuleInformation getModuleInformation;

/*===========================================================================*\
 |	Popup the cute DLL actions menu
\*===========================================================================*/

void showDLLActions(HWND hwnd, int x, int y) {
	HMENU hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_DLLMENU));
	HMENU subMenu = GetSubMenu(hMenu, 0);
	TrackPopupMenu(subMenu, TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
		x-2, y, 0, hwnd, NULL);
	DestroyMenu(subMenu);
	DestroyMenu(hMenu);		

	}



/*===========================================================================*\
 |	Action enumerators for the selected plugins
\*===========================================================================*/

class unloadBAC : public BitArrayCallback
{
public:
	PlugMgrUtility *pmu;
	void proc(int n)
	{
		DllDir *dd = pmu->ip->GetDllDirectory();
		DllDesc *myDesc = pmu->ddList[n].ddesc;
		pmu->recentArray.Set(pmu->ddList[n].realIdx);

		if(myDesc)
		{
			if(myDesc->loaded)
			{
				UnloadDLL(myDesc->fname,pmu->popup1,pmu->ip);
			}
		}
	}
};

unloadBAC unloadProcessor;


class loadBAC : public BitArrayCallback
{
public:
	PlugMgrUtility *pmu;
	void proc(int n)
	{
		DllDir *dd = pmu->ip->GetDllDirectory();
		DllDesc *myDesc = pmu->ddList[n].ddesc;
		pmu->recentArray.Set(pmu->ddList[n].realIdx);

		if(myDesc)
		{
			if(!myDesc->loaded)
			{
				LoadExistingDLL(myDesc->fname,pmu->popup1,pmu->ip);
			}
		}
	}
};

loadBAC loadProcessor;


/*===========================================================================*\
 |	Action enumerators for the tag'd plugins
\*===========================================================================*/


void dbgMsg(const TCHAR *format, ...)
{
	TCHAR buf[512];
	va_list args;
	va_start(args,format);
	_vsntprintf(buf,512,format,args);
	va_end(args);

	MessageBox(NULL,buf,"Debug Msg",MB_OK);
}


class tagunloadBAC : public BitArrayCallback
{
public:
	PlugMgrUtility *pmu;
	void proc(int n)
	{
		DllDir *dd = pmu->ip->GetDllDirectory();
		
		int resolvedTag = -1;
		for(int a=0;a<pmu->ddList.Count();a++)
		{
			if(pmu->ddList[a].realIdx == n) { resolvedTag = a; }
		}

		if(resolvedTag!=-1)
		{
			DllDesc *myDesc = pmu->ddList[resolvedTag].ddesc;

			if(myDesc)
			{
				if(myDesc->loaded)
				{
					UnloadDLL(myDesc->fname,pmu->popup1,pmu->ip);
				}
			}
		}
	}
};

tagunloadBAC tagunloadProcessor;


class tagloadBAC : public BitArrayCallback
{
public:
	PlugMgrUtility *pmu;
	void proc(int n)
	{
		DllDir *dd = pmu->ip->GetDllDirectory();

		int resolvedTag = -1;
		for(int a=0;a<pmu->ddList.Count();a++)
		{
			if(pmu->ddList[a].realIdx == n) { resolvedTag = a; }
		}

		if(resolvedTag!=-1)
		{
			DllDesc *myDesc = pmu->ddList[resolvedTag].ddesc;

			if(myDesc)
			{
				if(!myDesc->loaded)
				{
					LoadExistingDLL(myDesc->fname,pmu->popup1,pmu->ip);
				}
			}
		}
	}
};

tagloadBAC tagloadProcessor;

class tagselBAC : public BitArrayCallback
{
public:
	PlugMgrUtility *pmu;
	void proc(int n)
	{
		pmu->recentArray.Set(pmu->ddList[n].realIdx);
	}
};

tagselBAC tagselProcessor;



/*===========================================================================*\
 |	BitArray Emulation that maps to my ListView
\*===========================================================================*/

void PVSelectionEnumerator( HWND hWnd, BitArrayCallback &bac)
{
	HWND lv = GetDlgItem(hWnd,IDC_LIST1);
	int numItem = ListView_GetItemCount(lv);

	for(int i=0;i<numItem;i++)
	{
		if( (ListView_GetItemState(lv,i,LVIS_SELECTED))&LVIS_SELECTED )
		{
			bac.proc(i);
		}
	}

}


/*===========================================================================*\
 |	Column naming and tagging
\*===========================================================================*/

TCHAR *GetColumnName( int i )
{
	switch (i)
	{
		case 0: return GetString(IDS_TAG); break;
		case 1: return GetString(IDS_NAME); break;
		case 2: return GetString(IDS_DESCRIPTION); break;
		case 3: return GetString(IDS_STATUS); break;
		case 4: return GetString(IDS_SIZE); break;
		case 5: return GetString(IDS_FULLPATH); break;
	}
	return "-invalidindex-";
}

void retagColumns( HWND hWnd, int cold, int cnew )
{
	LV_COLUMN lvcolumn;
	lvcolumn.mask = LVCF_TEXT;
	TCHAR cmTitle[255];
						
	HWND lv = GetDlgItem(hWnd,IDC_LIST1);

	// restore old name
	_tcscpy(cmTitle,GetColumnName(cold));
	lvcolumn.pszText = cmTitle;
	ListView_SetColumn(lv,cold,&lvcolumn);

	// change new item to tagged name
	_tcscpy(cmTitle,GetColumnName(cnew));
	_tcscat(cmTitle," *");
	lvcolumn.pszText = cmTitle;
	ListView_SetColumn(lv,cnew,&lvcolumn);
}

static PlugMgrUtility *tmpPmu;
/*===========================================================================*\
 |	Sorting routines for our DLLDir cache list
\*===========================================================================*/
int __cdecl nameFilter( const void *arg1, const void *arg2 )
{
	plugCache *d1 = ((plugCache*)arg1);
	plugCache *d2 = ((plugCache*)arg2);

	if(d1&&d2)
	{
/*		for (int i=0; i < tmpPmu->ddList.Count(); i++)
		{
			if (&tmpPmu->ddList[i] == d1)
				DebugPrint(" .....d1:%i\n", i);
			if (&tmpPmu->ddList[i] == d2)
				DebugPrint(" .....d2:%i\n", i);
		}
*/
		return _tcsicmp(d1->ddesc->fname, d2->ddesc->fname);
	}
	else return 0;
}

int __cdecl pathFilter( const void *arg1, const void *arg2 )
{
	plugCache *d1 = ((plugCache*)arg1);
	plugCache *d2 = ((plugCache*)arg2);

	if(d1&&d2)
	{
		return _tcsicmp(d1->ddesc->directory, d2->ddesc->directory);
	}
	else return 0;
}

int __cdecl descFilter( const void *arg1, const void *arg2 )
{
	plugCache *d1 = ((plugCache*)arg1);
	plugCache *d2 = ((plugCache*)arg2);

	if(d1&&d2)
	{
		TSTR dsc1;
		TSTR dsc2;
		if(d1->ddesc->loaded==TRUE) dsc1 = d1->ddesc->Description();
		else dsc1 = d1->ddesc->tDescription;
		if(d2->ddesc->loaded==TRUE) dsc2 = d2->ddesc->Description();
		else dsc2 = d2->ddesc->tDescription;

		return _tcsicmp(dsc1,dsc2);
	}
	else return 0;
}



int __cdecl sizeFilter( const void *arg1, const void *arg2 )
{
	plugCache *d1 = ((plugCache*)arg1);
	plugCache *d2 = ((plugCache*)arg2);

	if(d1&&d2)
	{

		if(!d1->ddesc->loaded&&d2->ddesc->loaded) return 1;
		if(d1->ddesc->loaded&&!d2->ddesc->loaded) return -1;
		if(!d1->ddesc->loaded&&!d2->ddesc->loaded) return 0;

		return (d1->bytesize - d2->bytesize );
	}
	else return 0;
}

int __cdecl loadFilter( const void *arg1, const void *arg2 )
{
	plugCache *d1 = ((plugCache*)arg1);
	plugCache *d2 = ((plugCache*)arg2);

	if(d1&&d2)
	{
		if(!d1->ddesc->loaded&&d2->ddesc->loaded) return 1;
		if(d1->ddesc->loaded&&!d2->ddesc->loaded) return -1;
		if(!d1->ddesc->loaded&&!d2->ddesc->loaded) return 0;
	}
	
	return 0;
}

int __cdecl tagFilter( const void *arg1, const void *arg2 )
{
	plugCache *d1 = ((plugCache*)arg1);
	plugCache *d2 = ((plugCache*)arg2);

	if(d1&&d2&&tmpPmu)
	{
		if(!tmpPmu->recentArray[d1->realIdx]&&tmpPmu->recentArray[d2->realIdx]) return 1;
		if(tmpPmu->recentArray[d1->realIdx]&&!tmpPmu->recentArray[d2->realIdx]) return -1;
		if(!tmpPmu->recentArray[d1->realIdx]&&!tmpPmu->recentArray[d2->realIdx]) return 0;
	}
	
	return 0;
}


void resortCache( PlugMgrUtility *pmu )
{
	tmpPmu = pmu;
	switch(pmu->columnActive)
	{
		case 0: pmu->ddList.Sort(tagFilter); break;
		case 1: pmu->ddList.Sort(nameFilter); break;
		case 2: pmu->ddList.Sort(descFilter); break;
		case 3: pmu->ddList.Sort(loadFilter); break;
		case 4: pmu->ddList.Sort(sizeFilter); break;
		case 5: pmu->ddList.Sort(pathFilter); break;
	}
}



/*===========================================================================*\
 |	Rebuild the internal representation of the master DLL Directory
\*===========================================================================*/

void rebuildCache( Interface *ip, PlugMgrUtility *pmu, HWND hWnd )
{
	DllDir *dd = ip->GetDllDirectory();
	pmu->ddList.ZeroCount();

	HWND pd = GetDlgItem(hWnd,IDC_PLUGDIR);
	int numItem = ListView_GetItemCount(pd);

	plugCache pcItem;

	if(!dd) dbgMsg("PMU:rebuildCache:WARNING WARNING! NO Valid DD Found");


	// open up MAX's process handle for module polling
	DWORD processID = 1;
	DWORD threadID = GetWindowThreadProcessId(pmu->ip->GetMAXHWnd(),&processID);
	HANDLE hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, processID);
	
	for(int i=0;i<dd->Count();i++)
	{
		DllDesc *myDesc = &(*dd)[i];

		if(myDesc->loaded&& ( _tcsicmp(myDesc->directory,"")==0))
		{
				TCHAR out[MAX_PATH];
				GetModuleFileName(myDesc->handle,out,MAX_PATH-1);
				TCHAR path[MAX_PATH];
				BMMSplitFilename(out,path,NULL,NULL);
				myDesc->directory.printf("%s",path);
		}


		BOOL addMe = FALSE;
		for(int a=0;a<numItem;a++)
		{
			BOOL onOff = ListView_GetCheckState(pd,a);
			if(onOff)
			{
			TCHAR ptmp[MAX_PATH];
			ListView_GetItemText(pd,a,1,ptmp,MAX_PATH-1);

			if(_tcsicmp(ptmp,myDesc->directory)==0)  addMe = TRUE;
			}
		}
		if (!addMe)
		{
			// see if it is newly loaded
			for(int n=0; n < pmu->newlyLoaded.Count(); n++)
				if (myDesc == pmu->newlyLoaded[n])
				{
					addMe = TRUE;
					break;
				}
		}

		// make sure that the Plugin Manager is not added to the list
		// coz we don't want ourselves unloadable!!
		BOOL noadd = FALSE;
		if(myDesc->loaded)
		{
			for(int q=0;q<myDesc->NumberOfClasses();q++)
			{
				ClassDesc* cd = (*myDesc)[q];
				if (!cd) continue; //RK:07/20/00, there are cases where ClassDesc is NULL
				if(cd->ClassID() == PLUGIN_MANAGER_CLASSID) noadd = TRUE;
			}
		}

		if(!noadd&&addMe) 
		{
			int linSize=-1;
			if (PlugMgrUtility::hPsapi) {
				MODULEINFO mi;
				ZeroMemory(&mi, sizeof(MODULEINFO));
				getModuleInformation(hProcess,myDesc->handle,&mi,sizeof(MODULEINFO));
				linSize = (mi.SizeOfImage/1024);
			}

			// determine the true DD list index, for analysis purposes 
			// as the index into our ddlist will be faked if any filtering is on
			int realIdx;
			for(int z=0;z<dd->Count();z++)
			{
				if(myDesc == &(*dd)[z]) { realIdx = z; goto contc; }
			}
			contc:

			pcItem.ddesc = myDesc;
			pcItem.bytesize = linSize;
			pcItem.realIdx = realIdx;
			pmu->ddList.Append(1,&pcItem);
		}

	}

	// update tag array, n' clear it
	int scMax = dd->Count();
	pmu->recentArray.SetSize(scMax,1);

	CloseHandle(hProcess);

	resortCache( pmu );
}

void HandleSize(HWND hWnd,LPARAM lParam)
{
	HWND lv = GetDlgItem(hWnd,IDC_LIST1);
	HWND pd = GetDlgItem(hWnd,IDC_PLUGDIR);
    MoveWindow( lv, 0, 1, LOWORD(lParam), HIWORD(lParam)-listHeight, FALSE );

    MoveWindow( pd, 0, (HIWORD(lParam)-listHeight)+6, LOWORD(lParam)-pdWidth, pdHeight, FALSE );

	InvalidateRect(hWnd,NULL,TRUE);
}


/*===========================================================================*\
 |	Dialog Handler for Utility
\*===========================================================================*/

BOOL CALLBACK PMDefaultDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PlugMgrUtility *pmu = (PlugMgrUtility*)GetWindowLong(hWnd,GWL_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			pmu = (PlugMgrUtility*)lParam;
			SetWindowLong(hWnd,GWL_USERDATA,(LONG)pmu);
			GetCOREInterface()->RegisterDlgWnd(hWnd);
			pmu->Init(hWnd);

			// record size of initial listviewer
			RECT rc; GetWindowRect(GetDlgItem(hWnd,IDC_LIST1),&rc);
			RECT rc2; GetWindowRect(GetDlgItem(hWnd,IDC_PLUGDIR),&rc2);
			RECT cs; GetClientRect(hWnd,&cs);
			listHeight = (cs.bottom-cs.top)-(rc.bottom-rc.top);
			pdWidth = (cs.right-cs.left)-(rc2.right-rc2.left);
			pdHeight = (rc2.bottom-rc2.top);
			HandleSize(hWnd,MAKELPARAM((cs.right-cs.left),(cs.bottom-cs.top)));
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLong(pmu->ip->GetMAXHWnd(), GCLP_HICONSM));
			break;

		case WM_DESTROY:
			GetCOREInterface()->UnRegisterDlgWnd(hWnd);			
			if(pmu) { pmu->Destroy(hWnd); pmu->popup1 = NULL; }
			break;
		

		case WM_RBUTTONDOWN:
			{
				POINT lpPt; GetCursorPos(&lpPt);
				showDLLActions(hWnd,lpPt.x,lpPt.y);
			}
			break;

		case WM_NOTIFY:
			{
				if(pmu->UI_active)
				{
					NMHDR *nm = (LPNMHDR) lParam;
					if(nm->code == NM_RCLICK&&nm->idFrom==IDC_LIST1)
					{
						POINT lpPt; GetCursorPos(&lpPt);
						showDLLActions(hWnd,lpPt.x,lpPt.y);
					}
					if(nm->code == LVN_ITEMCHANGED&&nm->idFrom==IDC_PLUGDIR)
					{
						BOOL needRebuild = FALSE;
						HWND pd = GetDlgItem(hWnd,IDC_PLUGDIR);
						int numItem = ListView_GetItemCount(pd);

						for(int a=0;a<numItem;a++)
						{
							BOOL bTmp = ListView_GetCheckState(pd,a);
							if(pmu->pdrec[a] != bTmp) needRebuild = TRUE;
							pmu->pdrec[a] = bTmp;
						}
						if(needRebuild)
						{
							rebuildCache(pmu->ip,pmu,hWnd);
							PVRefresh( hWnd, pmu, FALSE);
						}
					}
					if(nm->code == LVN_COLUMNCLICK&&nm->idFrom==IDC_LIST1)
					{
						NMLISTVIEW *pnmv = (LPNMLISTVIEW) lParam;

						int remAst = pmu->columnActive;
						pmu->columnActive = pnmv->iSubItem;

						retagColumns(hWnd,remAst,pmu->columnActive);

						rebuildCache(pmu->ip,pmu,hWnd); //RK: must have, cache might be outdated
						resortCache(pmu);
						PVRefresh( hWnd, pmu, TRUE);
					}
				}
			}
			break;

		case WM_COMMAND:
			if(pmu)
			{

			switch (LOWORD(wParam)) {


			case ID_CLEAR:{
					ListView_SetItemState(GetDlgItem(hWnd,IDC_LIST1),-1,0,LVIS_SELECTED);
				break;}

			case ID_DLLMENU_LOAD:{
					pmu->recentArray.ClearAll();
					if (LoadNewDLL(hWnd,pmu->ip))
					{
						DllDir *dd = pmu->ip->GetDllDirectory();
						DllDesc *dsc = &(*dd)[dd->Count()-1];
						pmu->newlyLoaded.Append(1,&dsc);
						DeepRefreshUI(pmu->ip);
						rebuildCache(pmu->ip,pmu,hWnd);
						PVRefresh(hWnd,pmu,FALSE);
						}
				break;}

			case ID_DLLMENU_LOADSEL:{
					//pmu->recentArray.ClearAll();
					loadProcessor.pmu = pmu;
					PVSelectionEnumerator(hWnd,loadProcessor);
					ListView_SetItemState(GetDlgItem(hWnd,IDC_LIST1),-1,0,LVIS_SELECTED);
					DeepRefreshUI(pmu->ip);
					rebuildCache(pmu->ip,pmu,hWnd);
					PVRefresh(hWnd,pmu,TRUE);
				break;}

			case ID_DLLMENU_UNLOAD:{
					//pmu->recentArray.ClearAll();
					unloadProcessor.pmu = pmu;
					PVSelectionEnumerator(hWnd,unloadProcessor);
					ListView_SetItemState(GetDlgItem(hWnd,IDC_LIST1),-1,0,LVIS_SELECTED);
					rebuildCache(pmu->ip,pmu,hWnd);
					PVRefresh(hWnd,pmu,TRUE);
				break;}


			case ID_TAG_DEFER:{
				break;
							  }

			case ID_TAG_LOAD:{
					tagloadProcessor.pmu = pmu;
					pmu->recentArray.EnumSet(tagloadProcessor);
					rebuildCache(pmu->ip,pmu,hWnd);
					DeepRefreshUI(pmu->ip);
					PVRefresh(hWnd,pmu,TRUE);
				break;}

			case ID_TAG_UNLOAD:{
					tagunloadProcessor.pmu = pmu;
					pmu->recentArray.EnumSet(tagunloadProcessor);
					rebuildCache(pmu->ip,pmu,hWnd);
					PVRefresh(hWnd,pmu,TRUE);
				break;}

			case ID_TAG_FROMSEL:{
					tagselProcessor.pmu = pmu;
					PVSelectionEnumerator(hWnd,tagselProcessor);
					resortCache(pmu);
					//rebuildCache(pmu->ip,pmu,hWnd);
					PVRefresh(hWnd,pmu,TRUE);
				break;}

			case ID_TAG_CLEAR:{
					pmu->recentArray.ClearAll();
					resortCache(pmu);
					PVRefresh(hWnd,pmu,TRUE);
					
				break;}


			case ID_REFRESH:{
					InvalidateRect(hWnd,NULL,FALSE);
					rebuildCache(pmu->ip,pmu,hWnd);
					PVRefresh(hWnd,pmu,FALSE);
				break;}

			}
			}
			break;

		case WM_CLOSE:
			pmu->popup1 = NULL;
			EndDialog(hWnd,0);
			break;

		case WM_SIZE:
			HandleSize(hWnd,lParam);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


void AddLVItem(HWND lv, int index, TCHAR **items, int count)
{
	LV_ITEM item;
	item.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_INDENT;
	item.iItem = index;
	item.iSubItem = 0;
	item.pszText = items[0];
	item.cchTextMax = _tcslen(items[0]);

	int img = -1;
	if(_tcsicmp(items[0],"+")==0)
	{
		img = 3;
		item.pszText = "";
		item.cchTextMax = 0;
	}
	item.iImage = img;
	item.iIndent = 1;
	ListView_InsertItem(lv, &item);

	for(int i=1;i<count;i++)
	{
		item.mask = LVIF_TEXT|LVIF_IMAGE;
		if(i==3)
		{
			int img = -1;
			if(_tcsicmp(items[i],GetString(IDS_LOADED))==0) img = 2;
			if(_tcsicmp(items[i],GetString(IDS_UNLOADED))==0) img = 0;
			if(_tcsicmp(items[i],GetString(IDS_DEFERRED))==0) img = 1;
			item.iImage = img;
		}
		else
		{
			item.iImage = -1;
		}
		item.iItem = index;
		item.iSubItem = i;
		item.pszText = items[i];
		item.cchTextMax = _tcslen(items[i]);
		ListView_SetItem(lv, &item);
	}
}

void SetLVItem(HWND lv, int index, TCHAR **items, int count)
{
	LV_ITEM item;
	for(int i=0;i<count;i++)
	{
		item.mask = LVIF_TEXT|LVIF_IMAGE;
		item.iItem = index;
		item.iSubItem = i;
		item.pszText = items[i];
		item.cchTextMax = _tcslen(items[i]);

		if(i==3)
		{
			int img = -1;
			if(_tcsicmp(items[i],GetString(IDS_LOADED))==0) img = 2;
			if(_tcsicmp(items[i],GetString(IDS_UNLOADED))==0) img = 0;
			if(_tcsicmp(items[i],GetString(IDS_DEFERRED))==0) img = 1;
			item.iImage = img;
		}
		else if (i==0)
		{
			int img = -1;
			if(_tcsicmp(items[0],"+")==0) 
			{
				img = 3;
				item.pszText = "";
				item.cchTextMax = 0;
			}
			item.iImage = img;
			item.mask |=LVIF_INDENT;
			item.iIndent = 1;
		}
		else
		{
			item.iImage = -1;
		}
		ListView_SetItem(lv, &item);
	}
}

/*===========================================================================*\
 |  Utility implimentations
\*===========================================================================*/

void PVRefresh( HWND hWnd, PlugMgrUtility *pmu, BOOL rebuild )
{
	HWND lv = GetDlgItem(hWnd,IDC_LIST1);
	if(!rebuild) 
	{
		ListView_DeleteAllItems(lv);
		//pmu->recentArray.ClearAll();
	}

/*
	DWORD processID = 1;
	DWORD threadID = GetWindowThreadProcessId(pmu->ip->GetMAXHWnd(),&processID);
	HANDLE maxProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, processID); 

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo( maxProcess, &pmc, sizeof(pmc));
	DWORD wssSizeBefore = (pmc.WorkingSetSize/1024);

	CloseHandle(maxProcess);

	TCHAR nT[255];
	sprintf(nT,"Plugin Manager  --  Memory Used: %i KB  / Change: %i KB",wssSizeBefore);
	SetWindowText(hWnd,nT);
*/


	TCHAR *itm[6];

	DllDir *dd = pmu->ip->GetDllDirectory();
	BOOL weLoaded = FALSE;

	TCHAR dsc[255];	TCHAR fnm[255];


	for(int i=0;i<pmu->ddList.Count();i++)
	{
		DllDesc *myDesc = pmu->ddList[i].ddesc;
		
		//BOOL defr = pluginHasClassesRegistered(pmu->ddList[i].realIdx,pmu->ip);
		BOOL defr = (myDesc->handle==NULL);

		if(myDesc->loaded || myDesc->handle)
		{
			sprintf(dsc,"%s",myDesc->Description());
			weLoaded = TRUE;
		}
		else
		{
			sprintf(dsc,"%s",myDesc->tDescription);
			weLoaded = FALSE;
		}
		sprintf(fnm,"%s",strupr(myDesc->fname));

		TCHAR stat[255];
		TCHAR lSize[25]; _tcscpy(lSize,"");
		sprintf(stat,GetString(IDS_UNLOADED));
		if(defr==TRUE) 
			sprintf(stat, GetString(IDS_DEFERRED));
		else if(weLoaded==TRUE&&(pmu->ddList[i].bytesize!=-1))
		{ 
			sprintf(stat,GetString(IDS_LOADED)); 
			sprintf(lSize,GetString(IDS_PM_KB_FMT),pmu->ddList[i].bytesize); 
		}

		if(pmu->recentArray[pmu->ddList[i].realIdx]) itm[0] = "+";
		else itm[0] = "";
		itm[1] = fnm;
		itm[2] = dsc;
		itm[3] = stat;
		itm[4] = lSize;
		itm[5] = myDesc->directory;
		if(!rebuild) AddLVItem(lv, i, itm, 6);
		else SetLVItem(lv, i, itm, 6);
	}


}

void PlugMgrUtility::Init(HWND hWnd)
{
	UI_active = FALSE;

UI_MAKEBUSY

	// list view tests
	HWND lv = GetDlgItem(hWnd,IDC_LIST1);
	HWND pd = GetDlgItem(hWnd,IDC_PLUGDIR);


	pmImages = ImageList_Create(9, 9, ILC_COLOR16|ILC_MASK, 4, 0);
	HBITMAP hRed		= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RED));
	HBITMAP hTurq		= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TURQ));
	HBITMAP hGreen		= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_GREEN));
	HBITMAP hMask		= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK));
	HBITMAP hRa			= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RARROW));
	HBITMAP hRaMask		= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RA_MASK));
	ImageList_Add(pmImages, hRed, hMask);
	ImageList_Add(pmImages, hTurq, hMask);
	ImageList_Add(pmImages, hGreen, hMask);
	ImageList_Add(pmImages, hRa, hRaMask);
	DeleteObject(hRed);
	DeleteObject(hTurq);
	DeleteObject(hGreen);
	DeleteObject(hMask);
	DeleteObject(hRa);
	DeleteObject(hRaMask);

	ListView_SetImageList(lv,pmImages,LVSIL_SMALL);
	columnActive = 1;

	LV_COLUMN lvcolumn;
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetColumnName(0);
	lvcolumn.cx = 35;
	lvcolumn.iSubItem = 0;
	ListView_InsertColumn(lv,0,&lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetColumnName(1);
	lvcolumn.cx = 140;
	lvcolumn.iSubItem = 1;
	ListView_InsertColumn(lv,1,&lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetColumnName(2);
	lvcolumn.cx = 220;
	lvcolumn.iSubItem = 2;
	ListView_InsertColumn(lv,2,&lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetColumnName(3);
	lvcolumn.cx = 70;
	lvcolumn.iSubItem = 3;
	ListView_InsertColumn(lv,3,&lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetColumnName(4);
	lvcolumn.cx = 60;
	lvcolumn.iSubItem = 4;
	ListView_InsertColumn(lv,4,&lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetColumnName(5);
	lvcolumn.cx = 350;
	lvcolumn.iSubItem = 5;
	ListView_InsertColumn(lv,5,&lvcolumn);


	ListView_SetBkColor(lv,GetSysColor( COLOR_BTNSHADOW ));
	ListView_SetTextBkColor(lv,GetSysColor( COLOR_BTNSHADOW ));
	ListView_SetTextColor(lv,GetSysColor(COLOR_BTNHILIGHT));

	ListView_SetExtendedListViewStyle(lv,LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES);


	ListView_SetExtendedListViewStyle(pd,LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetString(IDS_DESCRIPTION);
	lvcolumn.cx = 200;
	lvcolumn.iSubItem = 0;
	ListView_InsertColumn(pd,0,&lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = GetString(IDS_LOAD_PATH);
	lvcolumn.cx = 400;
	lvcolumn.iSubItem = 1;
	ListView_InsertColumn(pd,1,&lvcolumn);

	int nPIE = ip->GetPlugInEntryCount();

	LV_ITEM item;
	pdrec = new BOOL[nPIE];

	for(int i=0;i<nPIE;i++)
	{
		item.mask = LVIF_TEXT|LVIF_IMAGE ;
		item.iImage = -1;
		item.iItem = i;
		item.iSubItem = 0;
		item.pszText = ip->GetPlugInDesc(i);
		item.cchTextMax = _tcslen(ip->GetPlugInDesc(i));
		ListView_InsertItem(pd, &item);

		item.mask = LVIF_TEXT|LVIF_IMAGE ;
		item.iImage = -1;
		item.iItem = i;
		item.iSubItem = 1;
		item.pszText = ip->GetPlugInDir(i);
		item.cchTextMax = _tcslen(ip->GetPlugInDir(i));
		ListView_SetItem(pd, &item);

		pdrec[i] = TRUE;
		ListView_SetCheckState(pd,i,TRUE);
	}

	retagColumns(hWnd,0,columnActive);

	rebuildCache(ip,this,hWnd);
	PVRefresh( hWnd, this, FALSE);

UI_MAKEFREE

	UI_active = TRUE;
}

extern void newPluginLoadedProc(void *param, NotifyInfo *info);
void PlugMgrUtility::Destroy(HWND hWnd)
{
	if (pdrec) delete [] pdrec;
	pdrec = NULL;
	ImageList_Destroy(pmImages);
	UnRegisterNotification(newPluginLoadedProc, (void*)this, NOTIFY_PLUGIN_LOADED);
}

